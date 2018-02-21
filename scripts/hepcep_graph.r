library(igraph)
library(data.table)
library(ggplot2)

file <- "../output/net_initial_11.gml" 

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

youngDist <- data.frame(distance=c("2","4","8","16","32","64","64+"), 
                        percent=n)

# Force the frame levels to specific order
youngDist$distance <- factor(youngDist$distance, levels=c("2","4","8","16","32","64","64+"))


ggplot(youngDist, aes(x=distance, y=percent)) +
  geom_bar(stat="identity",fill="green", alpha = 0.5, color="black") +
  theme_minimal() +
  theme(text = element_text(size=24)) +
  scale_y_continuous(breaks=seq(0, 35, 5))  # Ticks from 0-10, every .25


