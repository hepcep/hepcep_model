#
# Analysis of hepcep model outputs
#
# Eric Tatara
#
library(data.table)
library(ggplot2)
library(zoo)

# Std Err
std <- function(x) sd(x)/sqrt(length(x))

dt <- NULL
table <- NULL

# Load all of the stats files that exist in an experiments dir
fileName <- "/stats.csv"
dirs <- list.dirs (path=".", recursive=FALSE)

colsToKeep <- c("tick","run","cured_ALL",
                "RNApreval_ALL","RNApreval_agegrp_LEQ_30", "RNApreval_agegrp_OVER_30",
                "infected_daily_agegrp_LEQ_30", "infected_daily_agegrp_OVER_30",
                "population_agegrp_LEQ_30", "population_agegrp_OVER_30", 
                "infected_agegrp_LEQ_30", "infected_agegrp_OVER_30",
                "incidence_daily","population_ALL","infected_ALL",
                "treatment_recruited_daily")

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
      
      # Filter out columns to reduce data in memory size
      table <-  fread(path, select=colsToKeep)
      #table <-  fread(path)
      
      # Optionally store properties in the table for this run
      table$treatment_enrollment_per_PY <- props[Name=="treatment_enrollment_per_PY"]$Value

      table$treatment_svr <- props[Name=="treatment_svr"]$Value
      
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

# rows should be the number of entries in a single run
rows <- max(dt$tick)
#rows <- 4380
burninDays <- 365

# Day samples that correspond to the END (day 365) of each simulation year
days <- seq((burninDays+365), rows, 365)

startYear <- 2010   # First year of simulation
endYear <- 2060    
#years <- seq(startYear, (startYear + length(days) - 1))    # list of all sim years in data

# Convert the simulation day tick to the simulated year
dayToYear <- function(firstYear, day) firstYear + floor((day)/365)
dt$Year <- unlist(lapply(dt$tick, dayToYear, firstYear=startYear-1))

# Convert the simulation day tick to month number.
# The month number will be greater than 12 for days > 365
dayToMonth <- function(day) 1 + floor((day)/31)
dt$Month <- unlist(lapply(dt$tick, dayToMonth))

# Convert the simulation day tick to week number.
# The week number will be greater than 50 for days > 365
dayToWeek <- function(day) 1 + floor((day)/7)
dt$Week <- unlist(lapply(dt$tick, dayToWeek))

years <- unlist(unique(dt$Year))

# Create a subset of columns to select from the DT
categories <- c("prevalence_ALL", "prevalence_gender_MALE", "prevalence_gender_FEMALE", 
                "prevalence_race_NHWhite", "prevalence_race_NHBlack", "prevalence_race_Hispanic",
                "prevalence_syringesource_HR", "prevalence_syringesource_nonHR",
                "prevalence_agegrp_LEQ_30", "prevalence_agegrp_OVER_30", "prevalence_areatype_CITY",
                "prevalence_areatype_SUBURBAN")

catLabels <- c("ALL", "MALE", "FEMALE", "NHWhite", "NHBlack", "Hispanic", "HR", "nonHR" ,"LEQ30", 
               "Over30", "City", "Suburban")

# The annual data is a snapshot of stats on the last day in the year
annualData <- dt[tick %in% days]
#annualData <- annualData[,categories,with=FALSE]
#annualData[, Year := years]


# Optionally filter the annualData by experiment type, e.g. some swept parameter
#annualData <- annualData[treatment_enrollment_per_PY == 0.00]

# Aggregate statistics on the data by year
# Mean
annDataSummary<-aggregate(annualData, by=list(Year = annualData$Year), FUN=mean)
names(annDataSummary) <- paste0(names(annDataSummary),"_mean")

# Std Err
std <- function(x) sd(x)/sqrt(length(x))
annDataStd<-aggregate(annualData, by=list(Year = annualData$Year), FUN=std)
names(annDataStd) <- paste0(names(annDataStd),"_std")

# Std Dev
annDataSd<-aggregate(annualData, by=list(Year = annualData$Year), FUN=sd)
names(annDataSd) <- paste0(names(annDataSd),"_sd")

annDataSummary <-cbind(annDataSummary, annDataStd, annDataSd)

# Year for which we will generate a prediction data set
predictionYear <- 2012
predictionData <- annualData[Year==predictionYear,]

plotData <- predictionData[,categories,with=FALSE]
#Single run DF 
#predictionSet <- data.frame(cat=catLabels,vals=as.numeric(plotData[1]))

#Multi run DF calculate the mean and std dev for each category
predictionSet <- data.frame(cat=catLabels,vals=100*apply(plotData,2,mean), sd=100*apply(plotData,2,sd))

# Force the frame levels to specific order
predictionSet$cat <- factor(predictionSet$cat, levels=catLabels)
predictionSet$model <- "HepCEP"
row.names(predictionSet) <- NULL  # just use numeric rows

# Values from the PLOS paper for comparison
# Values from NHBS_2009_2012_validation_revised-2015-01-31__deIDed.xlsx
df <- data.frame(cat=catLabels, vals= c(42,42,41,37,55,42,38,46,23,51,50,35), 
                 sd=c(2.7,2.7,2.8,2.7,2.6,2.5,2.4,4.1,2.4,2.1,2.5,2.9), model="APK")

predictionSet <- rbind(predictionSet,df)

df <- data.frame(cat=catLabels, vals= c(42,43,40,36,57,43,42,41,24,54,53,32), 
                 sd=c(3,4,6,6,6,6,7,4,8,3,4,10), model="NHBS")
