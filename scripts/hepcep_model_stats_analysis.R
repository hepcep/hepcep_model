#
# Analysis of hepcep model outputs
#
# Eric Tatara
#
library(data.table)
library(ggplot2)

fileName <- "../output/stats.csv"
dt <- fread(fileName)

rows <- nrow(dt)
burninDays <- 365

# Day samples at the END (day 365) of each simulation year
days <- seq((burninDays+365), rows, 365)

startYear <- 2010
years <- seq(startYear, (startYear + length(days) - 1))

annualData <- dt[tick %in% days]

annualData[, Year := years]

# Year for which we will generate a prediction data set
predictionYear <- 2012
predictionData <- annualData[Year==predictionYear,]

categories <- c("prevalence_ALL", "prevalence_gender_MALE", "prevalence_gender_FEMALE", 
                "prevalence_race_NHWhite", "prevalence_race_NHBlack", "prevalence_race_Hispanic",
                "prevalence_syringesource_HR", "prevalence_syringesource_nonHR",
                "prevalence_agegrp_LEQ_30", "prevalence_agegrp_OVER_30", "prevalence_areatype_CITY",
                "prevalence_areatype_SUBURBAN")

catLabels <- c("ALL", "MALE", "FEMALE", "NHWhite", "NHBlack", "Hispanic", "HR", "nonHR" ,"LEQ30", 
               "Over30", "City", "Suburban")

plotData <- predictionData[,categories,with=FALSE]

df <- data.frame(cat=catLabels,vals=as.numeric(plotData[1]))

# Force the frame levels to specific order
df$cat <- factor(df$cat, levels=catLabels)

ggplot(df, aes(cat,vals*100)) +
  geom_col(fill="blue", alpha = 0.5, color="black") +
  theme_minimal() +
  theme(text = element_text(size=16)) +
  scale_y_continuous(limits=c(0, 70), breaks=seq(0,70,10)) +
  labs(title="HepCEP Model output (single run)", y="Prevalence (HCV AB+) %", x="")



# Prevalnce plot by racial groups
ggplot(annualData, aes(x=Year)) + 
  geom_line(aes(y=prevalence_race_NHWhite, color='NHWhite'), size=1.5) +
  geom_point(aes(y=prevalence_race_NHWhite, color='NHWhite', shape='NHWhite'), size=4) +
  geom_line(aes(y=prevalence_race_NHBlack, color='NHBlack'), size=1.5) +
  geom_point(aes(y=prevalence_race_NHBlack, color='NHBlack', shape='NHBlack'), size=4) +
  geom_line(aes(y=prevalence_race_Hispanic, color='Hispanic'), size=1.5) +
  geom_point(aes(y=prevalence_race_Hispanic, color='Hispanic', shape='Hispanic'), size=4) +
  geom_line(aes(y=prevalence_ALL, color='ALL'), size=1.5) +
  geom_point(aes(y=prevalence_ALL, color='ALL', shape='ALL'), size=4) +
  geom_line(aes(y=prevalence_syringesource_HR, color='HR'), size=1.5) +
  geom_point(aes(y=prevalence_syringesource_HR, color='HR', shape='HR'), size=4) +
  geom_line(aes(y=prevalence_syringesource_nonHR, color='nonHR'), size=1.5) +
  geom_point(aes(y=prevalence_syringesource_nonHR, color='nonHR', shape='nonHR'), size=4) +
  scale_x_continuous(breaks=years) +
  scale_y_continuous(limits=c(0, 0.6), breaks=seq(0,0.6,0.1)) +
  scale_color_manual(name="", values = c("NHWhite"="green", "NHBlack"="blue", "Hispanic"="red","ALL"="black", "HR"="black", "nonHR"="red")) +
  scale_shape_manual(name="", values = c("NHWhite"=23, "NHBlack"=3, "Hispanic"=25,"ALL"=17, "HR"=16, "nonHR"=20)) +
  labs(title="HepCEP Model output (single run)", y="Prevalence (HCV AB+)", x="Year") +
  theme_minimal() +
  theme(text = element_text(size=16), legend.position = c(.25, .25), legend.text=element_text(size=16))
  
