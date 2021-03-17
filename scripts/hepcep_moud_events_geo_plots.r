#
# Analysis of hepcep model outputs for MOUD experiments with GIS plots.
#
#  This script generates GIS plots with scores for treatment duration and
#    new chronic infections by zip code and MOUD drug type.
#
# Eric Tatara
#

library(data.table)
library(ggplot2)
library(rgdal)
library(broom)
library(sp)
library(rgeos)
library(mapdata)

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

#dt <- rbindlist(tableList)  # Stack the list of tables into a single DT
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
sm_agents <- data.table(dt_agents)
sm_agents[,Tick:=NULL]
sm_agents[,Year:=NULL]

setkey(sm_agents, Id, run)
stop_treat_events <- sm_agents[stop_treat_events, on=c("Id","run")]
stop_treat_events[, c("zipcode") := `Zip Code`]

# Summary of treatment durations, averaged by run, for each zipcode and scenario
treatmentSummary <- stop_treat_events[, list(Duration_mean=mean(Duration), Duration_sd=sd(Duration)), 
                                      by=list(Year,opioid_treatment_enrollment_per_PY, 
                                              opioid_scenario,Drug,zipcode,experiment)]

# Same as above, but also averaged over all zipcodes, so its a population mean
treatmentSummary_population <- stop_treat_events[, list(Duration_mean=mean(Duration), Duration_sd=sd(Duration)), 
                                      by=list(Year,opioid_treatment_enrollment_per_PY, 
                                              opioid_scenario,Drug,experiment)]

result_year <- 2030

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
case_REAL_population$Duration_mean <- NULL
case_REAL_population$Duration_sd <- NULL

# Save the REAL scenario treatment durations for reference and to check that
#  the durations match the expected values from literature.
fwrite(case_REAL_population, "case_REAL_population_mean_treatment_durations.csv")

# Calculate scores.  Score method 1 is relative to the population mean/sd, while
#  score method 2 is relative to the zip mean/sd

# The REAL case treatment duration mean and sd by zip code
case_REAL_zip_means <- case_REAL[, c("Drug", "zipcode", "Duration_mean", "Duration_sd")]
names(case_REAL_zip_means) <- c("Drug", "zipcode", "Duration_mean_real", "Duration_sd_real")

# Normalize the case by the REAL population mean and sd
case_REAL <- case_REAL[case_REAL_population, on=c("Drug")]
case_REAL[, "score 1" := (Duration_mean - Duration_mean_population) / Duration_sd_population]

# Normalize the case by the REAL population mean and sd
case_S1 <- year_result[opioid_scenario=='SCENARIO_1' & opioid_treatment_enrollment_per_PY==0.075]
case_S1 <- case_S1[case_REAL_population, on=c("Drug")]
case_S1 <- case_S1[case_REAL_zip_means, on=c("Drug","zipcode")]
case_S1[, "score 1" := (Duration_mean - Duration_mean_population) / Duration_sd_population]
case_S1[, "score 2" := (Duration_mean - Duration_mean_real) / Duration_sd_real]

# Normalize the case by the REAL population mean and sd
case_S2 <- year_result[opioid_scenario=='SCENARIO_2' & opioid_treatment_enrollment_per_PY==0.075]
case_S2 <- case_S2[case_REAL_population, on=c("Drug")]
case_S2 <- case_S2[case_REAL_zip_means, on=c("Drug","zipcode")]
case_S2[, "score 1" := (Duration_mean - Duration_mean_population) / Duration_sd_population]
case_S2[, "score 2" := (Duration_mean - Duration_mean_real) / Duration_sd_real]

# Normalize the case by the REAL population mean and sd
case_S3 <- year_result[opioid_scenario=='SCENARIO_3' & opioid_treatment_enrollment_per_PY==0.075]
case_S3 <- case_S3[case_REAL_population, on=c("Drug")]
case_S3 <- case_S3[case_REAL_zip_means, on=c("Drug","zipcode")]
case_S3[, "score 1" := (Duration_mean - Duration_mean_population) / Duration_sd_population]
case_S3[, "score 2" := (Duration_mean - Duration_mean_real) / Duration_sd_real]

