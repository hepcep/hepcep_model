#
# Check the hepcep model output variance
#
# Eric Tatara
#
library(data.table)

dt <- NULL
table <- NULL

# Load all of the stats files that exist in an experiments dir
fileName <- "/stats.csv"
dirs <- list.dirs (path=".", recursive=FALSE)

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
      
      table <-  fread(path)
      
      # Optionally store properties in the table for this run
      table$treatment_enrollment_per_PY <- props[Name=="treatment_enrollment_per_PY"]$Value
      
      table$treatment_svr <- props[Name=="treatment_svr"]$Value
      
      table$seed <- props[Name=="random.seed"]$Value
      
      
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

# check the standard deviation of prevalence and group by random seed.
# We expect that the variance of each seed group to be zero.
deviations <- dt[tick==700, .(sd=sd(prevalence_ALL)), by=list(seed)]
show(deviations)

one_group <- dt[tick==700 & seed==2]

# Manually change into single run folders to inspect the event log

#events <-  fread("events.csv")
#events3_t1 <- events[tick==1]

all_group <- dt[tick==700, .(tick=tick, run=run, seed=seed, prev=prevalence_ALL, pcount=population_ALL)]