predictionSet <- rbind(predictionSet,df)

# Prevalence for all groups in a single year (PLOS Fog 4)
p <- ggplot(predictionSet, aes(x=cat, y=vals, fill=model)) +
  geom_bar(position=position_dodge(), stat="identity", color="black", alpha=0.5) +
  geom_errorbar(aes(ymin=vals-sd, ymax=vals+sd), width=.2,position=position_dodge(.9)) +
  scale_y_continuous(limits=c(0, 70), breaks=seq(0,70,10)) +
  scale_fill_manual(name="", values = c("APK"="green", "HepCEP"="blue", "NHBS"="red")) +
  labs(y="Prevalence (HCV AB+) %", x="") +
  theme_minimal() +
  theme(text = element_text(size=30), legend.position = c(.2, .9), legend.text=element_text(size=22)) 
ggsave("Prevalence Histogram.png", plot=p, width=10, height=7)

# Prevalnce plot by racial groups (PLOS Fog 6)
p <- ggplot(annDataSummary, aes(x=Year_mean)) + 
  geom_line(aes(y=prevalence_race_NHWhite_mean, color='NHWhite'), size=1.5) +
  geom_point(aes(y=prevalence_race_NHWhite_mean, color='NHWhite', shape='NHWhite'), size=4) +
  geom_line(aes(y=prevalence_race_NHBlack_mean, color='NHBlack'), size=1.5) +
  geom_point(aes(y=prevalence_race_NHBlack_mean, color='NHBlack', shape='NHBlack'), size=4) +
  geom_line(aes(y=prevalence_race_Hispanic_mean, color='Hispanic'), size=1.5) +
  geom_point(aes(y=prevalence_race_Hispanic_mean, color='Hispanic', shape='Hispanic'), size=4) +
  geom_line(aes(y=prevalence_ALL_mean, color='ALL'), size=1.5) +
  geom_point(aes(y=prevalence_ALL_mean, color='ALL', shape='ALL'), size=4) +
  geom_line(aes(y=prevalence_syringesource_HR_mean, color='HR'), size=1.5) +
  geom_point(aes(y=prevalence_syringesource_HR_mean, color='HR', shape='HR'), size=4) +
  geom_line(aes(y=prevalence_syringesource_nonHR_mean, color='nonHR'), size=1.5) +
  geom_point(aes(y=prevalence_syringesource_nonHR_mean, color='nonHR', shape='nonHR'), size=4) +
  
  geom_errorbar(aes(ymin=prevalence_race_NHWhite_mean-prevalence_race_NHWhite_sd, 
                    ymax=prevalence_race_NHWhite_mean+prevalence_race_NHWhite_sd,color='NHWhite'),width=.15) + 
  geom_errorbar(aes(ymin=prevalence_race_NHBlack_mean-prevalence_race_NHBlack_sd, 
                    ymax=prevalence_race_NHBlack_mean+prevalence_race_NHBlack_sd,color='NHBlack'),width=.15) +
  geom_errorbar(aes(ymin=prevalence_race_Hispanic_mean-prevalence_race_Hispanic_sd, 
                    ymax=prevalence_race_Hispanic_mean+prevalence_race_Hispanic_sd,color='Hispanic'),width=.15) + 
  geom_errorbar(aes(ymin=prevalence_ALL_mean-prevalence_ALL_sd, 
                    ymax=prevalence_ALL_mean+prevalence_ALL_sd,color='ALL'),width=.15) +
  geom_errorbar(aes(ymin=prevalence_syringesource_HR_mean-prevalence_syringesource_HR_sd, 
                    ymax=prevalence_syringesource_HR_mean+prevalence_syringesource_HR_sd,color='HR'),width=.15) + 
  geom_errorbar(aes(ymin=prevalence_syringesource_nonHR_mean-prevalence_syringesource_nonHR_sd, 
                    ymax=prevalence_syringesource_nonHR_mean+prevalence_syringesource_nonHR_sd,color='nonHR'),width=.15) + 
  
#  scale_x_continuous(breaks=years) +
  scale_x_continuous(breaks=c(2010,2012,2014,2016,2018,2020)) +
  scale_y_continuous(limits=c(0, 0.6), breaks=seq(0,0.6,0.1)) +
  scale_color_manual(name="", values = c("NHWhite"="green", "NHBlack"="blue", "Hispanic"="red","ALL"="black", "HR"="black", "nonHR"="red")) +
  scale_shape_manual(name="", values = c("NHWhite"=23, "NHBlack"=3, "Hispanic"=25,"ALL"=17, "HR"=16, "nonHR"=20)) +
  labs(y="Prevalence (HCV AB+)", x="Year") +
  theme_minimal() +
  theme(text = element_text(size=30), legend.position = c(.25, .25), legend.text=element_text(size=24)) 
#  guides(fill=guide_legend(keywidth=0.1,keyheight=1.9,default.unit="inch"))
ggsave("HCV Prev Race Syringe.png", plot=p, width=8, height=6)


