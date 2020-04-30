library(data.table)
a0 <- fread("../../experiments/moud_3/run_0/agents.csv")

zip_N_a0 <- a0[Tick == 0, .N, by="Zip Code"]
table(zip_N_a0[,N])
str(zip_N_a0)

chiZIPs <- zip_N_a0[`Zip Code` < 60700 & `Zip Code` >= 60600,`Zip Code`]
nonChiZIPs <- zip_N_a0[!(`Zip Code` %in% chiZIPs), `Zip Code`]

# Check that the math works
length(chiZIPs)
length(nonChiZIPs)
length(a0[,unique(`Zip Code`)])

metrics <-  fread("../data/min_dist_real_reshuffled.csv")

thresholds <- metrics

methadone_urban_threshold = 2
methadone_non_urban_threshold = 10

buprenorphine_urban_threshold = 5
buprenorphine_non_urban_threshold = 20

naltrexone_urban_threshold = 5
naltrexone_non_urban_threshold = 20

# buprenorphine
thresholds[zip %in% chiZIPs, real_bup_close := ifelse(s2_bup < buprenorphine_urban_threshold, TRUE, FALSE) ]
thresholds[zip %in% chiZIPs] # all TRUE
thresholds[zip %in% nonChiZIPs, real_bup_close := ifelse(s2_bup < buprenorphine_non_urban_threshold, TRUE, FALSE) ]
thresholds[zip %in% nonChiZIPs, real_bup_close]

# methadone
thresholds[zip %in% chiZIPs, real_meth_close := ifelse(s2_meth < methadone_urban_threshold, TRUE, FALSE) ]
thresholds[zip %in% chiZIPs] # some FALSE
thresholds[zip %in% nonChiZIPs, real_meth_close := ifelse(s2_meth < methadone_non_urban_threshold, TRUE, FALSE) ]
thresholds[zip %in% nonChiZIPs, real_meth_close]

# naltrexone
thresholds[zip %in% chiZIPs, real_nal_close := ifelse(s2_nal < naltrexone_urban_threshold, TRUE, FALSE) ]
thresholds[zip %in% chiZIPs] # all TRUE
thresholds[zip %in% nonChiZIPs, real_nal_close := ifelse(s2_nal < naltrexone_non_urban_threshold, TRUE, FALSE) ]
thresholds[zip %in% nonChiZIPs, real_nal_close]

# Get counts for B, M, N
counts <- zip_N_a0[,.(zip = `Zip Code`, N = N)][thresholds[,.(zip,real_bup_close,real_meth_close,real_nal_close)], on = "zip"]
bup_close <- counts[(real_bup_close),sum(N)]
bup_far <- counts[!(real_bup_close),sum(N)]
meth_close <- counts[(real_meth_close),sum(N)]
meth_far <- counts[!(real_meth_close),sum(N)]
nal_close <- counts[(real_nal_close),sum(N)]
nal_far <- counts[!(real_nal_close),sum(N)]

bup_close
bup_far
meth_close
meth_far
nal_close
nal_far
