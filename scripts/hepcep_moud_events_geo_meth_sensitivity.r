#
# Analysis of hepcep model outputs for MOUD experiments.
#
#  This script generates CSV files with scores for treatment duration and
#    new chronic infections by zip code and MOUD drug type.
#
# Eric Tatara
#


library(data.table)
library(ggplot2)
library(gganimate)

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
      
      agent_table$methadone_p_close <- props[Name=="methadone_p_close"]$Value
      agent_table$methadone_p_far <- props[Name=="methadone_p_far"]$Value
      
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

stop_treat_events[, methadone_p_close := as.numeric(methadone_p_close)]
stop_treat_events[, methadone_p_far := as.numeric(methadone_p_far)]

stop_treat_events_methadone <- stop_treat_events[Drug == "methadone"]

# Mean duration by zip code
durations_mean <- stop_treat_events_methadone[, list(Duration_mean=mean(Duration), Duration_sd=sd(Duration)), 
                                      by=list(Year, zipcode,  methadone_p_close, methadone_p_far)]

# Mean duration across all zip codes
durations_mean_all_zip <- stop_treat_events_methadone[, list(Duration_mean_all=mean(Duration), 
                                                             Duration_sd_all=sd(Duration)), 
                                              by=list(Year,  methadone_p_close, methadone_p_far)]


result_year <- 2030  # Year at which to compute results for treatment duration
durations_mean_result_year <- durations_mean[Year == result_year]

durations_mean_all_zipresult_year <- durations_mean_all_zip[Year == result_year]

#stop_treat_events <- dt_agents[stop_treat_events, on=c("Id","run")]

durations_mean_result_year = durations_mean_all_zipresult_year[durations_mean_result_year, 
                                                               on=c("methadone_p_close", "methadone_p_far")]

durations_mean_result_year_large <- durations_mean[Duration_mean > 200]

foo <- as.data.frame(table(durations_mean_result_year_large$zipcode))

result_zip <- 60602
durations_mean_result_zip <- durations_mean_result_year[zipcode == result_zip]

p <- ggplot(durations_mean_result_zip) +
  geom_point(aes(x=methadone_p_close, y=Duration_mean), size=2, alpha=0.95) +
  
  scale_x_continuous(limits = c(0.95,1.0)) +
#  scale_colour_manual(values=cbPalette) +
#  scale_fill_manual(values=cbPalette) + 
#  labs(y="Deaths", x="interval", title="Chicago (interval) Deaths") +
  
  theme_bw() +
  theme(axis.text=element_text(size=14),axis.title=element_text(size=14),legend.text=element_text(size=14)) 
  
#  facet_wrap(vars(naloxone_use), labeller = label_both)

#theme(legend.position = "none")

show(p)


target_meth_p_close <- durations_mean_result_year[methadone_p_close == 0.98]

q <- ggplot(target_meth_p_close) +
  geom_point(aes(x=Duration_mean, y=zipcode), size=1, alpha=0.95) +
  
  geom_text(aes(x=Duration_mean, y=zipcode, label=zipcode), check_overlap = TRUE) +
  
  geom_vline(xintercept = 150, color="red") +
  
#  scale_x_continuous(limits = c(0.95,1.0)) +
  #  scale_colour_manual(values=cbPalette) +
  #  scale_fill_manual(values=cbPalette) + 
  #  labs(y="Deaths", x="interval", title="Chicago (interval) Deaths") +
  
  theme_bw() +
  theme(axis.text=element_text(size=14),axis.title=element_text(size=14),legend.text=element_text(size=14)) 

#  facet_wrap(vars(naloxone_use), labeller = label_both)

#theme(legend.position = "none")

show(q)



p1 <- ggplot(durations_mean_result_year) + 

  geom_point(aes(x=Duration_mean, y=zipcode, color=zipcode), size=1) +
  geom_vline(aes(xintercept = 150), color="blue") +
  geom_vline(aes(xintercept = Duration_mean_all), color="red") +
  
  scale_x_continuous(limits = c(0, 3000)) +
  
  transition_states(states=methadone_p_close, transition_length = 2, state_length = 1) +
  labs(title = "M_p.close: {closest_state}")

  #transition_time(methadone_p_close) + labs(title = "M_p.close: {frame_time}") #+ view_follow(fixed_y = T) +

#  ease_aes('linear')+
#  enter_fade()+
#  exit_fade()

animate(p1, type = "cairo", nframes = 120)
anim_save("meth_p_close.gif")


