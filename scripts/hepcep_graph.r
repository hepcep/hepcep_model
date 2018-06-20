#
# Analysis of hepcep network structure
#
# Eric Tatara
#

library(igraph)
library(data.table)
library(ggplot2)

# Load all of the graph files that exist in an experiments dir
fileName <- "net_initial.gml"

# TODO change to list dirs for new runs, currently assumes all graph files are in /output
#dirs <- list.dirs (path=".", recursive=FALSE)
files <- list.files(path=".",pattern="*.gml" );

graphList <- list()
#for (d in dirs){
#  path <- paste0(d,fileName)
for (path in files){
  
  if (!file.exists(path)){
    print(paste0("File doesnt exist! ",path))
  }
  else{
    print(paste0("Loading ", path ))
    
    tryCatch({
      graphList[[path]]  <- read.graph(path, format="gml") 
    }, 
    warning = function(w) {
      
    },
    error = function(e) {
      print(paste0("Error loading file: ", path))
    }, 
    finally = {
    }
    )
    
  }
}

calcYoungDistance <- function (g){

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

  distLabels <- c("2","4","8","16","32","64","64+")
  youngDist <- data.frame(distance=distLabels, vals=n)
  
  # Force the frame levels to specific order
  youngDist$distance <- factor(youngDist$distance, levels=distLabels)
  
  return(youngDist)

}

youngDistList <- lapply(graphList, calcYoungDistance)
youngDistTable <- rbindlist(youngDistList)

distStats<-aggregate(youngDistTable$vals, list(distance = youngDistTable$distance), mean)
colnames(distStats) <- c("distance", "mean")

distStdDev <- aggregate(youngDistTable$vals, list(distance = youngDistTable$distance), sd)
colnames(distStdDev) <- c("distance", "sd")

distStats$sd <- 2*distStdDev$sd
distStats$model <- "HepCEP"

# Values from the PLOS paper for comparison
# Values from network_apk_vs_boodram-2015-01-31.xlsx
distLabels <- c("2","4","8","16","32","64","64+")
df <- data.frame(distance=distLabels, mean= c(29,12,14,16,14,11,4), 
                 sd=c(1.2,1.4,1.4,1.4,1.4,1.4,1.4), model="APK")

distStats <- rbind(distStats,df)

df <- data.frame(distance=distLabels, mean= c(30,11,13,17,12,11,6), 
                 sd=c(3.6,4,4,4,4,4,4), model="YSN")
distStats <- rbind(distStats,df)

# Prevalence for all groups in a single year (PLOS Fog 4)
ggplot(distStats, aes(x=distance, y=mean/100, fill=model)) +
  geom_bar(position=position_dodge(), stat="identity", color="black", alpha=0.5) +
  geom_errorbar(aes(ymin=(mean-sd)/100, ymax=(mean+sd)/100), width=.2,position=position_dodge(.9)) +
  scale_y_continuous(limits=c(0, 0.35)) +
  scale_fill_manual(name="", values = c("APK"="green", "HepCEP"="blue", "YSN"="red")) +
  labs(x="Distance for a pair of connected young PWID (km)", y="Fraction of PWID Pairs") +
  scale_x_discrete(labels=c("[0-2)","[2-4)","[4-8)","[8-16)","[16-32)","[32-64)","64+")) + 
  theme_minimal() +
  theme(text = element_text(size=18), legend.position = c(.6, .9), legend.text=element_text(size=16)) 

ggsave(filename = "pwidpairs.png")