# Prevalnce plot by age and city/suburbs (PLOS Fog 6)
p <- ggplot(annDataSummary, aes(x=Year_mean)) + 
  geom_line(aes(y=prevalence_areatype_CITY_mean, color='City of Chicago'), size=1.5) +
  geom_point(aes(y=prevalence_areatype_CITY_mean, color='City of Chicago', shape='City of Chicago'), size=4) +
  geom_line(aes(y=prevalence_areatype_SUBURBAN_mean, color='Suburban'), size=1.5) +
  geom_point(aes(y=prevalence_areatype_SUBURBAN_mean, color='Suburban', shape='Suburban'), size=4) +
  geom_line(aes(y=prevalence_ALL_mean, color='ALL'), size=1.5) +
  geom_point(aes(y=prevalence_ALL_mean, color='ALL', shape='ALL'), size=4) +
  geom_line(aes(y=prevalence_agegrp_LEQ_30_mean, color='Under 30'), size=1.5) +
  geom_point(aes(y=prevalence_agegrp_LEQ_30_mean, color='Under 30', shape='Under 30'), size=4) +
  geom_line(aes(y=prevalence_agegrp_OVER_30_mean, color='Over 30'), size=1.5) +
  geom_point(aes(y=prevalence_agegrp_OVER_30_mean, color='Over 30', shape='Over 30'), size=4) +
  
  geom_errorbar(aes(ymin=prevalence_areatype_CITY_mean-prevalence_areatype_CITY_sd, 
                    ymax=prevalence_areatype_CITY_mean+prevalence_areatype_CITY_sd,color='City of Chicago'),width=.15) +
  geom_errorbar(aes(ymin=prevalence_areatype_SUBURBAN_mean-prevalence_areatype_SUBURBAN_sd, 
                    ymax=prevalence_areatype_SUBURBAN_mean+prevalence_areatype_SUBURBAN_sd,color='Suburban'),width=.15) +
  geom_errorbar(aes(ymin=prevalence_ALL_mean-prevalence_ALL_sd, 
                    ymax=prevalence_ALL_mean+prevalence_ALL_sd,color='ALL'),width=.15) +
  geom_errorbar(aes(ymin=prevalence_agegrp_LEQ_30_mean-prevalence_agegrp_LEQ_30_sd, 
                    ymax=prevalence_agegrp_LEQ_30_mean+prevalence_agegrp_LEQ_30_sd,color='Under 30'),width=.15) +
  geom_errorbar(aes(ymin=prevalence_agegrp_OVER_30_mean-prevalence_agegrp_OVER_30_sd, 
                    ymax=prevalence_agegrp_OVER_30_mean+prevalence_agegrp_OVER_30_sd,color='Over 30'),width=.15) +
  
#  scale_x_continuous(breaks=years) +
  scale_x_continuous(breaks=c(2010,2012,2014,2016,2018,2020)) +
  scale_y_continuous(limits=c(0, 0.6), breaks=seq(0,0.6,0.1)) +
  scale_color_manual(name="", values = c("City of Chicago"="green", "Suburban"="green", "ALL"="black", "Under 30"="blue", "Over 30"="orange")) +
  scale_shape_manual(name="", values = c("City of Chicago"=18, "Suburban"=4, "ALL"=17, "Under 30"=15, "Over 30"=16)) +
  labs(y="Prevalence (HCV AB+)", x="Year") +
  theme_minimal() +
  theme(text = element_text(size=30), legend.position = c(.25, .25), legend.text=element_text(size=24))
ggsave("HCV Prev Age Location.png", plot=p, width=8, height=6)

# Prevalnce plot by gender (PLOS Fog 6)
p <- ggplot(annDataSummary, aes(x=Year_mean)) + 
  geom_line(aes(y=prevalence_ALL_mean, color='ALL'), size=1.5) +
  geom_point(aes(y=prevalence_ALL_mean, color='ALL', shape='ALL'), size=4) +
  geom_line(aes(y=prevalence_gender_MALE_mean, color='Male'), size=1.5) +
  geom_point(aes(y=prevalence_gender_MALE_mean, color='Male', shape='Male'), size=4) +
  geom_line(aes(y=prevalence_gender_FEMALE_mean, color='Female'), size=1.5) +
  geom_point(aes(y=prevalence_gender_FEMALE_mean, color='Female', shape='Female'), size=4) +
  
  geom_errorbar(aes(ymin=prevalence_ALL_mean-prevalence_ALL_sd, 
                    ymax=prevalence_ALL_mean+prevalence_ALL_sd,color='ALL'),width=.15) +
  geom_errorbar(aes(ymin=prevalence_gender_MALE_mean-prevalence_gender_MALE_sd, 
                    ymax=prevalence_gender_MALE_mean+prevalence_gender_MALE_sd,color='Male'),width=.15) +
  geom_errorbar(aes(ymin=prevalence_gender_FEMALE_mean-prevalence_gender_FEMALE_sd, 
                    ymax=prevalence_gender_FEMALE_mean+prevalence_gender_FEMALE_sd,color='Female'),width=.15) +
  
#  scale_x_continuous(breaks=years) +
  scale_x_continuous(breaks=c(2010,2012,2014,2016,2018,2020)) +
  scale_y_continuous(limits=c(0, 0.6), breaks=seq(0,0.6,0.1)) +
  scale_color_manual(name="", values = c("ALL"="black", "Male"="dark green", "Female"="purple")) +
  scale_shape_manual(name="", values = c("ALL"=17, "Male"=18, "Female"=15)) +
  labs(y="Prevalence (HCV AB+)", x="Year") +
  theme_minimal() +
  theme(text = element_text(size=30), legend.position = c(.25, .25), legend.text=element_text(size=24))
ggsave("HCV Prev Gender.png", plot=p, width=8, height=6)

