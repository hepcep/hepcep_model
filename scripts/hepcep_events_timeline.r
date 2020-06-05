library(data.table)
library(ggplot2)
library(tidyverse)

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

dirs <- c("./run_1599")   # Only need one run for an agent event timeline
#dirs <- list.dirs (path=".", recursive=FALSE)

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
      table$treatment_enrollment_per_PY <- props[Name=="treatment_enrollment_per_PY"]$Value
      
      table$treatment_svr <- props[Name=="treatment_svr"]$Value
      
      tableList[[d]]  <- table
      
      event_table <- fread(event_path)
      agent_table <- fread(agent_path)
    
      # Create a run col for agent table using the event table value
      agent_table$run <- event_table$run[1]
      
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


# Find the persons with the most events of interest
#start_treat_events <- dt_events[.('STARTED_TREATMENT')]
chronic_infect_events <- dt_events[.('CHRONIC')]   # new chronic infections

personFreq <- as.data.table(table(chronic_infect_events$person_id))
max_events = max(personFreq$N)
personFreq <- personFreq[N == max_events] 

# Now we only have persons treated max times
# Grab a random ID
an_id <- personFreq[1]$V1

one_person_events <- dt_events[person_id == an_id]

fwrite(one_person_events, "one_person_events.csv")

gantt_data <- fread("one_person_events.csv")

gantt_data[, date := as.Date(date)]
gantt_data[, Activity := as.factor(Activity)]
gantt_data[, Detail := as.factor(Detail)]

start_date <- as.Date("2020-01-01")
end_date   <- as.Date("2025-01-01")

actcols <- c("#548235", "#DC143C", "#FF8C00", "#1E90FF")

p <- ggplot(gantt_data, aes(date, Activity, color = Detail, group=Item)) +
      geom_line(size=10) +
      coord_fixed(ratio = 100) +
      scale_x_date(breaks=seq.Date(start_date, end_date, by="year"),
                   labels=seq(2020,2025,1),
                   limits=c(start_date, end_date)) +
     
      scale_color_manual(values=actcols, name=NULL) + 
     
      labs(x="Date", y=NULL, title="Agent activity timeline") + 
      theme(
          legend.position="bottom", legend.text=element_text(size=14, margin = margin(r = 20, unit = "pt")), 
          
          axis.text=element_text(size=14), axis.title=element_text(size=14), legend.key.size = unit(0.5, "cm"),
          title=element_text(size=14),
          panel.border=element_rect(colour="gray", fill=NA))
  
show(p)
ggsave("Agent timeline.png", plot=p, width=10, height=8)


