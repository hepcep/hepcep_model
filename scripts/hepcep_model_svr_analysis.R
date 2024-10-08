#
# Analysis of hepcep model outputs - SVR effects.  This script is intented to be
#   run completetely and not piecemeal.
#
# Eric Tatara
#
library(data.table)
library(ggplot2)
library(zoo)

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
endYear <- 2060    
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

# Calculate the yearly incidence rate per 1000 person-years which is the yearly sum of 
#   the dt$incidence_daily by the population count, and
#   group by enrollment rate and by SVR
incidenceYear <- dt[Year %in% startYear:endYear, .(incidence=1000*sum(incidence_daily/(population_ALL-infected_ALL))), by=list(Year,treatment_enrollment_per_PY, treatment_svr, run)]

# Calculate the mean and std of yearly incidence rate
incidenceSummary <- incidenceYear[, list(mean=mean(incidence), sd=sd(incidence), std=std(incidence)), by=list(Year,treatment_enrollment_per_PY, treatment_svr)]

incidenceSummaryBaseline <- incidenceSummary[treatment_enrollment_per_PY == 0]
incidenceSummarySubset <- incidenceSummary[treatment_enrollment_per_PY %in% c(0.025,0.05,0.075, 0.1)]

# Calculate the relative incidence rate via the baseline normalization of the no-treatment mean in 2019
baseline <- incidenceSummaryBaseline[Year==2019]$mean

# optionally normalize the means relative to the untreated group
#  ... we also normalize the sd by the baseline mean
incidenceSummarySubset$mean <- incidenceSummarySubset$mean / baseline # incidenceSummaryBaseline$mean
incidenceSummarySubset$sd <- incidenceSummarySubset$sd / baseline # / incidenceSummaryBaseline$sd
incidenceSummarySubset$std <- incidenceSummarySubset$std / baseline # / incidenceSummaryBaseline$std

# 95% CI
z <- 1.960

p <- ggplot(incidenceSummarySubset) + geom_line(aes(x=Year+1, y=mean, color=treatment_enrollment_per_PY), size=1) +
  geom_point(aes(x=Year+1, y=mean, color=treatment_enrollment_per_PY), size=2) +
  scale_x_continuous(limits = c(2019.9,2030.1), breaks=c(2020, 2022, 2024, 2026, 2028, 2030)) +

  facet_wrap(vars(treatment_svr)) +
  
  geom_ribbon(aes(x=Year+1, ymin=mean-z*std, ymax=mean+z*std, fill=treatment_enrollment_per_PY),alpha=0.3,colour=NA) +
  
  labs(y="Relative Incidence", x="Year", color="treatment_enrollment_per_PY") + #, title="All Incidence") +
  theme_bw() +
  #  theme_minimal() + 
  theme(text = element_text(size=14), 
        legend.position = c(.05, .15), 
        legend.text=element_text(size=14),
        legend.background = element_rect(fill="white", size=0.5, linetype="solid", colour ="gray")) +
  theme(axis.text=element_text(size=14),axis.title=element_text(size=14)) +
  
  guides(color=guide_legend(title="Enrollment"),fill=guide_legend(title="Enrollment"))
ggsave("Treatment Incidence yes-retreat.png", plot=p, width=10, height=8)
fwrite(incidenceSummarySubset, file="incidenceSummary.csv")