# Fraction of population by age and city/suburbs (PLOS Fog 7)
p <- ggplot(annDataSummary, aes(x=Year_mean)) + 
  geom_line(aes(y=fraction_areatype_CITY_mean, color='City of Chicago'), size=1.5) +
  geom_point(aes(y=fraction_areatype_CITY_mean, color='City of Chicago', shape='City of Chicago'), size=4) +
  geom_line(aes(y=fraction_areatype_SUBURBAN_mean, color='Suburban'), size=1.5) +
  geom_point(aes(y=fraction_areatype_SUBURBAN_mean, color='Suburban', shape='Suburban'), size=4) +
  geom_line(aes(y=fraction_agegrp_LEQ_30_mean, color='Under 30'), size=1.5) +
  geom_point(aes(y=fraction_agegrp_LEQ_30_mean, color='Under 30', shape='Under 30'), size=4) +
  geom_line(aes(y=fraction_agegrp_OVER_30_mean, color='Over 30'), size=1.5) +
  geom_point(aes(y=fraction_agegrp_OVER_30_mean, color='Over 30', shape='Over 30'), size=4) +
  
  geom_errorbar(aes(ymin=fraction_areatype_CITY_mean-fraction_areatype_CITY_sd, 
                    ymax=fraction_areatype_CITY_mean+fraction_areatype_CITY_sd,color='City of Chicago'),width=.15) +
  geom_errorbar(aes(ymin=fraction_areatype_SUBURBAN_mean-fraction_areatype_SUBURBAN_sd, 
                    ymax=fraction_areatype_SUBURBAN_mean+fraction_areatype_SUBURBAN_sd,color='Suburban'),width=.15) +
  geom_errorbar(aes(ymin=fraction_agegrp_LEQ_30_mean-fraction_agegrp_LEQ_30_sd, 
                    ymax=fraction_agegrp_LEQ_30_mean+fraction_agegrp_LEQ_30_sd,color='Under 30'),width=.15) +
  geom_errorbar(aes(ymin=fraction_agegrp_OVER_30_mean-fraction_agegrp_OVER_30_sd, 
                    ymax=fraction_agegrp_OVER_30_mean+fraction_agegrp_OVER_30_sd,color='Over 30'),width=.15) +
  
#  scale_x_continuous(breaks=years) +
  scale_x_continuous(breaks=seq(2010,2045,5)) +
  scale_y_continuous(limits=c(0, 1.0), breaks=seq(0,1.0,0.2)) +
  scale_color_manual(name="", values = c("City of Chicago"="green", "Suburban"="green", "Under 30"="blue", "Over 30"="orange")) +
  scale_shape_manual(name="", values = c("City of Chicago"=18, "Suburban"=4, "Under 30"=15, "Over 30"=16)) +
  labs(y="Fraction of Population", x="Year") +
  theme_minimal() +
  theme(text = element_text(size=30), legend.position = c(.25, .90), legend.text=element_text(size=24))
ggsave("Fraction Age Location.png", plot=p, width=8, height=6)

# Fraction of population by age subgroups (PLOS Fog 7)
p <- ggplot(annDataSummary, aes(x=Year_mean)) + 
  geom_line(aes(y=fraction_agedec_AGE_LEQ_20_mean, color='<=20'), size=1.5) +
  geom_point(aes(y=fraction_agedec_AGE_LEQ_20_mean, color='<=20', shape='<=20'), size=4) +
  geom_line(aes(y=fraction_agedec_AGE_21_30_mean, color='21-30'), size=1.5) +
  geom_point(aes(y=fraction_agedec_AGE_21_30_mean, color='21-30', shape='21-30'), size=4) +
  geom_line(aes(y=fraction_agedec_AGE_31_40_mean, color='31-40'), size=1.5) +
  geom_point(aes(y=fraction_agedec_AGE_31_40_mean, color='31-40', shape='31-40'), size=4) +
  geom_line(aes(y=fraction_agedec_AGE_41_50_mean, color='41-50'), size=1.5) +
  geom_point(aes(y=fraction_agedec_AGE_41_50_mean, color='41-50', shape='41-50'), size=4) +
  geom_line(aes(y=fraction_agedec_AGE_51_60_mean, color='51-60'), size=1.5) +
  geom_point(aes(y=fraction_agedec_AGE_51_60_mean, color='51-60', shape='51-60'), size=4) +
  
  geom_errorbar(aes(ymin=fraction_agedec_AGE_LEQ_20_mean-fraction_agedec_AGE_LEQ_20_sd, 
                    ymax=fraction_agedec_AGE_LEQ_20_mean+fraction_agedec_AGE_LEQ_20_sd,color='<=20'),width=.15) +
  geom_errorbar(aes(ymin=fraction_agedec_AGE_21_30_mean-fraction_agedec_AGE_21_30_sd, 
                    ymax=fraction_agedec_AGE_21_30_mean+fraction_agedec_AGE_21_30_sd,color='21-30'),width=.15) +
  geom_errorbar(aes(ymin=fraction_agedec_AGE_31_40_mean-fraction_agedec_AGE_31_40_sd, 
                    ymax=fraction_agedec_AGE_31_40_mean+fraction_agedec_AGE_31_40_sd,color='31-40'),width=.15) +
  geom_errorbar(aes(ymin=fraction_agedec_AGE_41_50_mean-fraction_agedec_AGE_41_50_sd, 
                    ymax=fraction_agedec_AGE_41_50_mean+fraction_agedec_AGE_41_50_sd,color='41-50'),width=.15) +
  geom_errorbar(aes(ymin=fraction_agedec_AGE_51_60_mean-fraction_agedec_AGE_51_60_sd, 
                    ymax=fraction_agedec_AGE_51_60_mean+fraction_agedec_AGE_51_60_sd,color='51-60'),width=.15) +
  
