#
# Analysis of hepcep model outputs - MOUD Enrollment rate effects.  This script is intented to be
#   run completetely and not piecemeal.
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
                "incidence_daily","population_ALL","infected_ALL",
                "treatment_recruited_daily","inopioidtreatment_ALL", "inopioidtreatment_b_ALL",
                "inopioidtreatment_m_ALL","inopioidtreatment_n_ALL","opioid_treatment_recruited_daily")

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
      table$opioid_treatment_enrollment_per_PY <- props[Name=="opioid_treatment_enrollment_per_PY"]$Value
      
      table$opioid_scenario <- props[Name=="opioid_treatment_access_scenario"]$Value 
      
      table$treatment_svr <- props[Name=="treatment_svr"]$Value
      
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

# rows should be the number of entries in a single run
rows <- max(dt$tick)
#rows <- 4380
burninDays <- 365

# Day samples that correspond to the END (day 365) of each simulation year
days <- seq((burninDays+365), rows, 365)

startYear <- 2010   # First year of simulation
endYear <- 2040    
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

# TODO - incidence daily chronic, need to update

# Calculate the yearly incidence rate per 1000 person-years which is the yearly sum of 
#   the dt$incidence_daily by the population count
incidenceYear <- dt[Year %in% startYear:endYear, .(incidence=1000*sum(incidence_daily/(population_ALL-infected_ALL))), by=list(Year,opioid_treatment_enrollment_per_PY, opioid_scenario, treatment_svr, run)]
#incidenceYear <- dt[Year %in% startYear:endYear, .(incidence=1000*sum(infected_daily_agegrp_LEQ_30/(population_agegrp_LEQ_30-infected_agegrp_LEQ_30))), by=list(Year,treatment_enrollment_per_PY,run)]
#incidenceYear <- dt[Year %in% startYear:endYear, .(incidence=1000*sum(infected_daily_agegrp_OVER_30/(population_agegrp_OVER_30-infected_agegrp_OVER_30))), by=list(Year,treatment_enrollment_per_PY,run)]

# Calculate the mean and std of yearly incidence rate
incidenceSummary <- incidenceYear[, list(mean=mean(incidence), sd=sd(incidence), std=std(incidence)), by=list(Year,opioid_treatment_enrollment_per_PY, opioid_scenario, treatment_svr)]

# Convienience relabel
incidenceSummary$SVR <- factor (100 * as.numeric(incidenceSummary$treatment_svr))
incidenceSummary$opioid_treatment_enrollment_per_PY <- factor (100 * as.numeric(incidenceSummary$opioid_treatment_enrollment_per_PY))

incidenceSummaryBaseline <- incidenceSummary[opioid_treatment_enrollment_per_PY == 0]

# Subset on ony the treated groups and specific SVR
incidenceSummarySubset <- incidenceSummary[
  opioid_treatment_enrollment_per_PY %in% c(0,2.5,5,7.5,10) & SVR %in% c(90)]

# Relative incidence via the baseline normalization of the no-treatment mean in 2019
baseline <- incidenceSummaryBaseline[Year==2019]$mean

# optionally normalize the means relative to the untreated group
#  ... we also normalize the sd by the baseline mean
incidenceSummarySubset$mean <- incidenceSummarySubset$mean / baseline # incidenceSummaryBaseline$mean
incidenceSummarySubset$sd <- incidenceSummarySubset$sd / baseline # / incidenceSummaryBaseline$sd
incidenceSummarySubset$std <- incidenceSummarySubset$std / baseline # / incidenceSummaryBaseline$std

# 95% CI
z <- 1.960

p <- ggplot(incidenceSummarySubset) + geom_line(aes(x=Year+1, y=mean, color=opioid_treatment_enrollment_per_PY), size=1) +
  geom_point(aes(x=Year+1, y=mean, color=opioid_treatment_enrollment_per_PY), size=2) +
  scale_x_continuous(limits = c(2020, 2030)) + #, breaks=c(2020, 2022, 2024, 2026, 2028, 2030)) +
  scale_y_continuous(limits = c(0, 1.5), breaks=c(0, 0.25, 0.5, 0.75, 1.0, 1.25, 1.5)) +
  
  geom_ribbon(aes(x=Year+1, ymin=mean-z*std, ymax=mean+z*std, fill=opioid_treatment_enrollment_per_PY),alpha=0.3,colour=NA) +
  
  geom_hline(yintercept=0.1, linetype="dashed", color = "red") +
  
  facet_wrap(vars(opioid_scenario), labeller = label_both) +
  
  labs(y="Relative Incidence", x="Year", color="opioid_treatment_enrollment_per_PY") + #, title="All Incidence") +
  theme_bw() +
  #  theme_minimal() + 
  guides(color=guide_legend(title="Enrollment %"),fill=guide_legend(title="Enrollment %")) +
  theme(text = element_text(size=12), 
        legend.position = c(.15, .25), 
        legend.text=element_text(size=16),
        legend.background = element_rect(fill="gray95", size=.5, linetype="solid")) +
  theme(axis.text=element_text(size=12),axis.title=element_text(size=12))

ggsave("Treatment Incidence.png", plot=p, width=10, height=8)
fwrite(incidenceSummarySubset, file="incidenceSummary.csv")


# Calculate the Prevalence
prevalenceYear <- dt[Year %in% startYear:endYear, .(prevalence=RNApreval_ALL), by=list(Year,opioid_treatment_enrollment_per_PY, opioid_scenario, treatment_svr, run)]

