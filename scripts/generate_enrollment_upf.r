
#
# Unrolled parameter file generator
#
# Eric Tatara
#

enrollmentRateStart = 0   # Enrollment start value per_PY
enrollmentRateEnd = 0.1
enrollmentRateStep = 0.025
replicates <- 20          # Number of replicates per rate param value 

x <- ""
i <- 0

# Range of enrollment parameters to use
range <- seq(enrollmentRateStart,enrollmentRateEnd, by=enrollmentRateStep)

for (rate in range){
  seed <- 0   # Reset the seed counter so that each replicate uses the same set of seeds
  for (rep in 1:replicates){
    i = i + 1
    seed = seed + 1
  
    x <- paste0(x,"run.number=",i,"\t")
    x <- paste0(x,"random.seed=",seed,"\t")
  
    x <- paste0(x,"treatment_enrollment_per_PY=",rate,"\t")
  
    x <- paste0(x, "treatment_repeatable = false")
    
    x <- paste0(x,"\n")
  
  }
}

write(x, file="upf_enrollment_sweep_retreat2.txt")