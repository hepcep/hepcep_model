#
# Analysis of hepcep model outputs - Enrollment rate effects, adherence, and num re-treatments.  
#
# Eric Tatara
#
library(data.table)
library(ggplot2)

# Std Err
std <- function(x) sd(x)/sqrt(length(x))

dt <- NULL
table <- NULL

# Load all of the stats files that exist in an experiments dir
fileName <- "/stats.csv"
dirs <- list.dirs (path=".", recursive=FALSE)

colsToKeep <- c("tick","run","cured_ALL",
                "RNApreval_ALL","RNApreval_agegrp_LEQ_30", "RNApreval_agegrp_OVER_30",
                "infected_daily_agegrp_LEQ_30", "infected_daily_agegrp_OVER_30",
                "population_agegrp_LEQ_30", "population_agegrp_OVER_30", 
                "infected_agegrp_LEQ_30", "infected_agegrp_OVER_30",
                "incidence_daily","incidence_daily_chronic","population_ALL","infected_ALL",
                "treatment_recruited_daily")

tableList <- list()
for (d in dirs){
  path <- paste0(d,fileName)
  
  if (!file.exists(path)){
    print(paste0("File doesnt exist! ",path))
  }
  else{
    print(paste0("Loading ", path ))
    
    tryCatch({
      # Read the model.props for optional storing of parameter values
      propsRead <- fread(paste0(d,"/model.props"), fill=TRUE)
      props <- propsRead[,1]
      props$Value <- propsRead[,3]
      colnames(props)<-c("Name", "Value")
      
      # Filter out columns to reduce data in memory size
      table <-  fread(path, select=colsToKeep)
      #table <-  fread(path)
      
      # Optionally store properties in the table for this run
      table$treatment_enrollment_per_PY <- props[Name=="treatment_enrollment_per_PY"]$Value
      
      table$treatment_nonadherence <- props[Name=="treatment_nonadherence"]$Value
      table$max_num_daa_treatments <- props[Name=="max_num_daa_treatments"]$Value
      
      # Enable this prop when comparing VK vs APK
      table$immunology_type <- props[Name=="immunology.type"]$Value
      
      tableList[[d]]  <- table  
    }, 
    warning = function(w) {
      print(paste0("Error loading file: ", path, " ", w))
    },
    error = function(e) {
      print(paste0("Error loading file: ", path, " ", e))
    }, 
    finally = {
    }
    )
    
  }
}

dt <- rbindlist(tableList)  # Stack the list of tables into a single DT
tableList <- NULL           # clear mem

# Optionally save the data table as an RDS
saveRDS(dt, "all_runs_dt.rds")

# NOTE can also read an existing dt from RDS here.
dt <- readRDS("all_runs_dt.rds")

# rows should be the number of entries in a single run
rows <- max(dt$tick)
#rows <- 4380
burninDays <- 365

# Day samples that correspond to the END (day 365) of each simulation year
days <- seq((burninDays+365), rows, 365)

treatement_start_year <- 2020
startYear <- 2010   # First year of simulation
endYear <- 2050    
#years <- seq(startYear, (startYear + length(days) - 1))    # list of all sim years in data

# Convert the simulation day tick to the simulated year
dayToYear <- function(firstYear, day) firstYear + floor((day)/365)
dt$Year <- unlist(lapply(dt$tick, dayToYear, firstYear=startYear-1))

# Convert the simulation day tick to month number.
# The month number will be greater than 12 for days > 365
dayToMonth <- function(day) 1 + floor((day)/31)
dt$Month <- unlist(lapply(dt$tick, dayToMonth))

# Convert the simulation day tick to week number.
# The week number will be greater than 50 for days > 365
dayToWeek <- function(day) 1 + floor((day)/7)
dt$Week <- unlist(lapply(dt$tick, dayToWeek))

years <- unlist(unique(dt$Year))

# Frame the DAA enrollment in terms of total PWID treated annually instead of percent
pwid_population_size <- 32000
dt$treatment_enrollment_size <- as.numeric(dt$treatment_enrollment_per_PY) * pwid_population_size

dt_vk <- dt[immunology_type=="VK"]
dt_apk <- dt[immunology_type=="APK"]

data <- dt_vk

# Calculate the yearly incidence rate per 1000 person-years which is the yearly sum of 
#   the dt$incidence_daily by the population count
incidenceYear <- data[Year %in% startYear:endYear, .(incidence=1000*sum(incidence_daily_chronic/(population_ALL-infected_ALL))), 
                    by=list(Year,treatment_enrollment_size, treatment_enrollment_per_PY,
                            treatment_nonadherence, max_num_daa_treatments, run)]

