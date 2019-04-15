#
# Analysis of hepcep model outputs
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
      statsTable <- fread(statsPath)
      
      # Process events 
      if (nrow(evTable) == 0){
        print(paste0("No treatment event in log for: ", path))
        next
      }
      
      #TODO will need to filter on event type if new events added
      
      # Get the frequency of each person occurence
      personFreq <- as.data.frame(table(evTable$person_id))
      
      # Get the frequency of the occurence frequency
      retreatFreq <- as.data.frame(table(personFreq$Freq))
      
      # Store in results table
      resultsTable <- as.data.table(retreatFreq)
      setnames(resultsTable, old="Var1", new="Count")
      
      # Optionally store properties in the table for this run
      resultsTable$treatment_enrollment_per_PY <- props[Name=="treatment_enrollment_per_PY"]$Value
      
      # Treatment enrollment probabilities
#      resultsTable$tep_HRP         <- props[Name=="treatment_enrollment_probability_HRP"]$Value
#      resultsTable$tep_fullnetwork <- props[Name=="treatment_enrollment_probability_fullnetwork"]$Value
#      resultsTable$tep_inpartner   <- props[Name=="treatment_enrollment_probability_inpartner"]$Value
#      resultsTable$tep_outpartner  <- props[Name=="treatment_enrollment_probability_outpartner"]$Value
#      resultsTable$tep_unbiased    <- props[Name=="treatment_enrollment_probability_unbiased"]$Value
      
      # Process stats
      
      totalActivations <- sum(statsTable$activations_daily)
      
      resultsTable$totalActivations <- totalActivations
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


treatmentSummary <- dt[, list(mean=mean(Freq), sd=sd(Freq)), 
                                  by=list(Count,treatment_enrollment_per_PY)]

treatmentSummary <- treatmentSummary[Count %in% c(1,2,3,4,5,6,7)]

largestCount <- max(as.numeric(treatmentSummary$Count))
treatmentSummary$Count <- factor(treatmentSummary$Count, levels=seq(1,largestCount,1))

# Treatment count histogram
p <- ggplot(treatmentSummary, aes(x=Count, y=mean, fill=treatment_enrollment_per_PY)) +
  geom_bar(position=position_dodge(), stat="identity", color="black", alpha=0.5) +
#  geom_errorbar(aes(ymin=mean-sd, ymax=mean+sd), width=.2,position=position_dodge(.9)) +
#  scale_y_continuous(limits=c(0, 70), breaks=seq(0,70,10)) +
#  scale_x_discrete(limits=(c(1,2,3,4,5,6,7))) +

  scale_y_log10(limits=c(1,10000), 
                breaks = scales::trans_breaks("log10", function(x) 10^x),
                labels = scales::trans_format("log10", scales::math_format(10^.x))) +
  annotation_logticks() +
  labs(y="Frequency", x="Times Treated", title="Histogram of PWID Treatment Frequency") +
  theme_minimal() +
  guides(fill=guide_legend(title="Enrollment")) +
  theme(text = element_text(size=24), 
        legend.position = c(.8, .75), 
        legend.text=element_text(size=22),
        panel.border = element_rect(colour = "gray", fill=NA)) +
  theme(axis.text=element_text(size=26),axis.title=element_text(size=26)) 
show(p)
ggsave("Treatment Counts Distribution.png", plot=p, width=10, height=8)


# Treatment count pie chart
pieData <- treatmentSummary[treatment_enrollment_per_PY == 0.05]

zeroCount <- 100 - sum(pieData$mean)
pieData <- rbind(list(0,0.05,zeroCount,0), pieData)

p <- ggplot(pieData, aes(x="", y=mean, fill=Count)) +
  geom_bar(stat="identity", width=1, alpha=0.5, colour="black") +
  
  coord_polar("y", start=0) +
  
  scale_fill_brewer(palette="Blues",direction = -1) +
  
  geom_text(aes(y=50 , x=0.75, label = percent(mean[1]/100)), size=12) +
  geom_text(aes(y=10 , x=1.65, label = percent(mean[2]/100)), size=12) +
  geom_text(aes(y= 3 , x=1.65, label = percent(mean[3]/100)), size=12) +
  
#  geom_text(aes(label = paste0(round(mean,2), "%")), position = position_stack(vjust = 0.5),size=12) +
  
  
#  labs(y="Frequency", x="Times Treated", title="Histogram of PWID Treatment Log Frequency") +
  theme_minimal() +
#  guides(fill=guide_legend(title="Enrollment")) +
   theme(axis.title.x = element_blank(),
         axis.title.y = element_blank(),
         panel.border = element_blank(),
         axis.text.x=element_blank(),
         panel.grid=element_blank(),
         axis.ticks = element_blank(),
         text = element_text(size=22), 
#         legend.position = c(.9, .9), 
         legend.text=element_text(size=22)) 
show(p)
ggsave("Treatment Counts Pie chart.png", plot=p, width=10, height=8)

# Treatment count table
DAA_cost <- 25 # treatment cost in $1,000
treatmentSummary <- dt[, list(mean=mean(Freq), sd=sd(Freq)), 
                       by=list(Count,treatment_enrollment_per_PY)]

treatmentSummary <- treatmentSummary[Count %in% c(1,2,3,4,5,6,7)]

largestCount <- max(as.numeric(treatmentSummary$Count))
treatmentSummary$Count <- factor(treatmentSummary$Count, levels=seq(1,largestCount,1))

tableData <- treatmentSummary[treatment_enrollment_per_PY == 0.05]

tableData$mean <- round(tableData$mean)
tableData$num_episodes <- as.numeric(tableData$Count) * tableData$mean
#tableData$num_episodes <- round(tableData$num_episodes)
tableData$percent_treat <- 100 * tableData$num_episodes / sum(tableData$num_episodes)
tableData$percent_treat <- round(tableData$percent_treat,1)
tableData$DAA_cost <- tableData$num_episodes * DAA_cost