# Prevalnce plot by age and city/suburbs
ggplot(annualData, aes(x=Year)) + 
  geom_line(aes(y=prevalence_areatype_CITY, color='City of Chicago'), size=1.5) +
  geom_point(aes(y=prevalence_areatype_CITY, color='City of Chicago', shape='City of Chicago'), size=4) +
  geom_line(aes(y=prevalence_areatype_SUBURBAN, color='Suburban'), size=1.5) +
  geom_point(aes(y=prevalence_areatype_SUBURBAN, color='Suburban', shape='Suburban'), size=4) +
  geom_line(aes(y=prevalence_ALL, color='ALL'), size=1.5) +
  geom_point(aes(y=prevalence_ALL, color='ALL', shape='ALL'), size=4) +
  geom_line(aes(y=prevalence_agegrp_LEQ_30, color='Under 30'), size=1.5) +
  geom_point(aes(y=prevalence_agegrp_LEQ_30, color='Under 30', shape='Under 30'), size=4) +
  geom_line(aes(y=prevalence_agegrp_OVER_30, color='Over 30'), size=1.5) +
  geom_point(aes(y=prevalence_agegrp_OVER_30, color='Over 30', shape='Over 30'), size=4) +
  scale_x_continuous(breaks=years) +
  scale_y_continuous(limits=c(0, 0.6), breaks=seq(0,0.6,0.1)) +
  scale_color_manual(name="", values = c("City of Chicago"="green", "Suburban"="green", "ALL"="black", "Under 30"="blue", "Over 30"="yellow")) +
  scale_shape_manual(name="", values = c("City of Chicago"=18, "Suburban"=4, "ALL"=17, "Under 30"=15, "Over 30"=16)) +
  labs(title="HepCEP Model output (single run)", y="Prevalence (HCV AB+)", x="Year") +
  theme_minimal() +
  theme(text = element_text(size=16), legend.position = c(.25, .25), legend.text=element_text(size=16))

# Prevalnce plot by gender
ggplot(annualData, aes(x=Year)) + 
  geom_line(aes(y=prevalence_ALL, color='ALL'), size=1.5) +
  geom_point(aes(y=prevalence_ALL, color='ALL', shape='ALL'), size=4) +
  geom_line(aes(y=prevalence_gender_MALE, color='Male'), size=1.5) +
  geom_point(aes(y=prevalence_gender_MALE, color='Male', shape='Male'), size=4) +
  geom_line(aes(y=prevalence_gender_FEMALE, color='Female'), size=1.5) +
  geom_point(aes(y=prevalence_gender_FEMALE, color='Female', shape='Female'), size=4) +
  scale_x_continuous(breaks=years) +
  scale_y_continuous(limits=c(0, 0.6), breaks=seq(0,0.6,0.1)) +
  scale_color_manual(name="", values = c("ALL"="black", "Male"="dark green", "Female"="purple")) +
  scale_shape_manual(name="", values = c("ALL"=17, "Male"=18, "Female"=15)) +
  labs(title="HepCEP Model output (single run)", y="Prevalence (HCV AB+)", x="Year") +
  theme_minimal() +
  theme(text = element_text(size=16), legend.position = c(.25, .25), legend.text=element_text(size=16))

# Fraction of population by age and city/suburbs
ggplot(annualData, aes(x=Year)) + 
  geom_line(aes(y=fraction_areatype_CITY, color='City of Chicago'), size=1.5) +
  geom_point(aes(y=fraction_areatype_CITY, color='City of Chicago', shape='City of Chicago'), size=4) +
  geom_line(aes(y=fraction_areatype_SUBURBAN, color='Suburban'), size=1.5) +
  geom_point(aes(y=fraction_areatype_SUBURBAN, color='Suburban', shape='Suburban'), size=4) +
  geom_line(aes(y=fraction_agegrp_LEQ_30, color='Under 30'), size=1.5) +
  geom_point(aes(y=fraction_agegrp_LEQ_30, color='Under 30', shape='Under 30'), size=4) +
  geom_line(aes(y=fraction_agegrp_OVER_30, color='Over 30'), size=1.5) +
  geom_point(aes(y=fraction_agegrp_OVER_30, color='Over 30', shape='Over 30'), size=4) +
  scale_x_continuous(breaks=years) +
  scale_y_continuous(limits=c(0, 1.0), breaks=seq(0,1.0,0.2)) +
  scale_color_manual(name="", values = c("City of Chicago"="green", "Suburban"="green", "Under 30"="blue", "Over 30"="yellow")) +
  scale_shape_manual(name="", values = c("City of Chicago"=18, "Suburban"=4, "Under 30"=15, "Over 30"=16)) +
  labs(title="HepCEP Model output (single run)", y="Fraction of Population", x="Year") +
  theme_minimal() +
  theme(text = element_text(size=16), legend.position = c(.15, .90), legend.text=element_text(size=16))

