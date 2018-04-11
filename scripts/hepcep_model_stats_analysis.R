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

groups <- c("NHWHite","NHBlack")

ggplot(annualData, aes(x=Year)) + 
  geom_line(aes(y=prevalence_race_NHWhite, color='NHWhite')) +
  geom_point(aes(y=prevalence_race_NHWhite, color='NHWhite', shape='NHWhite'), size=2) +
  geom_line(aes(y=prevalence_race_NHBlack, color='NHBlack')) +
  geom_point(aes(y=prevalence_race_NHBlack, color='NHBlack', shape='NHBlack'), size=2) +
  geom_line(aes(y=prevalence_race_Hispanic, color='Hispanic')) +
  geom_point(aes(y=prevalence_race_Hispanic, color='Hispanic', shape='Hispanic'), size=2) +
  geom_line(aes(y=prevalence_ALL, color='ALL')) +
  geom_point(aes(y=prevalence_ALL, color='ALL', shape='ALL'), size=2) +
  geom_line(aes(y=prevalence_syringesource_HR, color='HR')) +
  geom_point(aes(y=prevalence_syringesource_HR, color='HR', shape='HR'), size=2) +
  geom_line(aes(y=prevalence_syringesource_nonHR, color='nonHR')) +
  geom_point(aes(y=prevalence_syringesource_nonHR, color='nonHR', shape='nonHR'), size=2) +
  scale_x_continuous(breaks=years) +
  scale_y_continuous(limits=c(0, 0.6), breaks=seq(0,0.6,0.1)) +
  scale_color_manual(name="", values = c("NHWhite"="green", "NHBlack"="blue", "Hispanic"="red","ALL"="black", "HR"="black", "nonHR"="red")) +
  scale_shape_manual(name="", values = c("NHWhite"=5, "NHBlack"=3, "Hispanic"=6,"ALL"=2, "HR"=1, "nonHR"=20)) +
  labs(title="HepCEP Model output (single run)", y="Prevalence (HCV AB+)", x="Year") +
  theme_minimal() +
  theme(text = element_text(size=16), legend.position = c(.25, .25))
  