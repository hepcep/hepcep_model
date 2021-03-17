#
# Analysis of hepcep model outputs - start treatment frequency tabulation for
#  determining re-treament counts
#
# Eric Tatara
#
library(data.table)
library(ggplot2)
library(scales)
library(Publish)

dt <- NULL
dt_infections <- NULL

# Load all of the stats files that exist in an experiments dir
eventsfileName <- "/events.csv"
statsfilename <- "/stats.csv"
dirs <- list.dirs (path=".", recursive=FALSE)

tableList <- list()
infection_tableList <- list()

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
      
      setkey(evTable, event_type) 
      start_treat_events <- evTable[.('STARTED_TREATMENT')]
    
      # Process events 
      if (nrow(start_treat_events) == 0){
        print(paste0("No start treatment event in log for: ", path))
        next
      }
      
      # Get the frequency of each person occurence
      personFreq <- as.data.frame(table(start_treat_events$person_id))
      
      # Get the frequency of start treatments
      start_treat_freq <- as.data.frame(table(personFreq$Freq))
      
      # Store in results table
      resultsTable <- as.data.table(start_treat_freq)
      setnames(resultsTable, old="Var1", new="Count")
      
      # Optionally store properties in the table for this run
      resultsTable$treatment_enrollment_per_PY <- props[Name=="treatment_enrollment_per_PY"]$Value
      
      resultsTable$treatment_nonadherence <- props[Name=="treatment_nonadherence"]$Value
      resultsTable$max_num_daa_treatments <- props[Name=="max_num_daa_treatments"]$Value
      
      resultsTable$run <- props[Name=="run.number"]$Value
      
      tableList[[d]]  <- resultsTable
      
      # Get the number of new chronic infections during the DAA treatment period
      new_chronic_events <- evTable[.('CHRONIC')]
      
      new_chronic_events <- new_chronic_events[tick > 4016]  # Hack for start treat year events
      
      # total infection count
      total_new_chronic <- nrow(new_chronic_events)
      
      # total number of re-infections (not counting original infection)
      re_infect_freq <- as.data.table(table(new_chronic_events$person_id))
      re_infect_freq[, reinfection := N - 1]
      total_reinfections <- sum(re_infect_freq$reinfection)
      
      infection_table <- data.table(total_infections = total_new_chronic,
                                    total_reinfections = total_reinfections)
      
      infection_table$treatment_enrollment_per_PY <- props[Name=="treatment_enrollment_per_PY"]$Value
      infection_table$treatment_nonadherence <- props[Name=="treatment_nonadherence"]$Value
      infection_table$max_num_daa_treatments <- props[Name=="max_num_daa_treatments"]$Value
      infection_table$run <- props[Name=="run.number"]$Value
      
      infection_tableList[[d]]  <- infection_table
      
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

dt <- rbindlist(tableList, fill=TRUE)  # Stack the list of tables into a single DT
tableList <- NULL           # clear mem

dt_infections <- rbindlist(infection_tableList, fill=TRUE)  # Stack the list of tables into a single DT
infection_tableList <- NULL           # clear mem

DAA_cost <- 25 # treatment cost in $1,000
dt$Count <- as.numeric(dt$Count)
dt[, cost := DAA_cost * Count * Freq]

# Calculate the mean and SD number of treatements per max_num_daa_treatments, 
#  enrollment rate, and non-adherence  
treatmentSummary <- dt[, ci.mean(Freq), 
                                  by=list(Count, treatment_enrollment_per_PY,
                                          treatment_nonadherence, max_num_daa_treatments)]

# DAA treatment costs mean & SE by number of retreatments
treatmentSummary_costs <- dt[, ci.mean(cost), 
                       by=list(Count, treatment_enrollment_per_PY,
                               treatment_nonadherence, max_num_daa_treatments)]

fwrite(treatmentSummary, file="treatment_counts_2.csv")
fwrite(treatmentSummary_costs, file="treatment_counts_costs_2.csv")


# Calculate the mean (95% CI) number of treatements per enrollment rate, and non-adherence
# First sum by run to get the total for each run
treatment_sums <- dt[, list(num_pwid=sum(Freq)), 
                           by=list(treatment_enrollment_per_PY,
                                   treatment_nonadherence, max_num_daa_treatments, run)]

treatment_sums_summary <- treatment_sums[, ci.mean(num_pwid), 
                       by=list(treatment_enrollment_per_PY,
                               treatment_nonadherence, max_num_daa_treatments)]

fwrite(treatment_sums_summary, file="treatment_sums_2.csv")

# Do similar for total cost per run for DAA treatment and then get the mean and CI
treatment_costs_total <- dt[, list(total_cost=sum(cost)), 
                          by=list(treatment_enrollment_per_PY,
                             treatment_nonadherence, max_num_daa_treatments, run)]

treatment_costs_summary <- treatment_costs_total[, ci.mean(total_cost), 
                                         by=list(treatment_enrollment_per_PY,
                                                 treatment_nonadherence, max_num_daa_treatments)]