fwrite(case_REAL, file="hepcep_moud_real_duration_new.csv")
fwrite(case_S1, file="hepcep_moud_s1_duration_new.csv")
fwrite(case_S2, file="hepcep_moud_s2_duration_new.csv")
fwrite(case_S3, file="hepcep_moud_s3_duration_new.csv")


# Use the real case, or...
case <- case_S2

# ...calculate the case difference from real
#case <- case_S2[case_REAL, on=c("Drug","zipcode")]
#case[, "Duration_Difference" := i.Duration_mean - Duration_mean]
#case <- case[!(is.na(case$Duration_Difference)) ]

# Create maps
zipcode_folder <- "../../gisdata/illinois_zips"
zipcode_file <- "zt17_d00"
zipcode_layer <- readOGR(dsn=zipcode_folder, layer=zipcode_file)

#zipcode_layer <- gSimplify(zipcode_layer, tol = 1, topologyPreserve = TRUE)

#spatial_df <- merge(zipcode_layer, case, by.x = "NAME", by.y = "Zipcode Name", duplicateGeoms = TRUE)
#spatial_df <- spatial_df[!is.na(spatial_df@data$zipcode),] # remove zipcodes without corresponding model data

spatial_df <- zipcode_layer

spatial_df_tidy <- tidy(spatial_df)
spatial_df$id <- row.names(spatial_df)
#spatial_df_tidy <- left_join(spatial_df_tidy, spatial_df@data)

# Combine the read shapefile polygon geoms and attributes into a single data table
spatial_geo <- as.data.table(spatial_df_tidy)   # Zipcode polygons
spatial_data <- as.data.table(spatial_df@data)  # Zipcode feature attributes
combined_spatial_data <- spatial_data[spatial_geo, on=c("id")]

# Create a subset that only incldues zip codes with corresponding model results
setkey(combined_spatial_data, NAME)
result_zips <- as.factor(unique(dt_agents$`Zip Code`))  # zip codes in the model results
combined_spatial_data_subset <- combined_spatial_data[.(result_zips)]

# Merge with the model results
combined_spatial_data_subset[, "zipcode" := as.numeric(as.character(NAME))]
plot_data <- case[combined_spatial_data_subset, on=c("zipcode"), allow.cartesian=TRUE]

# Remove Drug=NA
plot_data <- plot_data[!(is.na(plot_data$Drug)) ]

# State boundary for visualization
states <- as.data.table(map_data("state"))
illinois <- states[region=="illinois"]

p <- ggplot() + 
  geom_polygon(data = plot_data, 
               aes(x=long, y=lat, group=group, fill=score), color="black", size=0.25) + 
  geom_polygon(data = illinois, 
               aes(x=long, y=lat, group=group), fill="NA", color="black", size=1.0) + 
#  coord_fixed(1.3) +
  coord_fixed(xlim = c(-89.5, -87.5),  ylim = c(41, 42.5), ratio = 1.3) + # zoom Chicago
#  theme_void() +
  theme_bw() +
  
  facet_wrap(vars(Drug), labeller = label_both) +
#  scale_fill_gradientn(colours = rainbow(30))
#  scale_fill_gradient(low="blue", high="red")
#  theme(plot.margin=unit(c(1,1,1.1,1),"points")) +
  
  scale_fill_gradient2(low="red", mid="white", high="green")

ggsave("MOUD_map_S2_zoom.png", plot=p, width=10, height=8)


# Calculate new chronic infections

# pre-subset the treatment events since the complete event log is huge
setkey(dt_events, event_type) 
new_chronic_events <- dt_events[.('CHRONIC')]
new_chronic_events[, other:=NULL]
new_chronic_events[, c("Id") := person_id]

# Join the events and agents table by (Id, run), and use the Year and tick from the event  
setkey(new_chronic_events, Id, run)

# Copy agents and remove Tick,Year since we want the event times, not when agents are created
sm_agents <- data.table(dt_agents)
sm_agents[,Tick:=NULL]
sm_agents[,Year:=NULL]

setkey(sm_agents, Id, run)
new_chronic_events <- sm_agents[new_chronic_events, on=c("Id","run")]
new_chronic_events[, c("zipcode") := `Zip Code`]
new_chronic_events[, c("count") := 1]

