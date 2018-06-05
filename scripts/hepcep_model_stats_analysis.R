#
# Analysis of hepcep model outputs
#
# Eric Tatara
#
library(data.table)
library(ggplot2)

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
      tableList[[d]]  <- fread(path)  
    }, 
      warning = function(w) {
      
    },
      error = function(e) {
      print(paste0("Error loading file: ", path))
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
years <- seq(startYear, (startYear + length(days) - 1))    # list of all sim years in data

# Create a subset of columns to select from the DT
categories <- c("prevalence_ALL", "prevalence_gender_MALE", "prevalence_gender_FEMALE", 
                "prevalence_race_NHWhite", "prevalence_race_NHBlack", "prevalence_race_Hispanic",
                "prevalence_syringesource_HR", "prevalence_syringesource_nonHR",
                "prevalence_agegrp_LEQ_30", "prevalence_agegrp_OVER_30", "prevalence_areatype_CITY",
                "prevalence_areatype_SUBURBAN")

catLabels <- c("ALL", "MALE", "FEMALE", "NHWhite", "NHBlack", "Hispanic", "HR", "nonHR" ,"LEQ30", 
               "Over30", "City", "Suburban")

annualData <- dt[tick %in% days]
#annualData <- annualData[,categories,with=FALSE]
annualData[, Year := years]

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
ggplot(predictionSet, aes(x=cat, y=vals, fill=model)) +
  geom_bar(position=position_dodge(), stat="identity", color="black", alpha=0.5) +
  geom_errorbar(aes(ymin=vals-sd, ymax=vals+sd), width=.2,position=position_dodge(.9)) +
  scale_y_continuous(limits=c(0, 70), breaks=seq(0,70,10)) +
  scale_fill_manual(name="", values = c("APK"="green", "HepCEP"="blue", "NHBS"="red")) +
  labs(y="Prevalence (HCV AB+) %", x="") +
  theme_minimal() +
  theme(text = element_text(size=16), legend.position = c(.2, .9), legend.text=element_text(size=16)) 


# Prevalnce plot by racial groups (PLOS Fog 6)
ggplot(annDataSummary, aes(x=Year_mean)) + 
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
  
  scale_x_continuous(breaks=years) +
  scale_y_continuous(limits=c(0, 0.6), breaks=seq(0,0.6,0.1)) +
  scale_color_manual(name="", values = c("NHWhite"="green", "NHBlack"="blue", "Hispanic"="red","ALL"="black", "HR"="black", "nonHR"="red")) +
  scale_shape_manual(name="", values = c("NHWhite"=23, "NHBlack"=3, "Hispanic"=25,"ALL"=17, "HR"=16, "nonHR"=20)) +
  labs(y="Prevalence (HCV AB+)", x="Year") +
  theme_minimal() +
  theme(text = element_text(size=16), legend.position = c(.25, .25), legend.text=element_text(size=16))

# Prevalnce plot by age and city/suburbs (PLOS Fog 6)
ggplot(annDataSummary, aes(x=Year_mean)) + 
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
  
  scale_x_continuous(breaks=years) +
  scale_y_continuous(limits=c(0, 0.6), breaks=seq(0,0.6,0.1)) +
  scale_color_manual(name="", values = c("City of Chicago"="green", "Suburban"="green", "ALL"="black", "Under 30"="blue", "Over 30"="orange")) +
  scale_shape_manual(name="", values = c("City of Chicago"=18, "Suburban"=4, "ALL"=17, "Under 30"=15, "Over 30"=16)) +
  labs(y="Prevalence (HCV AB+)", x="Year") +
  theme_minimal() +
  theme(text = element_text(size=16), legend.position = c(.25, .25), legend.text=element_text(size=16))

# Prevalnce plot by gender (PLOS Fog 6)
ggplot(annDataSummary, aes(x=Year_mean)) + 
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
  
  scale_x_continuous(breaks=years) +
  scale_y_continuous(limits=c(0, 0.6), breaks=seq(0,0.6,0.1)) +
  scale_color_manual(name="", values = c("ALL"="black", "Male"="dark green", "Female"="purple")) +
  scale_shape_manual(name="", values = c("ALL"=17, "Male"=18, "Female"=15)) +
  labs(y="Prevalence (HCV AB+)", x="Year") +
  theme_minimal() +
  theme(text = element_text(size=16), legend.position = c(.25, .25), legend.text=element_text(size=16))

# Fraction of population by age and city/suburbs (PLOS Fog 7)
ggplot(annDataSummary, aes(x=Year_mean)) + 
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
  
  scale_x_continuous(breaks=years) +
  scale_y_continuous(limits=c(0, 1.0), breaks=seq(0,1.0,0.2)) +
  scale_color_manual(name="", values = c("City of Chicago"="green", "Suburban"="green", "Under 30"="blue", "Over 30"="orange")) +
  scale_shape_manual(name="", values = c("City of Chicago"=18, "Suburban"=4, "Under 30"=15, "Over 30"=16)) +
  labs(y="Fraction of Population", x="Year") +
  theme_minimal() +
  theme(text = element_text(size=16), legend.position = c(.15, .90), legend.text=element_text(size=16))

# Fraction of population by age subgroups (PLOS Fog 7)
ggplot(annDataSummary, aes(x=Year_mean)) + 
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
  
  scale_x_continuous(breaks=years) +
  scale_y_continuous(limits=c(0, 1.0), breaks=seq(0,1.0,0.2)) +
  scale_color_manual(name="", values = c("<=20"="red", "21-30"="blue", "31-40"="green", "41-50"="black", "51-60"="purple")) +
  scale_shape_manual(name="", values = c("<=20"=20, "21-30"=15, "31-40"=16, "41-50"=18, "51-60"=17)) +
  labs(y="Fraction of Population", x="Year") +
  theme_minimal() +
  theme(text = element_text(size=16), legend.position = c(.15, .90), legend.text=element_text(size=16))

# Fraction of population by racial groups (PLOS Fog 7)
ggplot(annDataSummary, aes(x=Year_mean)) + 
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
  
  scale_x_continuous(breaks=years) +
  scale_y_continuous(limits=c(0, 1.0), breaks=seq(0,1.0,0.2)) +
  scale_color_manual(name="", values = c("NHWhite"="orange", "NHBlack"="blue", "Hispanic"="red","ALL"="black", "HR"="black", "nonHR"="red")) +
  scale_shape_manual(name="", values = c("NHWhite"=23, "NHBlack"=3, "Hispanic"=25,"ALL"=17, "HR"=16, "nonHR"=20)) +
  labs(y="Fraction of Population", x="Year") +
  theme_minimal() +
  theme(text = element_text(size=16), legend.position = c(.15, .90), legend.text=element_text(size=16))

