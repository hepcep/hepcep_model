#
# Analysis of hepcep model outputs of needle sharing events by zipcode
#
#  
#
# Eric Tatara
#


library(data.table)

needle_sharing_filename <- "/needle_sharing.csv"

std <- function(x) sd(x)/sqrt(length(x))

dt <- NULL
table <- NULL

result_year <- 2030  # Year at which to compute results for treatment duration
startYear <- 2010    # First year of simulation
endYear <- 2030      # Last year to look at for results (sim may run longer)

# Convert the simulation day tick to the simulated year
dayToYear <- function(firstYear, day) firstYear + floor((day)/365)

#dirs <- c("./run_0")   # Test one run
dirs <- list.dirs (path=".", recursive=FALSE)

tableList <- list()

for (d in dirs){
  path <- paste0(d,needle_sharing_filename)
 
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

      # The needle sharing data is a matrix of tick x zipcode
      dt <- fread(path)
          
      # Filter out everything but the year(s) of interest
      dt[, "Year" := dayToYear(Tick, firstYear=startYear-1)]
      dt <- dt[Year == result_year] 
      
      # Reshape the matrix into a dataframe/table
      zipcodes <- names(dt)[3:(ncol(dt)-5)]
      dt_reshape = melt(dt, id.vars = c("Tick", "Run", "Year"),
                        measure.vars = zipcodes, variable.name = "zipcode", value.name = "needle_sharing_episodes")
      
      dt_sum <- dt_reshape[, .(total=sum(needle_sharing_episodes)), by=.(zipcode,Year,Run)]
      
     
      # Optionally store properties in the table for this run
      dt_sum$opioid_treatment_enrollment_per_PY <- props[Name=="opioid_treatment_enrollment_per_PY"]$Value
      dt_sum$opioid_scenario <- props[Name=="opioid_treatment_access_scenario"]$Value 
      
      dt_sum$experiment <- props[Name=="Experiment"]$Value
      dt_sum$name <- props[Name=="name"]$Value
      
      # If max distance thresholding is used
      max_threshold <- props[Name=="opioid_treatment_urban_max_threshold"]$Value
      dt_sum$maxthreshold = max_threshold > 0
      
      dt_sum$penalty <-  props[Name=="penalty"]$Value
      
      tableList[[d]]  <- dt_sum
      
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

dt_all <- rbindlist(tableList)  # Stack the list of tables into a single DT
tableList <- NULL               # clear mem

dt_all_summary <- dt_all[, list(needle_sharing_mean=mean(total), 
                                needle_sharing_sd=sd(total)), 
                                          by=list(Year, opioid_treatment_enrollment_per_PY, 
                                                  opioid_scenario, zipcode, experiment, 
                                                  name, penalty, maxthreshold)]

fwrite(dt_all_summary, file="needle_sharing.csv")

