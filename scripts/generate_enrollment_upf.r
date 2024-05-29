
#
# Unrolled parameter file generator
#
# Eric Tatara
#

enrollmentRateStart = .075   # Enrollment start value per_PY
enrollmentRateEnd = 0.075
enrollmentRateStep = 0.0
replicates <- 20          # Number of replicates per rate param value 

#non_adherence <- c(0.1, 0.2, 0.3, 0.4)
non_adherence <- c(0.1)
#max_num_daa_treatments <- c(1,2,3,4)
#svr <- c(0.9, 0.8, 0.7, 0.6)
immunology_types <- c("VK", "APK")

reduced_treatment_enrollment_per_PY_start = 0.025
reduced_treatment_enrollment_per_PY_end = 0.2
reduced_treatment_enrollment_per_PY_step = 0.025

x <- ""
i <- 0

# Range of enrollment parameters to use
#treat_enroll_range <- seq(enrollmentRateStart,enrollmentRateEnd, by=enrollmentRateStep)

treat_enroll_range <- c(0, 0.075, 0.2, 0.4, 0.8, 1.0, 1.25, 1.5, 0.75, 2.0)

reduced_treat_enroll_range <- seq(reduced_treatment_enrollment_per_PY_start,reduced_treatment_enrollment_per_PY_end, by=reduced_treatment_enrollment_per_PY_step)

for (s in non_adherence){
  for (t in immunology_types){
    for (rate in treat_enroll_range){
#      for (reduced_rate in reduced_treat_enroll_range){
        seed <- 0   # Reset the seed counter so that each replicate uses the same set of seeds
        for (rep in 1:replicates){
          i = i + 1
          seed = seed + 1
        
          x <- paste0(x,"run.number=",i,"\t")
          x <- paste0(x,"random.seed=",seed,"\t")
          x <- paste0(x,"treatment_enrollment_per_PY=",rate,"\t")
#          x <- paste0(x,"reduced_treatment_enrollment_per_PY=",reduced_rate,"\t")
#          x <- paste0(x, "treatment_repeatable=true\t")
#          x <- paste0(x, "treatment_nonadherence=", s,"\t")
          #x <- paste0(x, "max_num_daa_treatments=", t)
          x <- paste0(x, "immunology.type=", t)
        
          x <- paste0(x,"\n")
    
#      }
    }
  }
}
}
write(x, file="upf_enrollment_sweep_reduce_APK_VK.txt")
