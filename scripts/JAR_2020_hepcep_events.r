#
# Analysis of hepcep model outputs for JAR 2020 Paper.
# Generates the treatment count histogram for the allowed-retreatment scenario.

# This script should be run on the retreatemtn allowed scenario 
# (retreat_svr_chronic_01)
#
# Script should be run in the experiment directory containing the run subfolders.
#
# Eric Tatara
#
library(data.table)
library(ggplot2)
library(scales)

# Load all of the stats files that exist in an experiments dir
eventsfileName <- "/events.csv"
statsfilename <- "/stats.csv"
dirs <- list.dirs (path=".", recursive=FALSE)

# TODO put the run number in the event log
run <- 1
startYear <- 2010   # First year of simulation
endYear <- 2030   
dayToYear <- function(firstYear, day) firstYear + floor((day)/365)

tableList <- list()
for (d in dirs){
  path <- paste0(d,eventsfileName)
  statsPath <- paste0(d,statsfilename)
  
  if (!file.exists(path)){
    print(paste0("File doesnt exist! ",path))
  }
  else if (!file.exists(statsPath)){
    print(paste0("File doesnt exist! ",statsPath))
  }
  else{
    print(paste0("Loading ", path, " and ", statsPath ))
    
    tryCatch({
      # Read the model.props for optional storing of parameter values
      propsRead <- fread(paste0(d,"/model.props"), fill=TRUE)
      props <- propsRead[,1]
      props$Value <- propsRead[,3]
      colnames(props)<-c("Name", "Value")
      
      # Read the event and stats into tables
      evTable <-  fread(path)
#      statsTable <- fread(statsPath)
    
      evTable$Year <- unlist(lapply(evTable$tick, dayToYear, firstYear=startYear-1))
      
      # Process events 
      if (nrow(evTable) == 0){
        print(paste0("No treatment event in log for: ", path))
        next
      }
      
      #TODO will need to filter on event type if new events added
      
      # Get the frequency of each person occurence only during the study period (start year to end year)
      personFreq <- as.data.frame(table(evTable[Year < endYear]$person_id))
      
      # Get the frequency of the occurence frequency
      retreatFreq <- as.data.frame(table(personFreq$Freq))
      
      # Store in results table
      resultsTable <- as.data.table(retreatFreq)
      setnames(resultsTable, old="Var1", new="Count")
      
      # Optionally store properties in the table for this run
      resultsTable$treatment_enrollment_per_PY <-as.numeric( props[Name=="treatment_enrollment_per_PY"]$Value)
      
      resultsTable$treatment_svr <- as.numeric(props[Name=="treatment_svr"]$Value)
      
      # Treatment enrollment probabilities
#      resultsTable$tep_HRP         <- props[Name=="treatment_enrollment_probability_HRP"]$Value
#      resultsTable$tep_fullnetwork <- props[Name=="treatment_enrollment_probability_fullnetwork"]$Value
#      resultsTable$tep_inpartner   <- props[Name=="treatment_enrollment_probability_inpartner"]$Value
#      resultsTable$tep_outpartner  <- props[Name=="treatment_enrollment_probability_outpartner"]$Value
#      resultsTable$tep_unbiased    <- props[Name=="treatment_enrollment_probability_unbiased"]$Value
      
      # Process stats
      
#      totalActivations <- sum(statsTable$activations_daily)
#      resultsTable$totalActivations <- totalActivations
 
      resultsTable$run <- run
      
      tableList[[d]]  <- resultsTable  
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
  
  run <- run + 1
}

dt <- rbindlist(tableList, fill=TRUE)  # Stack the list of tables into a single DT
tableList <- NULL           # clear mem

# Generate Treatment count histogram - need to run separately for each SVR
treatmentSummary <- dt[, list(mean=mean(Freq), sd=sd(Freq)), 
                                  by=list(Count,treatment_enrollment_per_PY,treatment_svr)]

# Specify the SVR to graph
treatmentSummary <- treatmentSummary[Count %in% c(1,2,3,4,5,6,7)] # & treatment_svr %in% c(0.9)]

largestCount <- max(as.numeric(treatmentSummary$Count))
treatmentSummary$Count <- factor(treatmentSummary$Count, levels=seq(1,largestCount,1))
treatmentSummary$SVR <- factor (100 * as.numeric(treatmentSummary$treatment_svr))
treatmentSummary$treatment_enrollment_per_PY <- factor (100 * treatmentSummary$treatment_enrollment_per_PY)

#treatmentSummary$treatment_enrollment_per_PY <- factor(percent(
#  treatmentSummary$treatment_enrollment_per_P, accuracy = .1))

p <- ggplot(treatmentSummary, aes(x=Count, y=mean, fill=treatment_enrollment_per_PY)) +
  geom_bar(position=position_dodge(), stat="identity", color="black", alpha=0.5) +

#  geom_errorbar(aes(x=Count, ymin=mean-3*sd, ymax=mean+3*sd),width=.15,position = position_dodge(width = 0.9)) + 
  
#  scale_x_discrete(limits=(c(1,2,3,4,5,6,7))) +

  scale_y_log10(limits=c(1,10000), 
                breaks = scales::trans_breaks("log10", function(x) 10^x),
                labels = scales::trans_format("log10", scales::math_format(10^.x))) +
  
  annotation_logticks(sides="l") +
  
  facet_wrap(vars(SVR), labeller = label_both) +
  
  labs(y="Frequency", x="Times Treated", title="") +
  theme_bw() +
#  theme_minimal() +
  guides(fill=guide_legend(title="Enrollment %")) +
  theme(text = element_text(size=16), 
        legend.position = c(.4, .35), 
        legend.text=element_text(size=16),
        legend.background = element_rect(fill="gray95", size=.5, linetype="solid"),
        panel.border = element_rect(colour = "gray", fill=NA)) +
  theme(axis.text=element_text(size=16),axis.title=element_text(size=16)) 
#show(p)
ggsave("Treatment Counts Distribution svr ALL.png", plot=p, width=10, height=8)


#write.table(treatmentSummary, file="treatment_summary svr 0.9")


# Write the treatment count table to be nicely formatted in Excel/Word
DAA_cost <- 25 # treatment cost in $1,000
treatmentSummary <- dt[, list(freq_mean=mean(Freq), freq_sd=sd(Freq)), 
                       by=list(Count,treatment_enrollment_per_PY,treatment_svr)]

largestCount <- max(as.numeric(treatmentSummary$Count))
treatmentSummary$Count <- factor(treatmentSummary$Count, levels=seq(1,largestCount,1))

treatmentCounts <- treatmentSummary[, list(Count=Count, freq_mean=round(freq_mean)), 
                       by=list(treatment_enrollment_per_PY,treatment_svr)]
write.table(treatmentCounts, file="treatment_counts.csv", sep="," , row.names = FALSE)


