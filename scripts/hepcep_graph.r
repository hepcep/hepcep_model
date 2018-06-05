#
# Analysis of hepcep network structure
#
# Eric Tatara
#

library(igraph)
library(data.table)
library(ggplot2)

file <- "net_initial.gml" 

g <- read.graph(file, format="gml")

in_deg_dist <- degree.distribution(g, mode="in")
out_deg_dist <- degree.distribution(g, mode="out")

# Boolean array of vertex indices where age < 30
young <- vertex_attr(g, "age") < 30

dfedges <- as_data_frame(g, what = "edges")
dfverts <- as_data_frame(g, what = "vertices")

dtverts <- as.data.table(dfverts)
dtedges <- as.data.table(dfedges)

young_verts <- dtverts[age<30]
young_ids <- young_verts$id

#connected_young_verts <- young_verts[drug_in_deg > 0 & drug_out_deg > 0]

young_edges <- dtedges[from %in% young_ids & to %in% young_ids]

# TODO filtering out duplicate edges
#young_edges_2 <- young_edges[!(from %in% to)]
young_edges_2 <- young_edges

group1 <- young_edges_2[distance<=2]
group2 <- young_edges_2[distance>2 & distance<=4]
group3 <- young_edges_2[distance>4 & distance<=8]
group4 <- young_edges_2[distance>8 & distance<=16]
group5 <- young_edges_2[distance>16 & distance<=32]
group6 <- young_edges_2[distance>32 & distance<=64]
group7 <- young_edges_2[distance>64]

n <- array()
n[1] <- nrow(group1)/nrow(young_edges_2) * 100
n[2] <- nrow(group2)/nrow(young_edges_2) * 100
n[3] <- nrow(group3)/nrow(young_edges_2) * 100
n[4] <- nrow(group4)/nrow(young_edges_2) * 100
n[5] <- nrow(group5)/nrow(young_edges_2) * 100
n[6] <- nrow(group6)/nrow(young_edges_2) * 100
n[7] <- nrow(group7)/nrow(young_edges_2) * 100

catLabels <- c("2","4","8","16","32","64","64+")
youngDist <- data.frame(cat=catLabels, vals=n, sd=0, model="HepCEP")

# Force the frame levels to specific order
youngDist$distance <- factor(youngDist$distance, levels=catLabels)

# Values from the PLOS paper for comparison
# Values from network_apk_vs_boodram-2015-01-31.xlsx
df <- data.frame(cat=catLabels, vals= c(29,12,14,16,14,11,4), 
                 sd=c(1.2,1.4,1.4,1.4,1.4,1.4,1.4), model="APK")

youngDist <- rbind(youngDist,df)

df <- data.frame(cat=catLabels, vals= c(30,11,13,17,12,11,6), 
                 sd=c(3.6,4,4,4,4,4,4), model="YSN")
youngDist <- rbind(youngDist,df)

# Prevalence for all groups in a single year (PLOS Fog 4)
ggplot(youngDist, aes(x=cat, y=vals, fill=model)) +
  geom_bar(position=position_dodge(), stat="identity", color="black", alpha=0.5) +
  geom_errorbar(aes(ymin=vals-sd, ymax=vals+sd), width=.2,position=position_dodge(.9)) +
  scale_y_continuous(limits=c(0, 35), breaks=seq(0,70,10)) +
  scale_fill_manual(name="", values = c("APK"="green", "HepCEP"="blue", "YSN"="red")) +
  labs(x="Distance for a pair of connected young PWID (km)") +
  theme_minimal() +
  theme(text = element_text(size=16), legend.position = c(.6, .9), legend.text=element_text(size=16)) 