# Calculate the mean and std of yearly incidence rate
incidenceSummary <- incidenceYear[, list(mean=mean(incidence), sd=sd(incidence), std=std(incidence)), 
                                  by=list(Year,treatment_enrollment_size, treatment_enrollment_per_PY,
                                          treatment_nonadherence, max_num_daa_treatments)]

# Change the enrollment rate and adherence into factors for nicer plotting and..
#   convert DAA treatment non-adherence to adherence.
incidenceSummary$Adherence <- factor (100 * (1 - as.numeric(incidenceSummary$treatment_nonadherence)))
incidenceSummary$treatment_enrollment_per_PY <- factor (100 * as.numeric(incidenceSummary$treatment_enrollment_per_PY))
incidenceSummary$treatment_enrollment_size <- factor(incidenceSummary$treatment_enrollment_size)

# Create a combined percent - size factor for legend series (doesnt support tab!!)
incidenceSummary[, combined_levels := paste0(treatment_enrollment_per_PY, "% (", treatment_enrollment_size, ")")]

# Set what factor should be used for the figure legend series color
incidenceSummary$series_group <- incidenceSummary$treatment_enrollment_per_PY

incidenceSummaryBaseline <- incidenceSummary[treatment_enrollment_size == 0]

# NOTE Haven't needed to recently subset the data...
#incidenceSummarySubset <- incidenceSummary[treatment_enrollment_per_PY %in% c(2.5,5,7.5,10,20,40,60,80,100) & 
#                                             Adherence %in% c(90, 80, 70, 60) &
#                                             
#                                             # Manually update the DAA treatment max
#                                             
#                                             max_num_daa_treatments %in% c(99999)]

# Select the runs with an active DAA enrollment (> 0)
incidenceSummarySubset <- incidenceSummary[treatment_enrollment_size != 0]

# Relative incidence via the baseline normalization of the no-treatment mean in 2019
baseline <- incidenceSummaryBaseline[Year==2019]$mean

# optionally normalize the means relative to the untreated group
#  ... we also normalize the sd by the baseline mean
incidenceSummarySubset$mean <- incidenceSummarySubset$mean / baseline # incidenceSummaryBaseline$mean
incidenceSummarySubset$sd <- incidenceSummarySubset$sd / baseline # / incidenceSummaryBaseline$sd
incidenceSummarySubset$std <- incidenceSummarySubset$std / baseline # / incidenceSummaryBaseline$std

# 95% CI
z <- 1.960

legend_title <- "Annual DAA\nEnrollment %  "

# Color Version
p <- ggplot(incidenceSummarySubset) +
  geom_line(aes(x=Year-treatement_start_year+1, y=mean, color=series_group), size=1) +
  geom_point(aes(x=Year-treatement_start_year+1, y=mean, color=series_group), size=2) +
#  scale_x_continuous(limits = c(2020, endYear), breaks=seq(2020,2050,5)) +
  scale_x_continuous(limits = c(0, endYear-treatement_start_year), breaks=seq(0,endYear-treatement_start_year,5)) +
  scale_y_continuous(limits = c(0, 6)) +
  
  geom_ribbon(aes(x=Year-treatement_start_year+1, ymin=mean-z*std, ymax=mean+z*std, fill=series_group),alpha=0.3,colour=NA) +
  
  geom_hline(yintercept=0.1, linetype="dashed", color = "red") +
  geom_hline(yintercept=1.0, linetype="dashed", color = "black") +
  
#  facet_wrap(vars(Adherence), labeller = label_both) +
  
  labs(y="Relative Incidence", x="Year from DAA enrollment start", color="series_group") + #, title="All Incidence") +
  theme_bw() +
  #  theme_minimal() + 
  theme(text = element_text(size=20), 
        legend.position = c(.7, .8), 
        legend.text=element_text(size=20),
        legend.background = element_rect(fill="white", size=0.5, linetype="solid", colour ="gray")) +
  theme(axis.text=element_text(size=20),axis.title=element_text(size=20)) +
  
  guides(color=guide_legend(title=legend_title),fill=guide_legend(title=legend_title))

show(p)
ggsave("New Chronic Incidence VK stop treatment 2030 new.png", plot=p, width=10, height=8)
fwrite(incidenceSummarySubset, file="incidenceSummary.csv")