#  scale_x_continuous(breaks=years) +
  scale_x_continuous(breaks=seq(2010,2045,5)) +
  scale_y_continuous(limits=c(0, 1.0), breaks=seq(0,1.0,0.2)) +
  scale_color_manual(name="", values = c("<=20"="red", "21-30"="blue", "31-40"="green", "41-50"="black", "51-60"="purple")) +
  scale_shape_manual(name="", values = c("<=20"=20, "21-30"=15, "31-40"=16, "41-50"=18, "51-60"=17)) +
  labs(y="Fraction of Population", x="Year") +
  theme_minimal() +
  theme(text = element_text(size=30), legend.position = c(.15, .90), legend.text=element_text(size=24))
ggsave("Fraction Age.png", plot=p, width=8, height=6)

# Fraction of population by racial groups (PLOS Fog 7)
p <- ggplot(annDataSummary, aes(x=Year_mean)) + 
  geom_line(aes(y=fraction_race_NHWhite_mean, color='NHWhite'), size=1.5) +
  geom_point(aes(y=fraction_race_NHWhite_mean, color='NHWhite', shape='NHWhite'), size=4) +
  geom_line(aes(y=fraction_race_NHBlack_mean, color='NHBlack'), size=1.5) +
  geom_point(aes(y=fraction_race_NHBlack_mean, color='NHBlack', shape='NHBlack'), size=4) +
  geom_line(aes(y=fraction_race_Hispanic_mean, color='Hispanic'), size=1.5) +
  geom_point(aes(y=fraction_race_Hispanic_mean, color='Hispanic', shape='Hispanic'), size=4) +
  geom_line(aes(y=fraction_syringesource_HR_mean, color='HR'), size=1.5) +
  geom_point(aes(y=fraction_syringesource_HR_mean, color='HR', shape='HR'), size=4) +
  geom_line(aes(y=fraction_syringesource_nonHR_mean, color='nonHR'), size=1.5) +
  geom_point(aes(y=fraction_syringesource_nonHR_mean, color='nonHR', shape='nonHR'), size=4) +
  
  geom_errorbar(aes(ymin=fraction_race_NHWhite_mean-fraction_race_NHWhite_sd, 
                    ymax=fraction_race_NHWhite_mean+fraction_race_NHWhite_sd,color='NHWhite'),width=.15) +
  geom_errorbar(aes(ymin=fraction_race_NHBlack_mean-fraction_race_NHBlack_sd, 
                    ymax=fraction_race_NHBlack_mean+fraction_race_NHBlack_sd,color='NHBlack'),width=.15) +
  geom_errorbar(aes(ymin=fraction_race_Hispanic_mean-fraction_race_Hispanic_sd, 
                    ymax=fraction_race_Hispanic_mean+fraction_race_Hispanic_sd,color='Hispanic'),width=.15) +
  geom_errorbar(aes(ymin=fraction_syringesource_HR_mean-fraction_syringesource_HR_sd, 
                    ymax=fraction_syringesource_HR_mean+fraction_syringesource_HR_sd,color='HR'),width=.15) +
  geom_errorbar(aes(ymin=fraction_syringesource_nonHR_mean-fraction_syringesource_nonHR_sd, 
                    ymax=fraction_syringesource_nonHR_mean+fraction_syringesource_nonHR_sd,color='nonHR'),width=.15) +
  
#  scale_x_continuous(breaks=years) +
  scale_x_continuous(breaks=c(2010,2012,2014,2016,2018,2020)) +
  scale_y_continuous(limits=c(0, 1.0), breaks=seq(0,1.0,0.2)) +
  scale_color_manual(name="", values = c("NHWhite"="orange", "NHBlack"="blue", "Hispanic"="red","ALL"="black", "HR"="black", "nonHR"="red")) +
  scale_shape_manual(name="", values = c("NHWhite"=23, "NHBlack"=3, "Hispanic"=25,"ALL"=17, "HR"=16, "nonHR"=20)) +
  labs(y="Fraction of Population", x="Year") +
  theme_minimal() +
  theme(text = element_text(size=30), legend.position = c(.15, .90), legend.text=element_text(size=24))
ggsave("Fraction Race.png", plot=p, width=8, height=6)


# TODO Update the prevalence stats calculations to use the data.table method as with incidence.

# Aggregate on paramter sweep and year
# mean
annDataSummarySweep<-aggregate(annualData, 
                               by=list(Year = annualData$Year, 
                                       Enrollment=annualData$treatment_enrollment_per_PY), 
                               FUN=mean)
names(annDataSummarySweep) <- paste0(names(annDataSummarySweep),"_mean")

# Std Err
annDataSummarySweepStd<-aggregate(annualData, 
                                  by=list(Year = annualData$Year, 
                                          Enrollment=annualData$treatment_enrollment_per_PY), 
                                  FUN=std)

names(annDataSummarySweepStd) <- paste0(names(annDataSummarySweepStd),"_std")

# Std Dev
annDataSummarySweepSd<-aggregate(annualData, 
                                 by=list(Year = annualData$Year, 
                                         Enrollment=annualData$treatment_enrollment_per_PY), 
                                 FUN=sd)
names(annDataSummarySweepSd) <- paste0(names(annDataSummarySweepSd),"_sd")

annDataSummarySweep <-cbind(annDataSummarySweep, annDataSummarySweepStd, annDataSummarySweepSd)

annDataSummarySweep <- as.data.table(annDataSummarySweep)
setA <- annDataSummarySweep[Enrollment_mean %in% c(0,0.01)]
setA <- setA[Year_mean > 2014]

p <- ggplot(annDataSummarySweep) + geom_line(aes(x=Year_mean,y=RNApreval_ALL_mean,color=Enrollment_mean), size=1) +
  geom_point(aes(x=Year_mean,y=RNApreval_ALL_mean,color=Enrollment_mean), size=2) +
