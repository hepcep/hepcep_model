#
# Build Word-friendly summary tables of (re)infections from the pre-computed
# infections_summary_2.csv and reinfections_summary_2.csv files.
#
# Each output is a matrix table with one row per treatment_enrollment_per_PY and
# one column per treatment_duration; every cell is the mean number of
# (re)infections with its 95% CI in parentheses, e.g. "5,214 (5,023-5,404)".
# The 'mean' column is the mean number of infections (infections_summary_2.csv)
# or re-infections (reinfections_summary_2.csv).
#
# The columns treatment_nonadherence, max_num_daa_treatments, level, and
# statistic are ignored.
#
# Eric Tatara
#
library(data.table)

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------
# Directory holding the pre-computed summary CSVs.
base_dir <- "D:\\Projects\\HepCEP\\experiments\\treatment_duration_all_daa_01"

# Where to write the formatted tables.
#output_dir <- "L:\\HepCEP\\hepcep_model\\scripts"
output_dir <- base_dir

# Input file -> output file (and the quantity each describes)
files <- list(
  list(infile = "infections_summary_2.csv",   outfile = "infections_table_format.csv",   quantity = "infections"),
  list(infile = "reinfections_summary_2.csv", outfile = "reinfections_table_format.csv", quantity = "re-infections")
)

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------
fmt_int <- function(x) ifelse(is.na(x), "", formatC(round(x), format = "d"))

# "mean (lower-upper)"; blank if mean is missing
fmt_cell <- function(m, l, u) ifelse(is.na(m), "",
                                     paste0(fmt_int(m), " (", fmt_int(l), "-", fmt_int(u), ")"))

# 0.075 -> "7.5%"
fmt_enroll <- function(x) paste0(formatC(100 * x, format = "fg"), "%")

# ---------------------------------------------------------------------------
# Build one table per input file
# ---------------------------------------------------------------------------
for (f in files) {
  path <- file.path(base_dir, f$infile)
  if (!file.exists(path)) {
    print(paste0("File doesnt exist! ", path))
    next
  }

  dt <- fread(path)
  dt[, cell := fmt_cell(mean, lower, upper)]

  durations <- sort(unique(dt$treatment_duration))
  enrolls   <- sort(unique(dt$treatment_enrollment_per_PY))

  out <- data.table(`Enrollment Rate` = fmt_enroll(enrolls))

  for (dur in durations) {
    wk <- dur / 7
    col <- vapply(enrolls, function(r) {
      v <- dt[treatment_enrollment_per_PY == r & treatment_duration == dur]$cell
      if (length(v) == 0) "" else v[1]
    }, character(1))
    out[[paste0(wk, "-week")]] <- col
  }

  outpath <- file.path(output_dir, f$outfile)
  fwrite(out, file = outpath)
  print(paste0("Wrote mean ", f$quantity, " table (95% CI) to ", outpath))
}
