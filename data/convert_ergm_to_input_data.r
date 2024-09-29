library(network)
library(data.table)

setwd("E:\\ANL\\Projects\\HepCEP\\hepcep_networks\\simulate-from-ergms\\out")

load("on-revamped-oscar-non-randomized-indeg-0-outdeg-0.RData")

# Grab an edge list instance
#class(sim_results[[1]])
list.vertex.attributes(sim_results[[1]])
el <- as.edgelist(sim_results[[1]])

#net <- sim_results[[1]]
#class(net)

sim_results <- NULL  # clear huge list

## vertex.att.all object is a nested list of 100 elements, 
## Each of the 100 lists is composed of a list of 19 elements, corresponding to the attributes below:
vertex.att.all <- readRDS("vertex_att_all_oct122023.RDS")

y <- vertex.att.all[[2]]

# pwid_w_vertex_names is a dataframe consisting of 31,999 rows and 16 cols
# the column names correspond to > colnames(pwid_w_vertex_names)
pwid_w_vertex_names <- readRDS("pwid_w_vertex_names_oct122023.RDS")

pwid_dt <- as.data.table(pwid_w_vertex_names)

pwid_dt[, c("agecat", "age_lb","age_ub","lon","lat","zipcode"):=NULL]  # remove columns

# Rename columns to match existing HepCEP PWID input data schema

setnames(pwid_dt, c("age"), c("Age"))

setnames(pwid_dt, c("sex"), c("Gender"))
setnames(pwid_dt, c("race"), c("Race"))
setnames(pwid_dt, c("age_started"), c("Age_Started"))
setnames(pwid_dt, c("hcv_status"), c("HCV"))
setnames(pwid_dt, c("syringe_source"), c("Syringe_source"))
setnames(pwid_dt, c("fraction_recept_sharing"), c("Fraction_recept_sharing"))
setnames(pwid_dt, c("daily_injection_intensity"), c("Daily_injection_intensity"))

# Network in/out degree should be the initial condition of the network
pwid_dt$Drug_in_degree <- 0	
pwid_dt$Drug_out_degree <- 0	

# Rename the attributes to match the HepCEP input scheme
pwid_dt[Gender == "M", Gender := "Male"]
pwid_dt[Gender == "F", Gender := "Female"]

pwid_dt[Race == "Wh", Race := "NHWhite"]
pwid_dt[Race == "Bl", Race := "NHBlack"]
pwid_dt[Race == "Hi", Race := "Hispanic"]
pwid_dt[Race == "Ot", Race := "Other"]

pwid_dt[, Syringe_source := as.character(Syringe_source)]
pwid_dt[Syringe_source == "0", Syringe_source := "nonHR"]
pwid_dt[Syringe_source == "1", Syringe_source := "HR"]

# TODO Hack to fix NA but should be fixed in the ERGM data
pwid_dt[is.na(Syringe_source), Syringe_source := "nonHR"]

pwid_dt[HCV == "sucseptible", HCV := "susceptible"]
pwid_dt[HCV == "acute", HCV := "infectiousacute"]

# Set the data column order to match the HepCEP input data schema
setcolorder(pwid_dt, c("Age", "Age_Started", "Gender", "Race", "Syringe_source", "Zip",	
                      "HCV","Drug_in_degree",	"Drug_out_degree", 
                      "Daily_injection_intensity","Fraction_recept_sharing"))

fwrite(pwid_dt, "L:\\HepCEP\\hepcep_model\\data\\ergm_pwid_catalog_oct122023.csv")

# Convert the edge table V1 -> V2 to a table V1 -> V21, V22, V23...
#  eg for every unique V1, create one V1 row and map all V2s to it.
edges_dt <- as.data.table(el)
# Group by values in V1, and paste all values in other cols together separated by comma
compact_edges_dt <- edges_dt[, lapply(.SD, paste0, collapse = ","), by=V1]

fwrite(compact_edges_dt, "L:\\HepCEP\\hepcep_model\\data\\ergm_pwid_edges_oct122023.csv", quote=F)

