library(data.table)


# Load all of the stats files that exist in an experiments dir
eventsfileName <- "/events_16.csv"

# dir
d <- '.'

path <- paste0(d,eventsfileName)

# Read the event and stats into tables
evTable <-  fread(path)

stop_treat_events <- evTable[event_type == 'STOPPED_OPIOID_TREATMENT']
start_treat_events <- evTable[event_type == 'STARTED_OPIOID_TREATMENT']

tableList <- list()

for (r in 1:nrow(stop_treat_events)){
  
  row <- stop_treat_events[r,]
  
  stop_tick <- row$tick
  pid <- row$person_id
  drug <- row$other
  
  # all start ticks for this person & drug BEFORE the stop tick
  ts <- start_treat_events[person_id == pid & other == drug & tick < stop_tick]$tick
  
  # the start tick of this event must be the largest of found start ticks.
  start_tick <- max(ts)
  
#  print(paste0(drug, " ", (stop_tick - start_tick)))
 
  tableList[[r]]  <- data.frame(drug=drug, duration = (stop_tick-start_tick))   
}

dt <- rbindlist(tableList, fill=TRUE)  # Stack the list of tables into a single DT
tableList <- NULL           # clear mem

treatmentSummary <- dt[, list(mean=mean(duration), sd=sd(duration)), by=list(drug)]
