#
# Analysis of hepcep model outputs - Enrollment rate effects, adherence, and num re-treatments.  
#
# Eric Tatara
#
library(data.table)
library(ggplot2)

source("hepcep_plots.R")

dt <- NULL
table <- NULL

base_dir <- "D:\\Projects\\HepCEP\\experiments\\vk_apk_no_stop_treatment_01\\"
#base_dir <- "D:\\Projects\\HepCEP\\experiments\\treatment_duration_all_daa_01\\"

# Load all of the stats files that exist in an experiments dir
fileName <- "/stats.csv"
dirs <- list.dirs (path=base_dir, recursive=FALSE)

# colsToKeep <- c("tick","run","cured_ALL",
#                 "RNApreval_ALL","RNApreval_agegrp_LEQ_30", "RNApreval_agegrp_OVER_30",
#                 "infected_daily_agegrp_LEQ_30", "infected_daily_agegrp_OVER_30",
#                 "population_agegrp_LEQ_30", "population_agegrp_OVER_30", 
#                 "infected_agegrp_LEQ_30", "infected_agegrp_OVER_30",
#                 "incidence_daily","incidence_daily_chronic","population_ALL","infected_ALL",
#                 "treatment_recruited_daily")


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
      #table <-  fread(path, select=colsToKeep)
      table <-  fread(path)
      
      # Optionally store properties in the table for this run
      table$treatment_enrollment_per_PY <- props[Name=="treatment_enrollment_per_PY"]$Value
      
      table$treatment_nonadherence <- props[Name=="treatment_nonadherence"]$Value
      table$max_num_daa_treatments <- props[Name=="max_num_daa_treatments"]$Value
      
      # Enable this prop when comparing VK vs APK
      table$immunology_type <- props[Name=="immunology.type"]$Value
      
      table$treatment_duration <- props[Name=='treatment_duration']$Value
      
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
#saveRDS(dt, paste0(base_dir,"all_runs_dt.rds"))

# NOTE can also read an existing dt from RDS here.
dt <- readRDS(paste0(base_dir,"all_runs_dt.rds"))

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

dt$treatment_duration_weeks <- (as.numeric(dt$treatment_duration))/7

# Frame the DAA enrollment in terms of total PWID treated annually instead of percent
pwid_population_size <- 32000
dt$treatment_enrollment_size <- as.numeric(dt$treatment_enrollment_per_PY) * pwid_population_size

# Filter on treatment duration 28 56 84 168  (days)
#data <- dt[immunology_type=="VK" & treatment_duration==84]
data <- dt[immunology_type=="VK"]
#data <- dt[immunology_type=="APK"]
#data <- dt

# Reassign if using the DAA enrollment reduction approach
#data$treatment_enrollment_per_PY <- data$reduced_treatment_enrollment_per_PY

# The data summary interval period, e.g. annual, monthly rate

interval <- 'Year'   # note - use get(interval) in referencing data frame vars
#interval <- 'Month'

p <- new_chronic_incidence_plot(data, startYear, endYear, treatement_start_year, scale_baseline=T)
show(p)

ggsave(paste0(base_dir,"New Chronic Incidence VK no stop treatment 2030 2.png"), plot=p, width=10, height=8)
fwrite(p$data, paste0(base_dir,"New Chronic Incidence VK reduce treatment 2030.csv"))

p <- new_chronic_actual_plot(data, startYear, endYear, treatement_start_year, scale_baseline=F)
fwrite(p$data, paste0(base_dir,"New Chronic actual.csv"))
show(p)

p <- susceptible_plot(data, startYear, endYear, treatement_start_year, scale_baseline=F)
fwrite(p$data, paste0(base_dir,"Susceptible actual.csv"))
show(p)

p <- infected_plot(data, startYear, endYear, treatement_start_year, scale_baseline=F)
fwrite(p$data, paste0(base_dir,"infected actual.csv"))
show(p)

#fwrite(p$data[treatment_enrollment_per_PY == 0], paste0(base_dir,"New Chronic Incidence VK zero DAA enroll.csv"))

# HCV RNA Prevalence Plots
p <- prevalence_plot(data, startYear, endYear, treatement_start_year)
show(p)
fwrite(p$data, paste0(base_dir,"Prevalence VK no stop treatment.csv"))
ggsave(paste0(base_dir,"Prevalence stop treatment 2030.png"), plot=p, width=10, height=8)

#fwrite(p$data[treatment_enrollment_per_PY == 0], paste0(base_dir,"Prevalence VK zero DAA enroll.csv"))




# TODO Move below to hepcep_plots.R

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

treatedYearSummarySubset$lower_CI <- treatedYearSummarySubset$mean - z * treatedYearSummarySubset$std
treatedYearSummarySubset$upper_CI <- treatedYearSummarySubset$mean + z * treatedYearSummarySubset$std 

legend_title <- "DAA enrollment %"
#legend_title <- "Screening %"

r <- ggplot(treatedYearSummarySubset) + 
  geom_line(aes(x=Year-treatement_start_year+1, y=mean, color=series_group), size=1) +
  geom_point(aes(x=Year-treatement_start_year+1, y=mean, color=series_group), size=2) +
  
  scale_x_continuous(limits = c(0, endYear-treatement_start_year), breaks=seq(0,endYear-treatement_start_year,5)) +
#  scale_y_continuous(limits = c(0, 1000)) +
  
  geom_ribbon(aes(x=Year-treatement_start_year+1, ymin=lower_CI, ymax=upper_CI, fill=series_group),alpha=0.3,colour=NA) +
  
 
  labs(y="Total In Treatment", x="Year from DAA enrollment start", color="series_group", title="") +
  theme_bw() +
  theme(text = element_text(size=20), 
        legend.position = c(0.6, 0.5), 
        legend.text=element_text(size=20),
        legend.background = element_rect(fill="white", size=0.5, linetype="solid", colour ="gray")) +
  theme(axis.text=element_text(size=20),axis.title=element_text(size=20)) +
  
  guides(color=guide_legend(title=legend_title),fill=guide_legend(title=legend_title))

#show(r)
fwrite(r$data, paste0(base_dir,"Treatment Counts VK stop treatment 2030.csv"))

ggsave(paste0(base_dir,"Treatment Counts VK.png"), plot=r, width=10, height=8)
#fwrite(treatedYearSummarySubset, file="treatmentSummary.csv")
