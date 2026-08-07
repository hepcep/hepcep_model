#
# Build the DAA "treatment duration" summary table (Table 1 of the HepCEP DAA
# Treatment Duration manuscript) automatically, for ALL treatment_duration
# values present in an experiment directory, and for every non-zero
# treatment_enrollment_per_PY rate.
#
# For each enrollment rate the script writes a wide, Word-style table with one
# block of columns per treatment duration:
#
#     Times Treated | <wk>wk PWID Treated (95% CI) | <wk>wk Total Treatments (95% CI) | ...
#
# Rows are the "times treated" categories 1..6, a "7+" aggregate, a
# "Total Treatments" row, and a "Total Cost ($M)" row.
#
# Statistics: counts are aggregated PER RUN first (so the 7+ row and the totals
# are the mean of per-run sums), then summarised across runs as
# mean and 95% CI using the t-distribution (matches Publish::ci.mean).
#
# Eric Tatara
#
library(data.table)

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------
# Directory holding the run_* sub-directories (each with events.csv + model.props)
base_dir <- "D:\\Projects\\HepCEP\\experiments\\treatment_duration_all_daa_01"

# Where to write the output tables. Defaults to this scripts directory.
#output_dir <- "L:\\HepCEP\\hepcep_model\\scripts"
output_dir <- base_dir

# Cost per DAA treatment course in US dollars (Table 1 footnote: $11,000).
cost_per_course <- 11000

# Largest "times treated" value shown as its own row; everything above is
# pooled into the "<max_individual_count>+" row (the manuscript used 7+).
max_individual_count <- 6

eventsfileName <- "/events.csv"

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------
# Mean and 95% CI of a numeric vector using the t-distribution (n-1 df).
# Equivalent to Publish::ci.mean for a single group.
ci_mean <- function(x) {
  x <- x[!is.na(x)]
  n <- length(x)
  m <- mean(x)
  if (n > 1) {
    se <- sd(x) / sqrt(n)
    tcrit <- qt(0.975, df = n - 1)
    lower <- m - tcrit * se
    upper <- m + tcrit * se
  } else {
    se <- NA_real_; lower <- NA_real_; upper <- NA_real_
  }
  list(mean = m, se = se, lower = lower, upper = upper, n = n)
}

# Number formatting; blank for NA. No thousands separators so the file stays a
# clean CSV (Excel/Word will apply display formatting on import).
fmt_int <- function(x) ifelse(is.na(x), "", formatC(round(x), format = "d"))
fmt_num <- function(x, d = 1) ifelse(is.na(x), "", formatC(round(x, d), format = "f", digits = d))
fmt_ci  <- function(l, u) ifelse(is.na(l) | is.na(u), "",
                                 paste0("(", fmt_int(l), "-", fmt_int(u), ")"))
fmt_ci_num <- function(l, u, d = 1) ifelse(is.na(l) | is.na(u), "",
                                 paste0("(", fmt_num(l, d), "-", fmt_num(u, d), ")"))

# ---------------------------------------------------------------------------
# Load all runs -> per-run treatment-frequency table (dt)
# ---------------------------------------------------------------------------
dirs <- list.dirs(path = base_dir, recursive = FALSE)
tableList <- list()

for (d in dirs) {
  path <- paste0(d, eventsfileName)

  if (!file.exists(path)) {
    print(paste0("File doesnt exist! ", path))
    next
  }

  print(paste0("Loading ", path))

  tryCatch({
    # Read model.props for parameter values
    propsRead <- fread(paste0(d, "/model.props"), fill = TRUE)
    props <- propsRead[, 1]
    props$Value <- propsRead[, 3]
    colnames(props) <- c("Name", "Value")

    enrollment <- as.numeric(props[Name == "treatment_enrollment_per_PY"]$Value)

    # Skip zero-treatment scenarios (no STARTED_TREATMENT events)
    if (is.na(enrollment) || enrollment == 0) next

    evTable <- fread(path)
    setkey(evTable, event_type)
    start_treat_events <- evTable[.('STARTED_TREATMENT')]

    if (nrow(start_treat_events) == 0 || all(is.na(start_treat_events$person_id))) {
      print(paste0("No start treatment event in log for: ", path))
      next
    }

    # Per-person treatment frequency, then frequency-of-frequencies
    personFreq      <- as.data.frame(table(start_treat_events$person_id))
    start_treat_freq <- as.data.frame(table(personFreq$Freq))

    resultsTable <- as.data.table(start_treat_freq)
    setnames(resultsTable, old = "Var1", new = "Count")

    resultsTable$treatment_enrollment_per_PY <- enrollment
    resultsTable$treatment_nonadherence      <- as.numeric(props[Name == "treatment_nonadherence"]$Value)
    resultsTable$max_num_daa_treatments      <- as.numeric(props[Name == "max_num_daa_treatments"]$Value)
    resultsTable$treatment_duration          <- as.numeric(props[Name == "treatment_duration"]$Value)
    resultsTable$run                         <- props[Name == "run.number"]$Value

    tableList[[d]] <- resultsTable
  },
  warning = function(w) print(paste0("Error loading file: ", path, " ", w)),
  error   = function(e) print(paste0("Error loading file: ", path, " ", e)))
}

dt <- rbindlist(tableList, fill = TRUE)
tableList <- NULL

# Coerce numeric types
dt[, Count := as.numeric(as.character(Count))]
dt[, Freq  := as.numeric(Freq)]