# Fraction of population by age subgroups
ggplot(annualData, aes(x=Year)) + 
  geom_line(aes(y=fraction_agedec_AGE_LEQ_20, color='<=20'), size=1.5) +
  geom_point(aes(y=fraction_agedec_AGE_LEQ_20, color='<=20', shape='<=20'), size=4) +
  geom_line(aes(y=fraction_agedec_AGE_21_30, color='21-30'), size=1.5) +
  geom_point(aes(y=fraction_agedec_AGE_21_30, color='21-30', shape='21-30'), size=4) +
  geom_line(aes(y=fraction_agedec_AGE_31_40, color='31-40'), size=1.5) +
  geom_point(aes(y=fraction_agedec_AGE_31_40, color='31-40', shape='31-40'), size=4) +
  geom_line(aes(y=fraction_agedec_AGE_41_50, color='41-50'), size=1.5) +
  geom_point(aes(y=fraction_agedec_AGE_41_50, color='41-50', shape='41-50'), size=4) +
  geom_line(aes(y=fraction_agedec_AGE_51_60, color='51-60'), size=1.5) +
  geom_point(aes(y=fraction_agedec_AGE_51_60, color='51-60', shape='51-60'), size=4) +
  scale_x_continuous(breaks=years) +
  scale_y_continuous(limits=c(0, 1.0), breaks=seq(0,1.0,0.2)) +
  scale_color_manual(name="", values = c("<=20"="red", "21-30"="blue", "31-40"="green", "41-50"="black", "51-60"="purple")) +
  scale_shape_manual(name="", values = c("<=20"=20, "21-30"=15, "31-40"=16, "41-50"=18, "51-60"=17)) +
  labs(title="HepCEP Model output (single run)", y="Fraction of Population", x="Year") +
  theme_minimal() +
  theme(text = element_text(size=16), legend.position = c(.15, .90), legend.text=element_text(size=16))

# Fraction of population by racial groups
ggplot(annualData, aes(x=Year)) + 
  geom_line(aes(y=fraction_race_NHWhite, color='NHWhite'), size=1.5) +
  geom_point(aes(y=fraction_race_NHWhite, color='NHWhite', shape='NHWhite'), size=4) +
  geom_line(aes(y=fraction_race_NHBlack, color='NHBlack'), size=1.5) +
  geom_point(aes(y=fraction_race_NHBlack, color='NHBlack', shape='NHBlack'), size=4) +
  geom_line(aes(y=fraction_race_Hispanic, color='Hispanic'), size=1.5) +
  geom_point(aes(y=fraction_race_Hispanic, color='Hispanic', shape='Hispanic'), size=4) +
  geom_line(aes(y=fraction_syringesource_HR, color='HR'), size=1.5) +
  geom_point(aes(y=fraction_syringesource_HR, color='HR', shape='HR'), size=4) +
  geom_line(aes(y=fraction_syringesource_nonHR, color='nonHR'), size=1.5) +
  geom_point(aes(y=fraction_syringesource_nonHR, color='nonHR', shape='nonHR'), size=4) +
  scale_x_continuous(breaks=years) +
  scale_y_continuous(limits=c(0, 1.0), breaks=seq(0,1.0,0.2)) +
  scale_color_manual(name="", values = c("NHWhite"="green", "NHBlack"="blue", "Hispanic"="red","ALL"="black", "HR"="black", "nonHR"="red")) +
  scale_shape_manual(name="", values = c("NHWhite"=23, "NHBlack"=3, "Hispanic"=25,"ALL"=17, "HR"=16, "nonHR"=20)) +
  labs(title="HepCEP Model output (single run)", y="Fraction of Population", x="Year") +
  theme_minimal() +
  theme(text = element_text(size=16), legend.position = c(.15, .90), legend.text=element_text(size=16))