# Calculate the mean and std of yearly incidence rate
prevalenceSummary <- prevalenceYear[, list(mean=mean(prevalence), sd=sd(prevalence), std=std(prevalence)), by=list(Year,opioid_treatment_enrollment_per_PY, opioid_scenario, treatment_svr)]

prevalenceSummaryBaseline <- prevalenceSummary[opioid_treatment_enrollment_per_PY == 0]
prevalenceSummarySubset <- prevalenceSummary[opioid_treatment_enrollment_per_PY %in% c(0,0.025,0.05,0.075,0.1) & treatment_svr %in% c(0.6, 0.7, 0.8, 0.9)]

#prevalenceSummarySubset <- rbind(prevalenceSummarySubset,prevalenceSummaryBaseline)

# Relative incidence via the baseline normalization of the no-treatment mean in 2019
#baseline <- prevalenceSummaryBaseline[Year==2019]$mean

# optionally normalize the means relative to the untreated group
#  ... we also normalize the sd by the baseline mean
#prevalenceSummarySubset$mean <- prevalenceSummarySubset$mean / baseline # incidenceSummaryBaseline$mean
#prevalenceSummarySubset$sd <- prevalenceSummarySubset$sd / baseline # / incidenceSummaryBaseline$sd
#prevalenceSummarySubset$std <- prevalenceSummarySubset$std / baseline # / incidenceSummaryBaseline$std

# 95% CI
z <- 1.960

q <- ggplot(prevalenceSummarySubset) + geom_line(aes(x=Year+1, y=mean, color=opioid_treatment_enrollment_per_PY), size=1) +
  geom_point(aes(x=Year+1, y=mean, color=opioid_treatment_enrollment_per_PY), size=2) +
  scale_x_continuous(limits = c(2019.9,2030)) + #, breaks=c(2020, 2022, 2024, 2026, 2028, 2030)) +
  
  geom_ribbon(aes(x=Year+1, ymin=mean-z*std, ymax=mean+z*std, fill=opioid_treatment_enrollment_per_PY),alpha=0.3,colour=NA) +
  
  facet_wrap(vars(opioid_scenario)) +
  
  labs(y="Prevalence", x="Year", color="opioid_treatment_enrollment_per_PY") + #, title="All Incidence") +
  theme_bw() +
  #  theme_minimal() + 
  theme(text = element_text(size=14), 
        legend.position = c(.4, .35), 
        legend.text=element_text(size=14),
        legend.background = element_rect(fill="white", size=0.5, linetype="solid", colour ="gray")) +
  theme(axis.text=element_text(size=14),axis.title=element_text(size=14)) +
  
  guides(color=guide_legend(title="Enrollment"),fill=guide_legend(title="Enrollment"))
ggsave("Treatment Prevalence.png", plot=q, width=10, height=8)
fwrite(prevalenceSummarySubset, file="prevalenceSummary.csv")

# Calculate the annual in treatment sum
treatedYear <- dt[Year %in% startYear:endYear, .(treated=sum(opioid_treatment_recruited_daily)), by=list(Year,opioid_treatment_enrollment_per_PY,opioid_scenario,treatment_svr,run)]

# Calculate the mean and sd of treatment sum
treatedYearSUmmary <- treatedYear[, list(mean=mean(treated), sd=sd(treated), std=std(treated)), by=list(Year,opioid_treatment_enrollment_per_PY,opioid_scenario,treatment_svr)]

treatedYearSummaryBaseline <- treatedYearSUmmary[opioid_treatment_enrollment_per_PY == 0]
treatedYearSummarySubset <- treatedYearSUmmary[opioid_treatment_enrollment_per_PY %in% c(0,0.025,0.05,0.075,0.1) & treatment_svr %in% c(0.6, 0.7, 0.8, 0.9)]

r <- ggplot(treatedYearSummarySubset) + geom_line(aes(x=Year+1, y=mean, color=opioid_treatment_enrollment_per_PY), size=1) +
  geom_point(aes(x=Year+1, y=mean, color=opioid_treatment_enrollment_per_PY), size=2) +
  scale_x_continuous(limits = c(2019.9,2030)) + #, breaks=c(2020, 2022, 2024, 2026, 2028, 2030)) +
  
  geom_ribbon(aes(x=Year+1, ymin=mean-z*std, ymax=mean+z*std, fill=opioid_treatment_enrollment_per_PY),alpha=0.3,colour=NA) +
  
  facet_wrap(vars(opioid_scenario)) +
  
  # geom_label_repel(aes(x=Year+1, y=mean, label=ifelse(mean > 0, mean, ""), color=opioid_treatment_enrollment_per_PY    ),
  #                  box.padding   = 0.35,
  #                  point.padding = 0.5,
  #                  segment.color = 'grey50') +
  
  labs(y="Total In Treatment", x="Year", color="opioid_treatment_enrollment_per_PY", title="") +
  theme_bw() +
  theme(text = element_text(size=14), 
        legend.position = c(0.4, 0.3), 
        legend.text=element_text(size=14),
        legend.background = element_rect(fill="white", size=0.5, linetype="solid", colour ="gray")) +
  theme(axis.text=element_text(size=14),axis.title=element_text(size=14)) +
  
  guides(color=guide_legend(title="Enrollment"),fill=guide_legend(title="Enrollment"))
ggsave("Treatment Counts.png", plot=r, width=10, height=8)
fwrite(treatedYearSummarySubset, file="treatmentSummary.csv")



