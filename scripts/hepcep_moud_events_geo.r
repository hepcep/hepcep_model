library(data.table)
library(ggplot2)

agentsfileName <- "/agents.csv"

agentsfileName <- "/agents.csv"
eventsfileName <- "/events.csv"
statsfilename <- "/stats.csv"

#stop_treat_events <- evTable[event_type == 'STOPPED_OPIOID_TREATMENT']
#start_treat_events <- evTable[event_type == 'STARTED_OPIOID_TREATMENT']

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
      
      event_table <- fread(event_path)
      agent_table <- fread(agent_path)
    
      # Create a run col for agent table using the event table value
      agent_table$run <- event_table$run[1]
      agent_table$opioid_treatment_enrollment_per_PY <- props[Name=="opioid_treatment_enrollment_per_PY"]$Value
      agent_table$opioid_scenario <- props[Name=="opioid_treatment_access_scenario"]$Value 
      
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

dt <- rbindlist(tableList)  # Stack the list of tables into a single DT
dt_events <- rbindlist(event_tableList)  # Stack the list of tables into a single DT
dt_agents <- rbindlist(agent_tableList)  # Stack the list of tables into a single DT
tableList <- NULL           # clear mem
event_tableList <- NULL           # clear mem
agent_tableList <- NULL           # clear mem

startYear <- 2010   # First year of simulation
endYear <- 2030  

# Convert the simulation day tick to the simulated year
dayToYear <- function(firstYear, day) firstYear + floor((day)/365)
dt[, "Year" := dayToYear(tick, firstYear=startYear-1)]
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

# Join the events and agents table by (Id, run), and use the Year and tick from the event  
setkey(stop_treat_events, Id, run)

# Copy agents and remove Tick,Year since we want the event times, not when agents are created
sm_agents <- dt_agents
sm_agents[,Tick:=NULL]
sm_agents[,Year:=NULL]

setkey(sm_agents, Id, run)
stop_treat_events <- sm_agents[stop_treat_events, on=c("Id","run")]
stop_treat_events[, c("zipcode") := `Zip Code`]

treatmentSummary <- stop_treat_events[, list(mean=mean(Duration), sd=sd(Duration)), 
                                      by=list(Year,opioid_treatment_enrollment_per_PY, 
                                              opioid_scenario,Drug,zipcode)]

setkey(treatmentSummary, Year)
year_result <- treatmentSummary[.(2030)]