#  scale_x_continuous(breaks=c(2010,2012,2014,2016,2018,2020)) +
  
  geom_errorbar(aes(x=Year_mean, ymin=RNApreval_ALL_mean-RNApreval_ALL_sd, 
                    ymax=RNApreval_ALL_mean+RNApreval_ALL_sd,color=Enrollment_mean),width=.15) +
  
  scale_x_continuous(limits=c(2010, 2030)) +
  scale_y_continuous(limits=c(0, 0.4), breaks=seq(0,1.0,0.05)) +
  labs(y="HCV RNA Prevalence", x="Year", color="Enrollment") +
  theme_minimal() +
  theme(text = element_text(size=20), legend.position = c(.85, .80), legend.text=element_text(size=20))
ggsave("Treatment Prevalence ALL.png", plot=p, width=10, height=8)

p <- ggplot(annDataSummarySweep) + geom_line(aes(x=Year_mean,y=RNApreval_agegrp_LEQ_30_mean,color=Enrollment_mean), size=1) +
  geom_point(aes(x=Year_mean,y=RNApreval_agegrp_LEQ_30_mean,color=Enrollment_mean), size=2) +
  #  scale_x_continuous(breaks=c(2010,2012,2014,2016,2018,2020)) +
  
  geom_errorbar(aes(x=Year_mean, ymin=RNApreval_agegrp_LEQ_30_mean-RNApreval_agegrp_LEQ_30_sd, 
                    ymax=RNApreval_agegrp_LEQ_30_mean+RNApreval_agegrp_LEQ_30_sd,color=Enrollment_mean),width=.15) +
  
  scale_x_continuous(limits=c(2014, 2045)) +
  scale_y_continuous(limits=c(0, 0.4), breaks=seq(0,1.0,0.05)) +
  labs(y="HCV RNA Prevalence Young PWID (<30)", x="Year", color="Enrollment") +
  theme_minimal() +
  theme(text = element_text(size=20), legend.position = c(.85, .80), legend.text=element_text(size=20))
ggsave("Treatment Prevalence Young.png", plot=p, width=10, height=8)

p <- ggplot(annDataSummarySweep) + geom_line(aes(x=Year_mean,y=RNApreval_agegrp_OVER_30_mean,color=Enrollment_mean), size=1) +
  geom_point(aes(x=Year_mean,y=RNApreval_agegrp_OVER_30_mean,color=Enrollment_mean), size=2) +
  #  scale_x_continuous(breaks=c(2010,2012,2014,2016,2018,2020)) +
  
  geom_errorbar(aes(x=Year_mean, ymin=RNApreval_agegrp_OVER_30_mean-RNApreval_agegrp_OVER_30_sd, 
                    ymax=RNApreval_agegrp_OVER_30_mean+RNApreval_agegrp_OVER_30_sd,color=Enrollment_mean),width=.15) +
  
  scale_x_continuous(limits=c(2014, 2045)) +
  scale_y_continuous(limits=c(0, 0.4), breaks=seq(0,1.0,0.05)) +
  labs(y="HCV RNA Prevalence Older PWID (30+)", x="Year", color="Enrollment") +
  theme_minimal() +
  theme(text = element_text(size=20), legend.position = c(.85, .80), legend.text=element_text(size=20))
ggsave("Treatment Prevalence Old.png", plot=p, width=10, height=8)


# TODO change DT to exclude tick 0 and tick 1:burninDays in all data sets
#!(tick %in% 1:burninDays)

# Calculate the yearly incidence rate per 1000 person-years which is the yearly sum of 
#   the dt$incidence_daily by the population count
incidenceYear <- dt[Year %in% startYear:endYear, .(incidence=1000*sum(incidence_daily/(population_ALL-infected_ALL))), by=list(Year,treatment_enrollment_per_PY, run)]
#incidenceYear <- dt[Year %in% startYear:endYear, .(incidence=1000*sum(incidence_daily/(population_ALL-infected_ALL))), by=list(Year,treatment_enrollment_per_PY, treatment_svr, run)]

#incidenceYear <- dt[Year %in% startYear:endYear, .(incidence=1000*sum(infected_daily_agegrp_LEQ_30/(population_agegrp_LEQ_30-infected_agegrp_LEQ_30))), by=list(Year,treatment_enrollment_per_PY,run)]
#incidenceYear <- dt[Year %in% startYear:endYear, .(incidence=1000*sum(infected_daily_agegrp_OVER_30/(population_agegrp_OVER_30-infected_agegrp_OVER_30))), by=list(Year,treatment_enrollment_per_PY,run)]

# Calculate the mean and std of yearly incidence rate
incidenceSummary <- incidenceYear[, list(mean=mean(incidence), sd=sd(incidence), std=std(incidence)), by=list(Year,treatment_enrollment_per_PY)]
#incidenceSummary <- incidenceYear[, list(mean=mean(incidence), sd=sd(incidence), std=std(incidence)), by=list(Year,treatment_enrollment_per_PY, treatment_svr)]

incidenceSummaryBaseline <- incidenceSummary[treatment_enrollment_per_PY == 0]
incidenceSummarySubset <- incidenceSummary[treatment_enrollment_per_PY %in% c(0, 0.025,0.05,0.075, 0.1)]

# The baseline normalization is the no-treatment mean in 2019
baseline <- incidenceSummaryBaseline[Year==2019]$mean

