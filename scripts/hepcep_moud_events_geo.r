#
# Analysis of hepcep model outputs for MOUD experiments.
#
#  This script generates CSV files with scores for treatment duration and
#    new chronic infections by zip code and MOUD drug type.
#
# Eric Tatara
#


library(data.table)

agentsfileName <- "/agents.csv"

agentsfileName <- "/agents.csv"
eventsfileName <- "/events.csv"
statsfilename <- "/stats.csv"

std <- function(x) sd(x)/sqrt(length(x))

dt <- NULL
dt_events <- NULL
dt_agents <- NULL
table <- NULL
event_table <- NULL
agent_table <- NULL

#dirs <- c("./run_200")   # Test one run
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
event_tableList <- list()
agent_tableList <- list()

for (d in dirs){
  path <- paste0(d,statsfilename)
  event_path <- paste0(d,eventsfileName)
  agent_path <- paste0(d,agentsfileName)
  
  if (!file.exists(path)){
    print(paste0("File doesnt exist! ",path))
  }
  else{
    print(paste0("Loading ", d ))
    
    tryCatch({
      # Read the model.props for optional storing of parameter values
      propsRead <- fread(paste0(d,"/model.props"), fill=TRUE)
      props <- propsRead[,1]
      props$Value <- propsRead[,3]
      colnames(props)<-c("Name", "Value")
      
      # Filter out columns to reduce data in memory size
      #table <-  fread(path, select=colsToKeep)
      #table <-  fread(path)
      
      # Optionally store properties in the table for this run
      #table$opioid_treatment_enrollment_per_PY <- props[Name=="opioid_treatment_enrollment_per_PY"]$Value
      #table$opioid_scenario <- props[Name=="opioid_treatment_access_scenario"]$Value 
      #table$treatment_svr <- props[Name=="treatment_svr"]$Value
      
      experiment <- props[Name=="Experiment"]$Value
      
      # Provide a default experiment number if missing (for older runs w/o exp)
      if (identical(experiment, character(0))){
        experiment <- 0
      }
      
      #table$experiment <- experiment
      
      #tableList[[d]]  <- table
      
      event_table <- fread(event_path)
      agent_table <- fread(agent_path)
    
      # Create a run col for agent table using the event table value
      agent_table$run <- event_table$run[1]
      agent_table$opioid_treatment_enrollment_per_PY <- props[Name=="opioid_treatment_enrollment_per_PY"]$Value
      agent_table$opioid_scenario <- props[Name=="opioid_treatment_access_scenario"]$Value
      
      agent_table$experiment <- experiment
      
      event_tableList[[d]]  <- event_table
      agent_tableList[[d]]  <- agent_table
      
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

#dt <- rbindlist(tableList)               # Stack the list of tables into a single DT
tableList <- NULL                         # clear mem
dt_events <- rbindlist(event_tableList)   # Stack the list of tables into a single DT
event_tableList <- NULL                   # clear mem
dt_agents <- rbindlist(agent_tableList)   # Stack the list of tables into a single DT
agent_tableList <- NULL                   # clear mem

startYear <- 2010   # First year of simulation
endYear <- 2030     # Last year to look at for results (sim may run longer)

# Convert the simulation day tick to the simulated year
dayToYear <- function(firstYear, day) firstYear + floor((day)/365)
#dt[, "Year" := dayToYear(tick, firstYear=startYear-1)]
dt_events[, "Year" := dayToYear(tick, firstYear=startYear-1)]
dt_agents[, "Year" := dayToYear(Tick, firstYear=startYear-1)]  # Note cap Tick

# pre-subset the treatment events since the complete event log is huge
setkey(dt_events, event_type) 
stop_treat_events <- dt_events[.('STOPPED_OPIOID_TREATMENT')]
#start_treat_events <- dt_events[.('STARTED_OPIOID_TREATMENT')]

# The stop treatment events "other" column has colon-delimited info with the drug and treatment duration
stop_treat_events[, c("Drug", "start_tick") := tstrsplit(other, ":", fixed=TRUE)]
stop_treat_events[, c("Id") := person_id]
stop_treat_events[, c("Duration") := tick - as.numeric(start_tick)]
stop_treat_events[, event_type := NULL]       # Don't need so NULL to save memory
stop_treat_events[, other := NULL]            # Don't need so NULL to save memory
stop_treat_events[, person_id := NULL]        # Don't need so NULL to save memory

# Join the events and agents table by (Id, run), and use the Year and tick from the event  
setkey(stop_treat_events, Id, run)

# Save initial & final agent population for a *****single run*****
# *** Tick 365 *** is the first tick due to burn-in
single_run_agents <- dt_agents[run == 1]
initial_agents <- single_run_agents[Tick == 0]

# Subtracting all agents with a deactivated event from the entire agent event list
#   for a single run will provide the remaining agents at final time

activated_events <- dt_events[.('ACTIVATED')]
# *** Tick 365 *** is the first tick due to burn-in
activated_events <- activated_events[run == 1 & tick > 0]   #*****single run*****
deactivated_events <- dt_events[.('DEACTIVATED')]
deactivated_events <- deactivated_events[run == 1]   #*****single run*****

# Sanity check - there should be about 32,000 agents active at a time
num_agents <- nrow(activated_events) - nrow(deactivated_events)

if (num_agents < 31900 || num_agents > 32100){
  warning("The number of agents should be approximately 32,000 but is ", num_agents)
}

deactivated_agent_ids <- deactivated_events$person_id
final_agents <- single_run_agents[!(Id %in% deactivated_agent_ids)]

fwrite(initial_agents, "initial_agents.csv")
fwrite(final_agents, "final_agents.csv")

# Copy (using data.table so it's not a simple object reference) agents and remove 
#  Tick,Year since we want the event times, not when agents are created
#sm_agents <- data.table(dt_agents)
#sm_agents[,Tick:=NULL]
#sm_agents[,Year:=NULL]

dt_agents[,Tick:=NULL]
dt_agents[,Year:=NULL]

setkey(dt_agents, Id, run)
stop_treat_events <- dt_agents[stop_treat_events, on=c("Id","run")]
stop_treat_events[, c("zipcode") := `Zip Code`]

# Summary of treatment durations, averaged by run, for each zipcode and scenario
treatmentSummary <- stop_treat_events[, list(Duration_mean=mean(Duration), Duration_sd=sd(Duration)), 
                                      by=list(Year, opioid_treatment_enrollment_per_PY, 
                                              opioid_scenario, Drug, zipcode, experiment)]

# Same as above, but also averaged over all zipcodes, so its a population mean
treatmentSummary_population <- stop_treat_events[, list(Duration_mean=mean(Duration), Duration_sd=sd(Duration)), 
                                      by=list(Year, opioid_treatment_enrollment_per_PY, 
                                              opioid_scenario, Drug, experiment)]

result_year <- 2030  # Year at which to compute results for treatment duration

setkey(treatmentSummary, Year)
year_result <- treatmentSummary[.(result_year)]
year_result[, "Zipcode Name":=as.factor(as.character(zipcode))]

setkey(treatmentSummary_population, Year)
year_result_population <- treatmentSummary_population[.(result_year)]

# Select case to graph from REAL, SCENARIO_1, SCENARIO_2
case_REAL <- year_result[opioid_scenario=='REAL' 
                    & opioid_treatment_enrollment_per_PY==0.075]

# Gather population level statistics to be used for normalization
case_REAL_population <- year_result_population[opioid_scenario=='REAL' 
                                    & opioid_treatment_enrollment_per_PY==0.075]

case_REAL_population$Duration_mean_population <- case_REAL_population$Duration_mean
case_REAL_population$Duration_sd_population <- case_REAL_population$Duration_sd
case_REAL_population[, Duration_mean := NULL]
case_REAL_population[, Duration_sd := NULL]

# Save the REAL scenario treatment durations for reference and to check that
#  the durations match the expected values from literature.
fwrite(case_REAL_population, "case_REAL_population_mean_treatment_durations.csv")

# Remove since the individual cases will have these cols
case_REAL_population[, Year := NULL]
case_REAL_population[, opioid_treatment_enrollment_per_PY := NULL]
case_REAL_population[, opioid_scenario := NULL]

# Calculate scores.  Score method 1 is relative to the population mean/sd, while
#  score method 2 is relative to the zip mean/sd

# The REAL case treatment duration mean and sd by zip code
case_REAL_zip_means <- case_REAL[, c("Drug","zipcode", "experiment", "Duration_mean", "Duration_sd")]
names(case_REAL_zip_means) <- c("Drug", "zipcode", "experiment", "Duration_mean_real", "Duration_sd_real")

# Normalize the case by the REAL population mean and sd
case_REAL <- case_REAL[case_REAL_population, on=c("Drug", "experiment")]
case_REAL[, "score 1" := (Duration_mean - Duration_mean_population) / Duration_sd_population]

# Normalize the case by the REAL population mean and sd
case_S1 <- year_result[opioid_scenario=='SCENARIO_1' & opioid_treatment_enrollment_per_PY==0.075]
case_S1 <- case_S1[case_REAL_population, on=c("Drug","experiment")]

case_S1 <- case_S1[case_REAL_zip_means, on=c("Drug","zipcode","experiment")]
case_S1[, "score 1" := (Duration_mean - Duration_mean_population) / Duration_sd_population]
case_S1[, "score 2" := (Duration_mean - Duration_mean_real) / Duration_sd_real]

# Normalize the case by the REAL population mean and sd
case_S2 <- year_result[opioid_scenario=='SCENARIO_2' & opioid_treatment_enrollment_per_PY==0.075]
case_S2 <- case_S2[case_REAL_population, on=c("Drug","experiment")]
case_S2 <- case_S2[case_REAL_zip_means, on=c("Drug","zipcode", "experiment")]
case_S2[, "score 1" := (Duration_mean - Duration_mean_population) / Duration_sd_population]
case_S2[, "score 2" := (Duration_mean - Duration_mean_real) / Duration_sd_real]

# Normalize the case by the REAL population mean and sd
case_S3 <- year_result[opioid_scenario=='SCENARIO_3' & opioid_treatment_enrollment_per_PY==0.075]
case_S3 <- case_S3[case_REAL_population, on=c("Drug","experiment")]
case_S3 <- case_S3[case_REAL_zip_means, on=c("Drug", "zipcode", "experiment")]
case_S3[, "score 1" := (Duration_mean - Duration_mean_population) / Duration_sd_population]
case_S3[, "score 2" := (Duration_mean - Duration_mean_real) / Duration_sd_real]

fwrite(case_REAL, file="hepcep_moud_real_duration.csv")
fwrite(case_S1, file="hepcep_moud_s1_duration.csv")
fwrite(case_S2, file="hepcep_moud_s2_duration.csv")
fwrite(case_S3, file="hepcep_moud_s3_duration.csv")


# Calculate new chronic infections

# pre-subset the treatment events since the complete event log is huge
new_chronic_events <- dt_events[.('CHRONIC')]
new_chronic_events[, c("Id") := person_id]
new_chronic_events[, event_type := NULL]       # Don't need so NULL to save memory
new_chronic_events[, other := NULL]            # Don't need so NULL to save memory
new_chronic_events[, person_id := NULL]        # Don't need so NULL to save memory

# Join the events and agents table by (Id, run), and use the Year and tick from the event  
setkey(new_chronic_events, Id, run)

# Copy agents and remove Tick,Year since we want the event times, not when agents are created
#sm_agents <- data.table(dt_agents)
#sm_agents[,Tick:=NULL]
#sm_agents[,Year:=NULL]

#setkey(sm_agents, Id, run)

new_chronic_events <- dt_agents[new_chronic_events, on=c("Id","run")]
new_chronic_events[, c("zipcode") := `Zip Code`]
new_chronic_events[, c("count") := 1]

# Total new chronic counts per year, run, scenario, etc
# Note that here there are no Drug information for infection events
new_chronic_totals <- new_chronic_events[, list(new_chronic_total=sum(count)), 
                                      by=list(Year, opioid_treatment_enrollment_per_PY, 
                                              opioid_scenario, zipcode, run, experiment)]

# Average the new chronic counts over runs and group by zip, scenario
new_chronic_summary <- new_chronic_totals[, list(new_chronic_mean=mean(new_chronic_total), 
                                                 new_chronic_sd=sd(new_chronic_total)), 
                                         by=list(Year, opioid_treatment_enrollment_per_PY, 
                                                 opioid_scenario, zipcode, experiment)]

# TODO Population averages?  Dosent make sense as normalizing by pop average will be too small

# Subset results on result year to show
setkey(new_chronic_summary, Year)
new_chronic_year_result <- new_chronic_summary[.(result_year)]
new_chronic_year_result[, "Zipcode Name":=as.factor(as.character(zipcode))]

# REAL case stats, mean and sd of new chronic infections per zip code (over runs)
new_chronic_case_REAL <- new_chronic_year_result[opioid_scenario=='REAL' 
                         & opioid_treatment_enrollment_per_PY==0.075]

new_chronic_case_REAL$new_chronic_mean_REAL <- new_chronic_case_REAL$new_chronic_mean
new_chronic_case_REAL$new_chronic_sd_REAL <- new_chronic_case_REAL$new_chronic_sd
new_chronic_case_REAL[, new_chronic_mean := NULL]
new_chronic_case_REAL[, new_chronic_sd := NULL]

# Remove since the individual cases will have these cols
new_chronic_case_REAL[, Year := NULL]
new_chronic_case_REAL[, opioid_treatment_enrollment_per_PY := NULL]
new_chronic_case_REAL[, opioid_scenario := NULL]
new_chronic_case_REAL[, "Zipcode Name" := NULL]

# Normalize the case by the REAL mean and sd
new_chronic_case_S1 <- new_chronic_year_result[opioid_scenario=='SCENARIO_1' 
                       & opioid_treatment_enrollment_per_PY==0.075]
new_chronic_case_S1 <- new_chronic_case_S1[new_chronic_case_REAL, on=c("zipcode", "experiment")]
new_chronic_case_S1[, "score" := (new_chronic_mean - new_chronic_mean_REAL) / new_chronic_sd_REAL]
new_chronic_case_S1[is.na(score) | is.nan(score) | is.infinite(score)]$score <- 0

# Normalize the case by the REAL mean and sd
new_chronic_case_S2 <- new_chronic_year_result[opioid_scenario=='SCENARIO_2' 
                       & opioid_treatment_enrollment_per_PY==0.075]
new_chronic_case_S2 <- new_chronic_case_S2[new_chronic_case_REAL, on=c("zipcode", "experiment")]
new_chronic_case_S2[, "score" := (new_chronic_mean - new_chronic_mean_REAL) / new_chronic_sd_REAL]
new_chronic_case_S2[is.na(score) | is.nan(score) | is.infinite(score)]$score <- 0

# Normalize the case by the REAL mean and sd
new_chronic_case_S3 <- new_chronic_year_result[opioid_scenario=='SCENARIO_3' 
                                               & opioid_treatment_enrollment_per_PY==0.075]
new_chronic_case_S3 <- new_chronic_case_S3[new_chronic_case_REAL, on=c("zipcode", "experiment")]
new_chronic_case_S3[, "score" := (new_chronic_mean - new_chronic_mean_REAL) / new_chronic_sd_REAL]
new_chronic_case_S3[is.na(score) | is.nan(score) | is.infinite(score)]$score <- 0

fwrite(new_chronic_case_REAL, file="hepcep_moud_real_newchronic.csv")
fwrite(new_chronic_case_S1, file="hepcep_moud_s1_newchronic.csv")
fwrite(new_chronic_case_S2, file="hepcep_moud_s2_newchronic.csv")
fwrite(new_chronic_case_S3, file="hepcep_moud_s3_newchronic.csv")