# Black & White symbol Version
p <- ggplot(incidenceSummarySubset) + geom_line(aes(x=Year+1, y=mean, group=treatment_enrollment_per_PY), size=1) +
  geom_point(aes(x=Year+1, y=mean, shape=treatment_enrollment_per_PY), size=5) +
  scale_x_continuous(limits = c(2020, endYear), breaks=c(2020, 2022, 2024, 2026, 2028, 2030)) +
  scale_y_continuous(limits = c(0, 6)) +
  
  geom_ribbon(aes(x=Year+1, ymin=mean-z*std, ymax=mean+z*std, group=treatment_enrollment_per_PY),alpha=0.3,colour=NA) +
  
  geom_hline(yintercept=0.1, linetype="dashed", color = "black") +
  
  #  facet_wrap(vars(Adherence), labeller = label_both) +
  
  scale_shape_manual(values=c(15, 16, 17, 18)) +
  
  labs(y="Incidence Relative to Year 2020", x="Year", color="treatment_enrollment_per_PY") + #, title="All Incidence") +
  theme_bw() +
  #  theme_minimal() + 
  theme(text = element_text(size=22), 
        legend.position = c(.85, .85), 
        legend.text=element_text(size=22),
        legend.background = element_rect(fill="white", size=0.5, linetype="solid", colour ="gray")) +
  theme(axis.text=element_text(size=22),axis.title=element_text(size=22)) +
  
  guides(shape=guide_legend(title="Enrollment %"),fill=guide_legend(title="Enrollment %"))

ggsave("Treatment Incidence VK num DAA treat 4.png", plot=p, width=10, height=8)



########## Calculate the Prevalence

# Mean annual prevalence (could also use last day of year) for each run
prevalenceYear <- data[Year %in% startYear:endYear, .(prevalence=mean(RNApreval_ALL)), 
                      by=list(Year,treatment_enrollment_per_PY, treatment_nonadherence, max_num_daa_treatments, run)]

# Calculate the mean and std of yearly prevalence rate across runs
prevalenceSummary <- prevalenceYear[, list(mean=mean(prevalence), sd=sd(prevalence), std=std(prevalence)), 
                                  by=list(Year,treatment_enrollment_per_PY, treatment_nonadherence, max_num_daa_treatments)]

# Change the enrollment rate and adherence into factors for nicer plotting and..
#   convert DAA treatment non-adherence to adherence.
prevalenceSummary$Adherence <- factor (100 * (1 - as.numeric(prevalenceSummary$treatment_nonadherence)))
prevalenceSummary$treatment_enrollment_per_PY <- factor (100 * as.numeric(prevalenceSummary$treatment_enrollment_per_PY))

prevalenceSummaryBaseline <- prevalenceSummary[treatment_enrollment_per_PY == 0]
prevalenceSummarySubset <- prevalenceSummary[treatment_enrollment_per_PY %in% c(2.5,5,7.5,10) & 
                                             Adherence %in% c(90, 80, 70, 60) &
                                             
                                             # Manually update the DAA treatment max
                                             
                                             max_num_daa_treatments %in% c(99999)]

# Relative prevalence via the baseline normalization of the no-treatment mean in 2019
baseline <- prevalenceSummaryBaseline[Year==2019]$mean
baseline <- 1

# optionally normalize the means relative to the untreated group
#  ... we also normalize the sd by the baseline mean
prevalenceSummarySubset$mean <- prevalenceSummarySubset$mean / baseline # prevalenceSummaryBaseline$mean
prevalenceSummarySubset$sd <- prevalenceSummarySubset$sd / baseline # / prevalenceSummaryBaseline$sd
prevalenceSummarySubset$std <- prevalenceSummarySubset$std / baseline # / prevalenceSummaryBaseline$std

# 95% CI
z <- 1.960

# Color Version
p <- ggplot(prevalenceSummarySubset) + geom_line(aes(x=Year+1, y=mean, color=treatment_enrollment_per_PY), size=1) +
  geom_point(aes(x=Year+1, y=mean, color=treatment_enrollment_per_PY), size=2) +
  #  scale_x_continuous(limits = c(2020, endYear), breaks=c(2020, 2022, 2024, 2026, 2028, 2030)) +
  scale_y_continuous(limits = c(0, 0.4)) +
  
  geom_ribbon(aes(x=Year+1, ymin=mean-z*std, ymax=mean+z*std, fill=treatment_enrollment_per_PY),alpha=0.3,colour=NA) +
  
  labs(y="RNA prevalence", x="Year", color="treatment_enrollment_per_PY") + #, title="All prevalence") +
  theme_bw() +
  #  theme_minimal() + 
  theme(text = element_text(size=14), 
        legend.position = c(.1, .17), 
        legend.text=element_text(size=14),
        legend.background = element_rect(fill="white", size=0.5, linetype="solid", colour ="gray")) +
  theme(axis.text=element_text(size=14),axis.title=element_text(size=14)) +
  
  guides(color=guide_legend(title="Enrollment %"),fill=guide_legend(title="Enrollment %"))
show(p)

ggsave("Treatment prevalence num DAA treat no limit.png", plot=p, width=10, height=8)
fwrite(prevalenceSummarySubset, file="prevalenceSummary.csv")