# ---------------------------------------------------------------------------
# Per-run aggregation into "times treated" categories
# ---------------------------------------------------------------------------
plus_label <- paste0(max_individual_count + 1, "+")
cats <- c(as.character(seq_len(max_individual_count)), plus_label)

dt[, category := ifelse(Count <= max_individual_count, as.character(Count), plus_label)]

# Per (enrollment, duration, run, category): PWID treated and treatments received
per_run <- dt[, .(pwid = sum(Freq), treats = sum(Count * Freq)),
              by = .(treatment_enrollment_per_PY, treatment_duration, run, category)]

# Complete the run x category grid so absent categories count as 0 in the mean
runs_dt <- unique(dt[, .(treatment_enrollment_per_PY, treatment_duration, run)])
grid <- runs_dt[, .(category = cats),
                by = .(treatment_enrollment_per_PY, treatment_duration, run)]
per_run <- merge(grid, per_run,
                 by = c("treatment_enrollment_per_PY", "treatment_duration", "run", "category"),
                 all.x = TRUE)
per_run[is.na(pwid),   pwid := 0]
per_run[is.na(treats), treats := 0]

# Per-run totals across ALL counts (= sum over categories)
per_run_total <- dt[, .(total_treats = sum(Count * Freq),
                        total_pwid   = sum(Freq)),
                    by = .(treatment_enrollment_per_PY, treatment_duration, run)]
per_run_total[, total_cost_M := total_treats * cost_per_course / 1e6]

# ---------------------------------------------------------------------------
# Summarise across runs (mean + 95% CI)
# ---------------------------------------------------------------------------
pwid_sum <- per_run[, ci_mean(pwid),
                    by = .(treatment_enrollment_per_PY, treatment_duration, category)]
treats_sum <- per_run[, ci_mean(treats),
                      by = .(treatment_enrollment_per_PY, treatment_duration, category)]

total_treats_sum <- per_run_total[, ci_mean(total_treats),
                                  by = .(treatment_enrollment_per_PY, treatment_duration)]
total_cost_sum <- per_run_total[, ci_mean(total_cost_M),
                                by = .(treatment_enrollment_per_PY, treatment_duration)]

# ---------------------------------------------------------------------------
# Long-format raw output (all rates, durations, categories, metrics)
# ---------------------------------------------------------------------------
mk_long <- function(s, metric) {
  data.table(treatment_enrollment_per_PY = s$treatment_enrollment_per_PY,
             treatment_duration = s$treatment_duration,
             weeks = s$treatment_duration / 7,
             category = if ("category" %in% names(s)) s$category else metric,
             metric = metric,
             mean = s$mean, lower = s$lower, upper = s$upper, n = s$n)
}
raw_long <- rbindlist(list(
  mk_long(pwid_sum,         "pwid_treated"),
  mk_long(treats_sum,       "treatments_received"),
  mk_long(total_treats_sum, "total_treatments"),
  mk_long(total_cost_sum,   "total_cost_M")
), fill = TRUE)
setorder(raw_long, treatment_enrollment_per_PY, treatment_duration, metric, category)
fwrite(raw_long, file = file.path(output_dir, "treatment_duration_table_raw.csv"))
print(paste0("Wrote ", file.path(output_dir, "treatment_duration_table_raw.csv")))

# ---------------------------------------------------------------------------
# Wide, Word-style table - one file per enrollment rate
# ---------------------------------------------------------------------------
enroll_rates <- sort(unique(dt$treatment_enrollment_per_PY))
durations <- sort(unique(dt$treatment_duration))
row_order <- c(cats, "Total Treatments", "Total Cost ($M)")

for (r in enroll_rates) {
  out <- data.table(`Times Treated` = row_order)

  for (dur in durations) {
    wk <- dur / 7
    p <- pwid_sum[treatment_enrollment_per_PY == r & treatment_duration == dur]
    t <- treats_sum[treatment_enrollment_per_PY == r & treatment_duration == dur]
    tt <- total_treats_sum[treatment_enrollment_per_PY == r & treatment_duration == dur]
    tc <- total_cost_sum[treatment_enrollment_per_PY == r & treatment_duration == dur]

    setkey(p, category); setkey(t, category)
    p <- p[cats]; t <- t[cats]   # order rows by category

    # PWID treated column (mean + CI) - blank for the two total rows
    pwid_mean <- c(fmt_int(p$mean), "", "")
    pwid_ci   <- c(fmt_ci(p$lower, p$upper), "", "")

    # Treatments received column (mean + CI); total rows carry total treats / cost
    treats_mean <- c(fmt_int(t$mean), fmt_int(tt$mean), fmt_num(tc$mean, 1))
    treats_ci   <- c(fmt_ci(t$lower, t$upper),
                     fmt_ci(tt$lower, tt$upper),
                     fmt_ci_num(tc$lower, tc$upper, 1))

    out[[paste0(wk, "wk PWID Treated")]]        <- pwid_mean
    out[[paste0(wk, "wk PWID 95% CI")]]         <- pwid_ci
    out[[paste0(wk, "wk Total Treatments")]]    <- treats_mean
    out[[paste0(wk, "wk Treatments 95% CI")]]   <- treats_ci
  }

  fname <- file.path(output_dir, paste0("treatment_duration_table_enroll_", r, ".csv"))
  fwrite(out, file = fname)
  print(paste0("Wrote ", fname))
}