# Total new chronic counts per year, run, scenario, etc
# Note that here there are no Drug information for infection events
new_chronic_totals <- new_chronic_events[, list(new_chronic_total=sum(count)), 
                                      by=list(Year,opioid_treatment_enrollment_per_PY, 
                                              opioid_scenario,zipcode,run)]

# Average the new chronic counts over runs and group by zip, scenario
new_chronic_summary <- new_chronic_totals[, list(new_chronic_mean=mean(new_chronic_total), new_chronic_sd=sd(new_chronic_total)), 
                                         by=list(Year,opioid_treatment_enrollment_per_PY, 
                                                 opioid_scenario,zipcode)]

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
new_chronic_case_REAL$new_chronic_mean <- NULL
new_chronic_case_REAL$new_chronic_sd <- NULL


# Normalize the case by the REAL population mean and sd
new_chronic_case_S1 <- new_chronic_year_result[opioid_scenario=='SCENARIO_1' 
                       & opioid_treatment_enrollment_per_PY==0.075]
new_chronic_case_S1 <- new_chronic_case_S1[new_chronic_case_REAL, on=c("zipcode")]
new_chronic_case_S1[, "score" := (new_chronic_mean - new_chronic_mean_REAL) / new_chronic_sd_REAL]
new_chronic_case_S1[is.na(score) | is.nan(score) | is.infinite(score)]$score <- 0

# Normalize the case by the REAL population mean and sd
new_chronic_case_S2 <- new_chronic_year_result[opioid_scenario=='SCENARIO_2' 
                       & opioid_treatment_enrollment_per_PY==0.075]
new_chronic_case_S2 <- new_chronic_case_S2[new_chronic_case_REAL, on=c("zipcode")]
new_chronic_case_S2[, "score" := (new_chronic_mean - new_chronic_mean_REAL) / new_chronic_sd_REAL]
new_chronic_case_S2[is.na(score) | is.nan(score) | is.infinite(score)]$score <- 0

# Normalize the case by the REAL population mean and sd
new_chronic_case_S3 <- new_chronic_year_result[opioid_scenario=='SCENARIO_3' 
                                               & opioid_treatment_enrollment_per_PY==0.075]
new_chronic_case_S3 <- new_chronic_case_S3[new_chronic_case_REAL, on=c("zipcode")]
new_chronic_case_S3[, "score" := (new_chronic_mean - new_chronic_mean_REAL) / new_chronic_sd_REAL]
new_chronic_case_S3[is.na(score) | is.nan(score) | is.infinite(score)]$score <- 0

fwrite(new_chronic_case_REAL, file="hepcep_moud_real_newchronic.csv")
fwrite(new_chronic_case_S1, file="hepcep_moud_s1_newchronic.csv")
fwrite(new_chronic_case_S2, file="hepcep_moud_s2_newchronic.csv")
fwrite(new_chronic_case_S3, file="hepcep_moud_s3_newchronic.csv")

# Use the real case, or...
case <- new_chronic_case_S2

#result_zips <- unique(case$`Zipcode Name`)
combined_spatial_data_subset <- combined_spatial_data[.(result_zips)]

# Merge with the model results
combined_spatial_data_subset[, "zipcode" := as.numeric(as.character(NAME))]
plot_data <- case[combined_spatial_data_subset, on=c("zipcode"), allow.cartesian=TRUE]

p <- ggplot() + 
  geom_polygon(data = plot_data, 
               aes(x=long, y=lat, group=group, fill=score), color="black", size=0.25) + 
  geom_polygon(data = illinois, 
               aes(x=long, y=lat, group=group), fill="NA", color="black") + 
#  coord_fixed(1.3) +
  coord_fixed(xlim = c(-89.5, -87.5),  ylim = c(41, 42.5), ratio = 1.3) + # zoom Chicago
  #  theme_void() +
  theme_bw() +
  
  #  scale_fill_gradientn(colours = rainbow(30))
  #  scale_fill_gradient(low="blue", high="red")
  #  theme(plot.margin=unit(c(1,1,1.1,1),"points")) +
  
  scale_fill_gradient2(low="red", mid="white", high="green")

ggsave("MOUD_map_new_chronic_S2_zoom.png", plot=p, width=10, height=8)