# optionally normalize the means relative to the untreated group
#  ... we also normalize the sd by the baseline mean
incidenceSummarySubset$mean <- incidenceSummarySubset$mean / baseline # incidenceSummaryBaseline$mean
incidenceSummarySubset$sd <- incidenceSummarySubset$sd / baseline # / incidenceSummaryBaseline$sd
incidenceSummarySubset$std <- incidenceSummarySubset$std / baseline # / incidenceSummaryBaseline$std

# 95% CI
z <- 1.960

p <- ggplot(incidenceSummarySubset) + geom_line(aes(x=Year+1, y=mean, color=treatment_enrollment_per_PY), size=1) +
  geom_point(aes(x=Year+1, y=mean, color=treatment_enrollment_per_PY), size=2) +
  scale_x_continuous(limits = c(2019.9,2060)) + #, breaks=c(2020, 2022, 2024, 2026, 2028, 2030)) +
#  scale_y_continuous(limits = c(0,1.8)) +
 
#  facet_wrap(vars(treatment_svr)) +
   
#  geom_errorbar(aes(x=Year+1, ymin=mean-z*std, ymax=mean+z*std, color=treatment_enrollment_per_PY),width=.15) +
  
  geom_ribbon(aes(x=Year+1, ymin=mean-z*std, ymax=mean+z*std, fill=treatment_enrollment_per_PY),alpha=0.3,colour=NA) +
  
#  scale_y_continuous(limits=c(0, 0.4), breaks=seq(0,1.0,0.1)) +
  labs(y="Relative Incidence (per 1000 person-years)", x="Year", color="treatment_enrollment_per_PY") + #, title="All Incidence") +
  theme_bw() +
#  theme_minimal() + 
  theme(text = element_text(size=14), 
        legend.position = c(.05, .15), 
        legend.text=element_text(size=14),
        legend.background = element_rect(fill="white", size=0.5, linetype="solid", colour ="gray")) +
  theme(axis.text=element_text(size=14),axis.title=element_text(size=14)) +
  
  guides(color=guide_legend(title="Enrollment"),fill=guide_legend(title="Enrollment"))
ggsave("Treatment Incidence yes-retreat.pdf", plot=p, width=10, height=8)
fwrite(incidenceSummarySubset, file="incidenceSummary.csv")

# Create a new plot with checkpoint data comparing to non-checkpoint data
q <- p + geom_line(data=incidenceSummarySubset_cp,aes(x=Year+1, y=mean, color=treatment_enrollment_per_PY), size=1, linetype = "dashed")

# Testing Box plots
ggplot(incidenceYear[treatment_enrollment_per_PY==0.05], aes(x=Year, y=incidence/12, group=Year)) + 
  geom_boxplot(outlier.colour="red") +
#  geom_dotplot(binaxis='y', stackdir='center', dotsize=0.3)

  scale_x_continuous(limits = c(2019.5,2030), breaks=c(2020, 2022, 2024, 2026, 2028, 2030)) 



# Calculated the annual cured counts
# The annual data is a snapshot of stats on the last day in the year
annualData <- dt[Year %in% startYear:endYear & tick %in% days]

# Calculate the mean and std of yearly cured counts
curedYearSummary <- annualData[, list(mean=mean(cured_ALL), sd=sd(cured_ALL)), by=list(Year,treatment_enrollment_per_PY)]
#curedSummarySubset <- curedYearSummary[treatment_enrollment_per_PY %in% c(0,0.01,0.05)]

p <- ggplot(curedYearSummary) + geom_line(aes(x=Year, y=mean, color=treatment_enrollment_per_PY), size=1) +
  geom_point(aes(x=Year, y=mean, color=treatment_enrollment_per_PY), size=2) +
#  scale_x_continuous(breaks=seq(2010,2045,5)) +
  
  geom_errorbar(aes(x=Year, ymin=mean-sd, ymax=mean+sd, color=treatment_enrollment_per_PY),width=.15) +
  
  #  scale_y_continuous(limits=c(0, 0.4), breaks=seq(0,1.0,0.1)) +
  labs(y="Total Cured", x="Year", color="treatment_enrollment_per_PY", title="") +
  theme_minimal() +
  theme(text = element_text(size=20), legend.position = c(.85, .80), legend.text=element_text(size=20)) +
  guides(color=guide_legend(title="Enrollment"))
ggsave("Treatment Counts.png", plot=p, width=10, height=8)


# Calculate the annual in treatment sum
treatedYear <- dt[Year %in% startYear:endYear, .(treated=sum(treatment_recruited_daily)), by=list(Year,treatment_enrollment_per_PY,run)]

# Calculate the mean and sd of treatment sum
treatedYearSUmmary <- treatedYear[, list(mean=mean(treated), sd=sd(treated)), by=list(Year,treatment_enrollment_per_PY)]

p <- ggplot(treatedYearSUmmary) + geom_line(aes(x=Year, y=mean, color=treatment_enrollment_per_PY), size=1) +
  geom_point(aes(x=Year, y=mean, color=treatment_enrollment_per_PY), size=2) +
#  scale_x_continuous(breaks=seq(2010,2045,5)) +
  
  geom_errorbar(aes(x=Year, ymin=mean-sd, ymax=mean+sd, color=treatment_enrollment_per_PY),width=.15) +
  
  #  scale_y_continuous(limits=c(0, 0.4), breaks=seq(0,1.0,0.1)) +
  labs(y="Total In Treatment", x="Year", color="treatment_enrollment_per_PY", title="") +
  theme_minimal() +
  theme(text = element_text(size=20), legend.position = c(.85, .80), legend.text=element_text(size=20)) +
  guides(color=guide_legend(title="Enrollment"))
