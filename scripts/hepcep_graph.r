library(igraph)
library(data.table)
library(ggplot2)

file <- "../output/net_initial_5.gml" 

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

young_edges <- dtedges[from %in% young_ids & to %in% young_ids]

group1 <- young_edges[distance<=2]
group2 <- young_edges[distance>2 & distance<=4]
group3 <- young_edges[distance>4 & distance<=8]
group4 <- young_edges[distance>8 & distance<=16]
group5 <- young_edges[distance>16 & distance<=32]
group6 <- young_edges[distance>32 & distance<=64]
group7 <- young_edges[distance>64]

ggplot(data=young_edges, aes(young_edges$distance)) + 
  geom_histogram(breaks=c(0, 2, 4, 8, 16, 32, 64), 
                 col="red", 
                 fill="green", 
                 alpha = .2) 