# Black & White symbol Version
p <- ggplot(prevalenceSummarySubset) + geom_line(aes(x=Year+1, y=mean, group=treatment_enrollment_per_PY), size=1) +
  geom_point(aes(x=Year+1, y=mean, shape=treatment_enrollment_per_PY), size=5) +
#  scale_x_continuous(limits = c(2020, endYear), breaks=c(2020, 2022, 2024, 2026, 2028, 2030)) +
  scale_y_continuous(limits = c(0, 0.4)) +
  
  geom_ribbon(aes(x=Year+1, ymin=mean-z*std, ymax=mean+z*std, group=treatment_enrollment_per_PY),alpha=0.3,colour=NA) +
  
  scale_shape_manual(values=c(15, 16, 17, 18)) +
  
  labs(y="prevalence Relative to Year 2020", x="Year", color="treatment_enrollment_per_PY") + #, title="All prevalence") +
  theme_bw() +
  #  theme_minimal() + 
  theme(text = element_text(size=22), 
        legend.position = c(.85, .85), 
        legend.text=element_text(size=22),
        legend.background = element_rect(fill="white", size=0.5, linetype="solid", colour ="gray")) +
  theme(axis.text=element_text(size=22),axis.title=element_text(size=22)) +
  
  guides(shape=guide_legend(title="Enrollment %"),fill=guide_legend(title="Enrollment %"))

ggsave("Treatment prevalence VK num DAA treat 4.png", plot=p, width=10, height=8)





# Calculate the annual in treatment sum
treatedYear <- data[Year %in% startYear:endYear, .(treated=sum(treatment_recruited_daily)), 
                  by=list(Year,treatment_enrollment_per_PY,treatment_nonadherence,max_num_daa_treatments,run)]

# Calculate the mean and sd of treatment sum
treatedYearSUmmary <- treatedYear[, list(mean=mean(treated), sd=sd(treated), std=std(treated)), 
                                  by=list(Year,treatment_enrollment_per_PY,treatment_nonadherence,max_num_daa_treatments)]

# Change the enrollment rate and adherence into factors for nicer plotting and..
#   convert DAA treatment non-adherence to adherence.
treatedYearSUmmary$Adherence <- factor (100 * (1 - as.numeric(treatedYearSUmmary$treatment_nonadherence)))
treatedYearSUmmary$treatment_enrollment_per_PY <- factor (100 * as.numeric(treatedYearSUmmary$treatment_enrollment_per_PY))
treatedYearSUmmary$reduced_treatment_enrollment_per_PY <- factor (100 * as.numeric(treatedYearSUmmary$reduced_treatment_enrollment_per_PY))


# Set what factor should be used for the figure legend series color
treatedYearSUmmary$series_group <- treatedYearSUmmary$treatment_enrollment_per_PY

#treatedYearSummaryBaseline <- treatedYearSUmmary[treatment_enrollment_per_PY == 0]
#treatedYearSummarySubset <- treatedYearSUmmary[treatment_enrollment_per_PY %in% c(2.5,5,7.5,10,20,40,60,80,100) & 
#                                                 Adherence %in% c(90, 80, 70, 60) &
#                                                 
#                                                 # Manually update the DAA treatment max
#                                                 
#                                                 max_num_daa_treatments %in% c(99999)]

treatedYearSummarySubset <- treatedYearSUmmary[treatment_enrollment_per_PY != 0]

# 95% CI
z <- 1.960

legend_title <- "Annual DAA\nEnrollment %  "

r <- ggplot(treatedYearSummarySubset) + 
  geom_line(aes(x=Year-treatement_start_year+1, y=mean, color=series_group), size=1) +
  geom_point(aes(x=Year-treatement_start_year+1, y=mean, color=series_group), size=2) +
  
  scale_x_continuous(limits = c(0, endYear-treatement_start_year), breaks=seq(0,endYear-treatement_start_year,5)) +
  
  geom_ribbon(aes(x=Year-treatement_start_year+1, ymin=mean-z*std, ymax=mean+z*std, fill=series_group),alpha=0.3,colour=NA) +
  
 
  labs(y="Total In Treatment", x="Year from DAA enrollment start", color="series_group", title="") +
  theme_bw() +
  theme(text = element_text(size=20), 
        legend.position = c(0.6, 0.5), 
        legend.text=element_text(size=20),
        legend.background = element_rect(fill="white", size=0.5, linetype="solid", colour ="gray")) +
  theme(axis.text=element_text(size=20),axis.title=element_text(size=20)) +
  
  guides(color=guide_legend(title=legend_title),fill=guide_legend(title=legend_title))

show(r)
ggsave("Treatment Counts VK stop treatment 2030 new.png", plot=r, width=10, height=8)
fwrite(treatedYearSummarySubset, file="treatmentSummary.csv")
