#
# Analysis of hepcep model outputs
#
# Eric Tatara
#
library(data.table)
library(ggplot2)
library(zoo)

# Load all of the stats files that exist in an experiments dir
fileName <- "/stats.csv"
dirs <- list.dirs (path=".", recursive=FALSE)

colsToKeep <- c("tick","run","cured_ALL",
                "RNApreval_ALL","RNApreval_agegrp_LEQ_30", "RNApreval_agegrp_OVER_30",
                "infected_daily_agegrp_LEQ_30", "infected_daily_agegrp_OVER_30",
                "population_agegrp_LEQ_30", "population_agegrp_OVER_30", 
                "infected_agegrp_LEQ_30", "infected_agegrp_OVER_30",
                "incidence_daily","population_ALL","infected_ALL",
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
      
      table <-  fread(path, select=colsToKeep)
      
      # Optionally store properties in the table for this run
      table$treatment_enrollment_per_PY <- props[Name=="treatment_enrollment_per_PY"]$Value
      
      # Treatment enrollment probabilities
      table$tep_HRP         <- props[Name=="treatment_enrollment_probability_HRP"]$Value
      table$tep_fullnetwork <- props[Name=="treatment_enrollment_probability_fullnetwork"]$Value
      table$tep_inpartner   <- props[Name=="treatment_enrollment_probability_inpartner"]$Value
      table$tep_outpartner  <- props[Name=="treatment_enrollment_probability_outpartner"]$Value
      table$tep_unbiased    <- props[Name=="treatment_enrollment_probability_unbiased"]$Value
      
      
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

startYear <- 2010   
endYear <- 2030    
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

# Create unique string IDs for each pattern of enrollment probabilities in each run
#   that we can use for grouping results.
#   Pattern is enrollment probability_HRP:FULLNETWORK:INPARTNER:OUTPARNER:UNBIASED
dt$enrollment_pattern <- paste0(dt[,treatment_enrollment_per_PY],"_", dt[,tep_HRP],":",
                                dt[,tep_fullnetwork],":",dt[,tep_inpartner],":",
                                dt[,tep_outpartner],":",dt[,tep_unbiased])

# Std Err
se <- function(x) sd(x)/sqrt(length(x))

# 95% confidence interval on t-distribution
error_95 <- function(x) qt(0.975,df=length(x)-1)*sd(x)/sqrt(length(x))

# TODO change DT to exclude tick 0 and tick 1:burninDays in all data sets
#!(tick %in% 1:burninDays)

# ##### GRAPHS #####

# ##### INCIDENCE #####
# Calculate the yearly incidence rate per 1000 person-years which is the yearly sum of 
#   the dt$incidence_daily by the population count 

incidenceYear <- dt[Year %in% startYear:endYear, .(incidence=1000*sum(incidence_daily/(population_ALL-infected_ALL))), by=list(Year,enrollment_pattern,run)]
#incidenceYear <- dt[Year %in% startYear:endYear, .(incidence=1000*sum(infected_daily_agegrp_LEQ_30/(population_agegrp_LEQ_30-infected_agegrp_LEQ_30))), by=list(Year,enrollment_pattern,run)]
#incidenceYear <- dt[Year %in% startYear:endYear, .(incidence=1000*sum(infected_daily_agegrp_OVER_30/(population_agegrp_OVER_30-infected_agegrp_OVER_30))), by=list(Year,enrollment_pattern,run)]

# Calculate the mean and std of yearly incidence rate
incidenceSummary <- incidenceYear[, list(mean=mean(incidence), sd=sd(incidence), error_95=error_95(incidence) ), 
                                  by=list(Year,enrollment_pattern)]

# find the top 20 incidence at end year and sort mean low to high
finalIncidence <- incidenceSummary[Year == endYear][order(mean)][1:20,]

# Incidence summary for all years for the top set
incidenceSummaryTop <- incidenceSummary[enrollment_pattern %in% finalIncidence$enrollment_pattern]

# Profile for non-treatment (enrollment prop = 0)
incidenceSummaryNoTreat <- incidenceSummary[enrollment_pattern == "0_0.0:0.0:0.0:0.0:1.0"]

# Plot the mean incidence groups by enrollment pattern
treatmentStartYear <- 2010
p <- ggplot(incidenceSummary) + geom_line(aes(x=Year, y=mean, group=enrollment_pattern), 
                                          size=0.1, alpha=0.4, color='black') +
     geom_line(data=incidenceSummaryTop, aes(x=Year, y=mean, group=enrollment_pattern), 
            size=0.5, alpha=0.4, color='blue') +
  
     geom_line(data=incidenceSummaryNoTreat, aes(x=Year, y=mean, group=enrollment_pattern), 
            size=1.0, alpha=1.0, color='green') +
  
     geom_errorbar(data=finalIncidence, aes(x=Year, ymin=mean-sd, ymax=mean+sd, 
            group=enrollment_pattern), width=0.5, alpha=0.4, color='blue') +
  
     geom_errorbar(data=incidenceSummaryNoTreat[Year == 2030], aes(x=Year, ymin=mean-sd, ymax=mean+sd, 
             group=enrollment_pattern), width=0.5, alpha=1.0, color='green') +
  
  
#     scale_x_continuous(limits=c(treatmentStartYear,endYear), breaks=seq(startYear,endYear,5)) +

#    + geom_errorbar(aes(x=Year, ymin=mean-sd, ymax=mean+sd, color=enrollment_pattern),width=.15) 
#    + scale_y_continuous(limits=c(0, 0.4), breaks=seq(0,1.0,0.1))
    labs(y="Incidence (per 1000 person-years)", x="Year", color="enrollment_pattern", title="Age 30+ Incidence") +
    theme_minimal() +
#   theme(text = element_text(size=20), legend.position = c(.85, .80), legend.text=element_text(size=20)) +
    theme(text = element_text(size=20), legend.position="none") 

ggsave("Treatment Incidence.png", plot=p, width=10, height=8)


# Plot errors on final incidences
p <- ggplot(finalIncidence, aes(y=reorder(enrollment_pattern,-mean), x=mean)) +
#     geom_point() + geom_errorbarh(aes(xmax = mean+sd, xmin = mean-sd, height = .2), color='blue') + 
     geom_point() + geom_errorbarh(aes(xmax = mean+error_95, xmin = mean-error_95, height = .2), color='blue') +

     labs(y="Enrollment Pattern", x="Mean Annual Incidence Rate", title="Age 30+ Lowest Incidence Rates at Year 2030 (95% CI)") +
     theme_minimal() +
     scale_x_continuous(limits=c(5,10)) + 
     theme(text = element_text(size=14), legend.position="none", plot.title=element_text(size=12)) 
ggsave("Final Incidence Rate spread.png", plot=p, width=10, height=8)
  

# #### PREVALENCE ####
# The annual data is a snapshot of stats on the last day in the year
annualData <- dt[Year %in% startYear:endYear & tick %in% days]


# Calculate the mean and std of prevalence
#prevalenceSummary <- annualData[, list(mean=mean(RNApreval_ALL),sd=sd(RNApreval_ALL),error_95=error_95(RNApreval_ALL)), by=list(Year,enrollment_pattern)]
#prevalenceSummary <- annualData[, list(mean=mean(RNApreval_agegrp_LEQ_30),sd=sd(RNApreval_agegrp_LEQ_30),error_95=error_95(RNApreval_agegrp_LEQ_30)), by=list(Year,enrollment_pattern)]
prevalenceSummary <- annualData[, list(mean=mean(RNApreval_agegrp_OVER_30),sd=sd(RNApreval_agegrp_OVER_30),error_95=error_95(RNApreval_agegrp_OVER_30)), by=list(Year,enrollment_pattern)]

# find the top 20 incidence at end year and sort mean low to high
finalPrevalence <- prevalenceSummary[Year == endYear][order(mean)][1:20,]

# Prevalence summary for all years for the top set
prevalenceSummaryTop <- prevalenceSummary[enrollment_pattern %in% finalPrevalence$enrollment_pattern]

# Profile for non-treatment (enrollment prop = 0)
prevalenceSummaryNoTreat <- prevalenceSummary[enrollment_pattern == "0_0.0:0.0:0.0:0.0:1.0"]

treatmentStartYear <- 2010
p <- ggplot(prevalenceSummary) + geom_line(aes(x=Year, y=mean, group=enrollment_pattern), 
                                          size=0.1, alpha=0.4, color='black') +
    geom_line(data=prevalenceSummaryTop, aes(x=Year, y=mean, group=enrollment_pattern), 
            size=0.5, alpha=0.4, color='blue') +
  
     geom_line(data=prevalenceSummaryNoTreat, aes(x=Year, y=mean, group=enrollment_pattern), 
            size=1.0, alpha=1.0, color='green') +
  
     geom_errorbar(data=finalPrevalence, aes(x=Year, ymin=mean-sd, ymax=mean+sd, 
                                         group=enrollment_pattern), width=0.5, alpha=0.4, color='blue') +
  
     geom_errorbar(data=prevalenceSummaryNoTreat[Year == 2030], aes(x=Year, ymin=mean-sd, ymax=mean+sd, 
                                                                group=enrollment_pattern), width=0.5, alpha=1.0, color='green') +
  
  
  #     scale_x_continuous(limits=c(treatmentStartYear,endYear), breaks=seq(startYear,endYear,5)) +
  
  #    + geom_errorbar(aes(x=Year, ymin=mean-sd, ymax=mean+sd, color=enrollment_pattern),width=.15) 
  #    + scale_y_continuous(limits=c(0, 0.4), breaks=seq(0,1.0,0.1))
  labs(y="HCV RNA Prevalence", x="Year", color="enrollment_pattern", title="Ages 30+ Prevalence") +
  theme_minimal() +
  #   theme(text = element_text(size=20), legend.position = c(.85, .80), legend.text=element_text(size=20)) +
  theme(text = element_text(size=20), legend.position="none") 

ggsave("Treatment Prevalence.png", plot=p, width=10, height=8)

# Plot errors on final prevalence
p <- ggplot(finalPrevalence, aes(y=reorder(enrollment_pattern,-mean), x=mean)) +
  #     geom_point() + geom_errorbarh(aes(xmax = mean+sd, xmin = mean-sd, height = .2), color='blue') + 
  geom_point() + geom_errorbarh(aes(xmax = mean+error_95, xmin = mean-error_95, height = .2), color='blue') +
  
  labs(y="Enrollment Pattern", x="Mean Prevalence", title="Age 30+ Lowest Prevalence at Year 2030 (95% CI)") +
  theme_minimal() +
  scale_x_continuous(limits=c(0.07,0.11)) + 
  theme(text = element_text(size=14), legend.position="none", plot.title=element_text(size=12)) 
ggsave("Final Prevalence spread.png", plot=p,  width=8, height=6)









# TODO

# Calculate the annual cured counts

# The annual data is a snapshot of stats on the last day in the year
annualData <- dt[Year %in% startYear:endYear & tick %in% days]

# Calculate the mean and std of yearly cured counts
curedYearSummary <- annualData[, list(mean=mean(cured_ALL), sd=sd(cured_ALL)), by=list(Year,enrollment_pattern)]
#curedSummarySubset <- curedYearSummary[enrollment_pattern %in% c(0,0.01,0.05)]

p <- ggplot(curedYearSummary) + geom_line(aes(x=Year, y=mean, color=enrollment_pattern), 
                                          size=0.01, alpha=0.1, color='blue') +
  scale_x_continuous(limits=c(treatmentStartYear,endYear), breaks=seq(startYear,endYear,5)) +

  labs(y="Total Cured", x="Year", color="enrollment_pattern", title="") +
  theme_minimal() +
  theme(text = element_text(size=20), legend.position="none") 
ggsave("Cured Counts.png", plot=p, width=10, height=8)


# Calculate the annual in treatment sum
treatedYear <- dt[Year %in% startYear:endYear, .(treated=sum(treatment_recruited_daily)), by=list(Year,enrollment_pattern,run)]

# Calculate the mean and sd of treatment sum
treatedYearSUmmary <- treatedYear[, list(mean=mean(treated), sd=sd(treated)), by=list(Year,enrollment_pattern)]

p <- ggplot(treatedYearSUmmary) + geom_line(aes(x=Year, y=mean, color=enrollment_pattern), size=1) +
  geom_point(aes(x=Year, y=mean, color=enrollment_pattern), size=2) +
  scale_x_continuous(breaks=seq(startYear,endYear,5)) +
  
  geom_errorbar(aes(x=Year, ymin=mean-sd, ymax=mean+sd, color=enrollment_pattern),width=.15) +
  
  #  scale_y_continuous(limits=c(0, 0.4), breaks=seq(0,1.0,0.1)) +
  labs(y="Total In Treatment", x="Year", color="enrollment_pattern", title="") +
  theme_minimal() +
  theme(text = element_text(size=20), legend.position = c(.85, .80), legend.text=element_text(size=20)) +
  guides(color=guide_legend(title="Enrollment"))
ggsave("Treatment Counts.png", plot=p, width=10, height=8)



