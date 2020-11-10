
#
# Unrolled parameter file generator
#
# Eric Tatara
#

#library(gtools)
library(data.table)

# Read in a list of experiments which is a CSV table of parameter values
experiments <- fread("moud_LMH_combos_b7.csv")

# Insert the column name in each value cell with a '=' separator 
experiments[] <- Map(paste, names(experiments), experiments, sep = "=")

# Create a string array of UPF name=value tab-separated pairs
exp_str_array = apply(experiments,1,paste,collapse = "\t")

enrollmentRateStart = 0.075   # Enrollment start value per_PY
enrollmentRateEnd = 0.075
enrollmentRateStep = .025
replicates <- 20          # Number of replicates per rate param value 

x <- ""
i <- 0

# Range of enrollment parameters to use
range <- seq(enrollmentRateStart,enrollmentRateEnd, by=enrollmentRateStep)

scenarios <- c("REAL","SCENARIO_1","SCENARIO_2","SCENARIO_3")

# This is the prior way to generate UPF lines that use ranges
# for (rate in range){     # Loop over each entrollment 
#   
#   print(rate)  # show progress
#   
#   for (s in scenarios){  # Loop over each scenario
#     seed <- 0   # Reset the seed counter so that each replicate uses the same set of seeds
#     for (rep in 1:replicates){
#       i = i + 1
#       seed = seed + 1
#       
#       x <- paste0(x,"run.number=",i,"\t")
#       x <- paste0(x,"random.seed=",seed,"\t")
#       
#       x <- paste0(x,"opioid_treatment_enrollment_per_PY=",rate,"\t")
#       
#       x <- paste0(x,"opioid_treatment_access_scenario=",  s)
#       x <- paste0(x,"\n")
#     }
#   }
# }

rate <- enrollmentRateStart
s <- scenarios[1]

  print(rate)  # show progress

  for (ex in exp_str_array){  # Loop over each experimant
    seed <- 0   # Reset the seed counter so that each replicate uses the same set of seeds
    for (rep in 1:replicates){
      i = i + 1
      seed = seed + 1

      x <- paste0(x,"run.number=",i,"\t")
      x <- paste0(x,"random.seed=",seed,"\t")

      x <- paste0(x,"opioid_treatment_enrollment_per_PY=",rate,"\t")

      x <- paste0(x,"opioid_treatment_access_scenario=",s,"\t")
      
      x <- paste0(x, ex)  # Paste the experiment parameters
      
      x <- paste0(x,"\n")
    }
  }



write(x, file="upf_moud_b7_REAL.txt")




