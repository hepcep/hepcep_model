
#
# Unrolled parameter file generator
#
# Eric Tatara
#

library(gtools)
library(data.table)

enrollmentRateStart = 0.0   # Enrollment start value per_PY
enrollmentRateEnd = 0.1
enrollmentRateStep = .025
replicates <- 20          # Number of replicates per rate param value 

x <- ""
i <- 0

# Range of enrollment parameters to use
range <- seq(enrollmentRateStart,enrollmentRateEnd, by=enrollmentRateStep)

scenarios <- c("REAL","SCENARIO_1","SCENARIO_2")

for (rate in range){     # Loop over each entrollment 
  
  print(rate)  # show progress
  
  for (s in scenarios){  # Loop over each scenario
    seed <- 0   # Reset the seed counter so that each replicate uses the same set of seeds
    for (rep in 1:replicates){
      i = i + 1
      seed = seed + 1
      
      x <- paste0(x,"run.number=",i,"\t")
      x <- paste0(x,"random.seed=",seed,"\t")
      
      x <- paste0(x,"opioid_treatment_enrollment_per_PY=",rate,"\t")
      
      x <- paste0(x,"opioid_treatment_access_scenario=",  s)
      x <- paste0(x,"\n")
    }
  }
}
write(x, file="upf_moud_1.txt")