ggsave("Treatment Counts.png", plot=p, width=10, height=8)



#### Testing weekly and daily incidence



# Calculate the Weekly incidence rate per 1000 person-years which is the yearly sum of 
#   the dt$incidence_daily by the population count 
incidenceWeek <- dt[Year %in% startYear:endYear, .(incidence=1000*sum(incidence_daily/(population_ALL-infected_ALL))), 
                    by=list(Week,treatment_enrollment_per_PY,run)]

# Calculate the mean and std of Weekly incidence rate
incidenceSummaryWeekly <- incidenceWeek[, list(mean=mean(incidence), sd=sd(incidence)), 
                                        by=list(Week,treatment_enrollment_per_PY)]

p <- ggplot(incidenceSummaryWeekly) + geom_line(aes(x=Week, y=mean, color=treatment_enrollment_per_PY), size=1) +
#  geom_point(aes(x=Week, y=mean, color=treatment_enrollment_per_PY), size=2) +
  #  scale_x_continuous(breaks=c(2010,2012,2014,2016,2018,2020)) +
  
#  geom_errorbar(aes(x=Week, ymin=mean-sd, ymax=mean+sd, color=treatment_enrollment_per_PY),width=.15) +
  
  #  scale_y_continuous(limits=c(0, 0.4), breaks=seq(0,1.0,0.1)) +
  labs(y="Incidence (per 1000 person-years)", x="Week", color="treatment_enrollment_per_PY") +
  theme_minimal() +
  theme(text = element_text(size=20), legend.position = c(.85, .80), legend.text=element_text(size=20)) +
  guides(color=guide_legend(title="Enrollment"))
ggsave("Treatment Incidence_2.png", plot=p, width=10, height=8)


# Calculate the monthly incidence rate per 1000 person-years which is the yearly sum of 
#   the dt$incidence_daily by the population count 
incidenceMonth <- dt[Year %in% startYear:endYear, .(incidence=1000*sum(incidence_daily/(population_ALL-infected_ALL))), 
                    by=list(Month,treatment_enrollment_per_PY,run)]

# Calculate the mean and std of Weekly incidence rate
incidenceSummaryMonthly <- incidenceMonth[, list(mean=mean(incidence), sd=sd(incidence)), 
                                        by=list(Month,treatment_enrollment_per_PY)]

p <- ggplot(incidenceSummaryMonthly) + geom_line(aes(x=Month, y=mean, color=treatment_enrollment_per_PY), size=1) +
#  geom_point(aes(x=Month, y=mean, color=treatment_enrollment_per_PY), size=2) +
  #  scale_x_continuous(breaks=c(2010,2012,2014,2016,2018,2020)) +
  
  #  geom_errorbar(aes(x=Week, ymin=mean-sd, ymax=mean+sd, color=treatment_enrollment_per_PY),width=.15) +
  
  #  scale_y_continuous(limits=c(0, 0.4), breaks=seq(0,1.0,0.1)) +
  labs(y="Incidence (per 1000 person-years)", x="Month", color="treatment_enrollment_per_PY") +
  theme_minimal() +
  theme(text = element_text(size=20), legend.position = c(.85, .80), legend.text=element_text(size=20)) +
  guides(color=guide_legend(title="Enrollment"))
ggsave("Treatment Incidence_2.png", plot=p, width=10, height=8)


foo <- incidenceMonth[run == 100]

ggplot(foo) + geom_line(aes(x=Month, y=incidence, color=treatment_enrollment_per_PY), size=1) +
  #  geom_point(aes(x=Month, y=mean, color=treatment_enrollment_per_PY), size=2) +
  #  scale_x_continuous(breaks=c(2010,2012,2014,2016,2018,2020)) +
  
  #  geom_errorbar(aes(x=Week, ymin=mean-sd, ymax=mean+sd, color=treatment_enrollment_per_PY),width=.15) +
  
  #  scale_y_continuous(limits=c(0, 0.4), breaks=seq(0,1.0,0.1)) +
  labs(y="Incidence (per 1000 person-years)", x="Month", color="treatment_enrollment_per_PY") +
  theme_minimal() +
  theme(text = element_text(size=20), legend.position = c(.85, .80), legend.text=element_text(size=20)) +
  guides(color=guide_legend(title="Enrollment"))


#Experiment with moving average incidence

window = 365
incidence_daily_rolling_mean = dt[Year %in% startYear:endYear, 
                                     .(tick, Year, run, treatment_enrollment_per_PY, 
                                       rm_inc = 365*1000*rollmean(incidence_daily/(population_ALL-infected_ALL), 
                                                               window, fill = NA))]

foo <- incidence_daily_rolling_mean[run == 100]

ggplot(foo) + geom_line(aes(x=tick, y=rm_inc, color=treatment_enrollment_per_PY), size=1) +
  #  geom_point(aes(x=Month, y=mean, color=treatment_enrollment_per_PY), size=2) +
  #  scale_x_continuous(breaks=c(2010,2012,2014,2016,2018,2020)) +
  
  #  geom_errorbar(aes(x=Week, ymin=mean-sd, ymax=mean+sd, color=treatment_enrollment_per_PY),width=.15) +
  
  #  scale_y_continuous(limits=c(0, 0.4), breaks=seq(0,1.0,0.1)) +
  labs(y="Incidence (per 1000 person-years)", x="Day", color="treatment_enrollment_per_PY") +
  theme_minimal() +
  theme(text = element_text(size=20), legend.position = c(.85, .80), legend.text=element_text(size=20)) +
  guides(color=guide_legend(title="Enrollment"))