fwrite(treatment_costs_summary, file="treatment_costs_sums_2.csv")


# Calculate the mean (95% CI) number of infections and re-infections per enrollment rate, and non-adherence
infections_summary <- dt_infections[, ci.mean(total_infections), 
                                                 by=list(treatment_enrollment_per_PY,
                                                         treatment_nonadherence, max_num_daa_treatments)]

fwrite(infections_summary, file="infections_summary_2.csv")

reinfections_summary <- dt_infections[, ci.mean(total_reinfections), 
                                    by=list(treatment_enrollment_per_PY,
                                            treatment_nonadherence, max_num_daa_treatments)]

fwrite(reinfections_summary, file="reinfections_summary_2.csv")



# OLD STUFF BELOW ##########################

#treatmentSummary <- treatmentSummary[Count %in% c(1,2,3,4,5,6,7)]

#largestCount <- max(as.numeric(treatmentSummary$Count))
#treatmentSummary$Count <- factor(treatmentSummary$Count, levels=seq(1,largestCount,1))

# # Treatment count histogram
# p <- ggplot(treatmentSummary, aes(x=Count, y=mean, fill=treatment_enrollment_per_PY)) +
#   geom_bar(position=position_dodge(), stat="identity", color="black", alpha=0.5) +
# #  geom_errorbar(aes(ymin=mean-sd, ymax=mean+sd), width=.2,position=position_dodge(.9)) +
# #  scale_y_continuous(limits=c(0, 70), breaks=seq(0,70,10)) +
# #  scale_x_discrete(limits=(c(1,2,3,4,5,6,7))) +
# 
#   scale_y_log10(limits=c(1,10000), 
#                 breaks = scales::trans_breaks("log10", function(x) 10^x),
#                 labels = scales::trans_format("log10", scales::math_format(10^.x))) +
#   annotation_logticks() +
#   labs(y="Frequency", x="Times Treated", title="Histogram of PWID Treatment Frequency") +
#   theme_minimal() +
#   guides(fill=guide_legend(title="Enrollment")) +
#   theme(text = element_text(size=24), 
#         legend.position = c(.8, .75), 
#         legend.text=element_text(size=22),
#         panel.border = element_rect(colour = "gray", fill=NA)) +
#   theme(axis.text=element_text(size=26),axis.title=element_text(size=26)) 
# show(p)
# ggsave("Treatment Counts Distribution.png", plot=p, width=10, height=8)
# 
# 
# # Treatment count pie chart
# pieData <- treatmentSummary[treatment_enrollment_per_PY == 0.05]
# 
# zeroCount <- 100 - sum(pieData$mean)
# pieData <- rbind(list(0,0.05,zeroCount,0), pieData)
# 
# p <- ggplot(pieData, aes(x="", y=mean, fill=Count)) +
#   geom_bar(stat="identity", width=1, alpha=0.5, colour="black") +
#   
#   coord_polar("y", start=0) +
#   
#   scale_fill_brewer(palette="Blues",direction = -1) +
#   
#   geom_text(aes(y=50 , x=0.75, label = percent(mean[1]/100)), size=12) +
#   geom_text(aes(y=10 , x=1.65, label = percent(mean[2]/100)), size=12) +
#   geom_text(aes(y= 3 , x=1.65, label = percent(mean[3]/100)), size=12) +
#   
# #  geom_text(aes(label = paste0(round(mean,2), "%")), position = position_stack(vjust = 0.5),size=12) +
#   
#   
# #  labs(y="Frequency", x="Times Treated", title="Histogram of PWID Treatment Log Frequency") +
#   theme_minimal() +
# #  guides(fill=guide_legend(title="Enrollment")) +
#    theme(axis.title.x = element_blank(),
#          axis.title.y = element_blank(),
#          panel.border = element_blank(),
#          axis.text.x=element_blank(),
#          panel.grid=element_blank(),
#          axis.ticks = element_blank(),
#          text = element_text(size=22), 
# #         legend.position = c(.9, .9), 
#          legend.text=element_text(size=22)) 
# show(p)
# ggsave("Treatment Counts Pie chart.png", plot=p, width=10, height=8)

# Treatment count table


tableData <- treatmentSummary[treatment_enrollment_per_PY == 0.075]

# 95% CI = mean(x) +/- z * std(x)
z <- 1.960
tableData$LCI_95 <- tableData$mean - z * tableData$std
tableData$UCI_95 <- tableData$mean + z * tableData$std

tableData$STD_95 <- z * tableData$std

#tableData$mean <- round(tableData$mean)
tableData$num_episodes <- as.numeric(tableData$Count) * tableData$mean
#tableData$num_episodes <- round(tableData$num_episodes)
tableData$DAA_cost <- tableData$num_episodes * DAA_cost

tableData$percent_treat <- 100 * tableData$num_episodes / sum(tableData$num_episodes)
tableData$percent_treat <- round(tableData$percent_treat,1)

