
#
# Unrolled parameter file generator
#
# Eric Tatara
#

library(gtools)
library(data.table)

enrollmentRateStart = 0.025   # Enrollment start value per_PY
enrollmentRateEnd = 0.1
enrollmentRateStep = .025
replicates <- 10          # Number of replicates per rate param value 

x <- ""
i <- 0

# Generate all permutations of the values (0, 0.2, 0.4, 0.6, 0.8, 1) into 5 bins
allpermut <- as.data.table(permutations(6,5,c(0,.2,.4,.6,.8,1), repeats.allowed = TRUE))

# Select only permutations where the sum = 1
validpermut <- allpermut[, Sum := Reduce(`+`, .SD), ][Sum == 1]

# Range of enrollment parameters to use
range <- seq(enrollmentRateStart,enrollmentRateEnd, by=enrollmentRateStep)

npermut <- nrow(validpermut)

for (rate in range){     # Loop over each entrollment 
  
  print(rate)  # show progress
  
  for (p in 1:npermut){  # Loop over each treatment combination
    seed <- 0   # Reset the seed counter so that each replicate uses the same set of seeds
    for (rep in 1:replicates){
      i = i + 1
      seed = seed + 1
      
      x <- paste0(x,"run.number=",i,"\t")
      x <- paste0(x,"random.seed=",seed,"\t")
      x <- paste0(x, "treatment_repeatable = false\t")
      
      x <- paste0(x,"treatment_enrollment_per_PY=",rate,"\t")
      
      row <- validpermut[p,]
      
      x <- paste0(x,"treatment_enrollment_probability_unbiased=",    validpermut[p,1],"\t")
      x <- paste0(x,"treatment_enrollment_probability_HRP=",         validpermut[p,2],"\t")
      x <- paste0(x,"treatment_enrollment_probability_fullnetwork=", validpermut[p,3],"\t")
      x <- paste0(x,"treatment_enrollment_probability_inpartner=",   validpermut[p,4],"\t")
      x <- paste0(x,"treatment_enrollment_probability_outpartner=",  validpermut[p,5])
      
      
      x <- paste0(x,"\n")
      
    }
  }
}
write(x, file="upf_enrollment_method_sweep_no_retreat.txt")




