#
# Analysis of hepcep model outputs
#
# Eric Tatara
#
library(data.table)
library(ggplot2)
library(zoo)

# Std Err
std <- function(x) sd(x)/sqrt(length(x))

# Load all of the stats files that exist in an experiments dir
fileName <- "/stats.csv"
events_fileName <- "/events.csv"
dirs <- list.dirs (path=".", recursive=FALSE)

colsToKeep <- c("tick","run","cured_ALL",
                "RNApreval_ALL",
                "incidence_daily","population_ALL","infected_ALL",
                "treatment_recruited_daily")

tableList <- list()
for (d in dirs){
  path <- paste0(d,fileName)
  events_path <- paste0(d,events_fileName)
  
  if (!file.exists(path)){
    print(paste0("File doesnt exist! ", path))
  }
  if (!file.exists(events_path)){
    print(paste0("File doesnt exist! ", events_path))
  }
  else{
    print(paste0("Loading ", path, ", ", events_path ))
    
    tryCatch({
      # Read the model.props for optional storing of parameter values
      propsRead <- fread(paste0(d,"/model.props"), fill=TRUE)
      props <- propsRead[,1]
      props$Value <- propsRead[,3]
      colnames(props)<-c("Name", "Value")
      
      table <-  fread(path, select=colsToKeep)
      events_table <- fread(events_path)

      # Optionally store properties in the table for this run
      table$treatment_enrollment_per_PY <- props[Name=="treatment_enrollment_per_PY"]$Value
      
      # Treatment enrollment probabilities
      table$tep_HRP         <- props[Name=="treatment_enrollment_probability_HRP"]$Value
      table$tep_fullnetwork <- props[Name=="treatment_enrollment_probability_fullnetwork"]$Value
      table$tep_inpartner   <- props[Name=="treatment_enrollment_probability_inpartner"]$Value
      table$tep_outpartner  <- props[Name=="treatment_enrollment_probability_outpartner"]$Value
      table$tep_unbiased    <- props[Name=="treatment_enrollment_probability_unbiased"]$Value
      
      # Record the total treated in just the last row of the table.  Later we might
      #   Want to save the treatments at each time they occur by merging tables here.
      lastrow <- nrow(table)
      table$num_treatments <- rep(0,lastrow)
      # Use -1 index since the last day is usually the first day of the next year
      table[lastrow-1]$num_treatments <- nrow(events_table[event_type == 'STARTED_TREATMENT'])
      
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


# TODO change DT to exclude tick 0 and tick 1:burninDays in all data sets
#!(tick %in% 1:burninDays)

# Calculate the yearly incidence rate per 1000 person-years which is the yearly sum of 
#   the dt$incidence_daily by the population count 

#incidenceYear <- dt[Year %in% startYear:endYear, .(incidence=1000*sum(incidence_daily/(population_ALL-infected_ALL))), by=list(Year,enrollment_pattern,run)]

incidenceYear <- dt[Year %in% startYear:endYear, list(incidence=1000*sum(incidence_daily/(population_ALL-infected_ALL)),
                                                      num_treatments=sum(num_treatments)), by=list(Year,enrollment_pattern,run)]

#incidenceYear <- dt[Year %in% startYear:endYear, .(incidence=1000*sum(infected_daily_agegrp_LEQ_30/(population_agegrp_LEQ_30-infected_agegrp_LEQ_30))), by=list(Year,enrollment_pattern,run)]
#incidenceYear <- dt[Year %in% startYear:endYear, .(incidence=1000*sum(infected_daily_agegrp_OVER_30/(population_agegrp_OVER_30-infected_agegrp_OVER_30))), by=list(Year,enrollment_pattern,run)]

# Aggregate over the stochastic run variations
# Calculate the mean and std of yearly incidence rate
incidenceSummary <- incidenceYear[, list(inc_mean=mean(incidence), inc_sd=sd(incidence), inc_std=std(incidence),
                                         treatments_mean=mean(num_treatments)), 
                                  by=list(Year,enrollment_pattern)]

# The baseline normalization is the no-treatment mean in 2019
baseline <- 12.914

# normalize the means relative to the baseline
#  ... we also normalize the sd by the baseline mean
incidenceSummary$inc_mean <- incidenceSummary$inc_mean / baseline # incidenceSummaryBaseline$mean
incidenceSummary$inc_sd <- incidenceSummary$inc_sd / baseline # / incidenceSummaryBaseline$sd
incidenceSummary$inc_std <- incidenceSummary$inc_std / baseline # / incidenceSummaryBaseline$std

# find the top 20 incidence at end year and sort mean low to high
#finalIncidence <- incidenceSummary[Year == endYear][order(mean)] #[1:20,]
#finalIncidence <- incidenceSummary[Year == endYear & mean >= 0.1][order(mean)] #[1:20,]
finalIncidence <- incidenceSummary[Year == endYear][order(treatments_mean)] #[1:20,]

finalIncidenceTop20 <- incidenceSummary[Year == endYear & inc_mean == 0][order(treatments_mean)][1:20,]

# Profile for non-treatment (enrollment prop = 0)
#incidenceSummaryNoTreat <- incidenceSummary[enrollment_pattern == "0_0.0:0.0:0.0:0.0:1.0"]

# 95% CI
# z <- 1.960
# 
# # Plot errors on final incidences
# p <- ggplot(finalIncidence, aes(y=reorder(enrollment_pattern,-mean), x=mean)) +
#     geom_errorbarh(aes(xmax = mean+z*std, xmin = mean-z*std, height = .2), color='blue') +
#     geom_point() +
#     labs(y="Enrollment Pattern", x="Mean Relative Incidence (per 1000 person-years)", title="Lowest Relative Incidence at Year 2030") +
#     theme_minimal() +
# #    scale_x_continuous(limits=c(0.0,1.0)) + 
#     theme(text = element_text(size=14), legend.position="none") +
#     theme(plot.title=element_text(size=20)) +
#     theme(axis.text=element_text(size=18),axis.title=element_text(size=18)) +
#     theme(axis.text.y = element_blank()) +
#     theme(panel.grid.major.y =  element_blank()) +
#     theme(panel.border = element_rect(colour = "black", fill=NA, size=0.5)) 
# 
# ggsave("Final Incidence Rates yes-retreat_part.png", plot=p, width=10, height=8)
 
# Create the 2D pareto-like plot of incidence vs. num treatments to compare with the GA results
d <- finalIncidence
d$treatments_mean <- d$treatments_mean / 10000

# Color-blind palette with grey:
cbPalette <- c("#999999", "#E69F00", "#56B4E9", "#009E73", "#F0E442", "#0072B2", "#D55E00", "#CC79A7")

sweep_sol_plot <- ggplot(d) +
  geom_point(aes(x=treatments_mean, y=inc_mean), shape=16, size=10, alpha=0.5, color='black') +
  scale_y_continuous( limits=c(0,1), breaks=c(0,0.25,0.5,0.75,1)) +
  scale_x_continuous(limits=c(0.2,1.3), breaks=c(0.2,0.4,0.6,0.8,1.0,1.2)) +
  labs(y="Relative Incidence (per 1000 person-years)", x=expression("Treatment Count x10"^"4")) +
  #    labs(y="", x="", title=paste0("Generation ",i)) +
  theme_minimal() +
  #    theme(panel.grid.major = element_line(colour = "black",size=0.5)) +
  theme(panel.border = element_rect(colour = "black", fill=NA, size=0.5)) + 
#  theme(panel.grid.minor = element_blank()) +
  theme(panel.grid.major = element_blank()) +
  theme(axis.ticks = element_line(size = 0.5)) + 
  theme(axis.text=element_text(size=20),axis.title=element_text(size=20))

ggsave("sweep solutions.png", plot=sweep_sol_plot, width=10, height=8)


# Combine the sweep and GA pareto results.  GA results must already be loaded
d2 <- pareto_table[Generation == 20]
d2$Treatment_Count <- d2$Treatment_Count / 10000

p <- sweep_sol_plot +
  geom_point(data=d2, aes(x=Treatment_Count, y=Relative_Incidence), shape=16, 
             size=10, alpha=0.7, color=cbPalette[3]) +
  geom_hline(yintercept=0.1, linetype="dashed", color = "red", size=1)

ggsave("sweep plus GA solutions_14.png", plot=p, width=10, height=8) 
