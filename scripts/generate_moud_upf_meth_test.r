
#
# Unrolled parameter file generator
#
# Eric Tatara
#

#library(gtools)
library(data.table)

enrollmentRateStart = 0.075   # Enrollment start value per_PY
enrollmentRateEnd = 0.075
enrollmentRateStep = .025
replicates <- 10          # Number of replicates per rate param value 

x <- ""
i <- 0

# Range of enrollment parameters to use
range <- seq(enrollmentRateStart,enrollmentRateEnd, by=enrollmentRateStep)

scenarios <- c("REAL","SCENARIO_1","SCENARIO_2","SCENARIO_3")

p_close_range <- seq(0.95, 1.0, 0.001)

rate <- enrollmentRateStart
s <- scenarios[1]

  print(s)  # show progress

  for (p_close in p_close_range){  # Loop over each experimant
    seed <- 0   # Reset the seed counter so that each replicate uses the same set of seeds
      for (rep in 1:replicates){
      i = i + 1
      seed = seed + 1

      x <- paste0(x,"run.number=",i,"\t")
      x <- paste0(x,"random.seed=",seed,"\t")

      x <- paste0(x,"opioid_treatment_enrollment_per_PY=",rate,"\t")

      x <- paste0(x,"opioid_treatment_access_scenario=",s,"\t")
      
      x <- paste0(x,"methadone_p_close=", p_close,"\t")
      
      p_far = 0.5 * p_close
      
      x <- paste0(x,"methadone_p_far=", p_far,"\t")
      
      x <- paste0(x,"methadone_urban_threshold=", 1,"\t")
      x <- paste0(x,"methadone_non_urban_threshold=", 5)
            
      x <- paste0(x,"\n")
    }
  }



write(x, file="upf_moud_meth_test.txt")


