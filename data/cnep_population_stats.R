library(data.table)
library(ggplot2)

cnep <- fread("100k_CNEP+_pwid_catalog_2018-11-23.csv")

cnep_hr <- cnep[Syringe_source=="HR"]
cnep_nonhr <- cnep[Syringe_source=="nonHR"]

p <- ggplot(cnep_nonhr, aes(x=Fraction_recept_sharing)) + 
  geom_histogram(color="darkblue", fill="lightblue")

show(p)

p <- ggplot(cnep_hr, aes(x=Fraction_recept_sharing)) + 
  geom_histogram(color="darkblue", fill="lightblue")

show(p)

p<-ggplot(cnep, aes(x=Fraction_recept_sharing, fill=Syringe_source)) +
  geom_histogram( position="identity", alpha=0.5, color="blue") +
#  lims(y = c(0, 3000)) +
  coord_cartesian(ylim=c(0, 45000)) + 
  #scale_fill_brewer(palette="Dark2") + 
  theme_minimal()+theme(legend.position="top")
show(p)

p<-ggplot(cnep, aes(x=Age, fill=Syringe_source)) +
  geom_histogram( position="identity", alpha=0.5, color="blue") +
  theme_minimal()+theme(legend.position="top")
show(p)

p<-ggplot(cnep, aes(x=Age_Started, fill=Syringe_source)) +
  geom_histogram( position="identity", alpha=0.5, color="blue") +
  theme_minimal()+theme(legend.position="top")
show(p)

p<-ggplot(cnep, aes(x=Race, fill=Syringe_source)) +
  geom_histogram( position="identity", alpha=0.5, color="blue", stat="count") +
  theme_minimal()+theme(legend.position="top")
show(p)

p<-ggplot(cnep, aes(x=Zip, fill=Syringe_source)) +
  geom_histogram( position="identity", alpha=0.5, color="blue", bins=100) +
  lims(x = c(60000, 60700)) +
  theme_minimal()+theme(legend.position="top")
show(p)

p<-ggplot(cnep, aes(x=HCV, fill=Syringe_source)) +
  geom_histogram( position="identity", alpha=0.5, color="blue", stat="count") +
  theme_minimal()+theme(legend.position="top")
show(p)

p<-ggplot(cnep, aes(x=Drug_in_degree, fill=Syringe_source)) +
  geom_histogram( position="identity", alpha=0.5, color="blue") +
  theme_minimal()+theme(legend.position="top")
show(p)

p<-ggplot(cnep, aes(x=Drug_out_degree, fill=Syringe_source)) +
  geom_histogram( position="identity", alpha=0.5, color="blue") +
  theme_minimal()+theme(legend.position="top")
show(p)

p<-ggplot(cnep, aes(x=Daily_injection_intensity, fill=Syringe_source)) +
  geom_histogram( position="identity", alpha=0.5, color="blue") +
  theme_minimal()+theme(legend.position="top")
show(p)

