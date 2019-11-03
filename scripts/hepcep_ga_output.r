
library(data.table)
library(ggplot2)
library(scales)
library(scatterpie)
library(ggrepel)
library(gridExtra)
library(grid)

baseline_incidence <- 12.914 # Year 2019 incidence baseline value before treatment begins

processFile = function(filepath) {
  con = file(filepath, "r")
  results <- c()
  while ( TRUE ) {
    line = readLines(con, n = 1)
    if ( length(line) == 0 ) {
      break
    }
    results <- c(results,line)
  }
  
  close(con)
  
  return(results)
}

pareto_obj_lines <- processFile("pareto.txt")            #pareto front Hall of Fame objective values
pareto_param_lines <- processFile("pareto_params.txt")   #pareto front Hall of Fame parameter values

# Create a data table with the pareto objective values
tableList <- list()
for (line in pareto_obj_lines){

  # First char is generation number
  linesplit <- unlist(strsplit(line, '\t'))
  gen <- as.numeric(linesplit[1])
  
  # Second split is the string of the objectives for each individual
  objstrings <- unlist(strsplit(linesplit[2], ';'))
  objstrings <- gsub("\\(|\\)","",objstrings)    # remove parens
  
  dtp <- data.frame(do.call(rbind, strsplit(objstrings,',')), stringsAsFactors = FALSE) 
  
  names(dtp) <- c('Treatment_Count','Incidence_Rate')
  dtp$Treatment_Count <- as.numeric(dtp$Treatment_Count)
  dtp$Incidence_Rate <- as.numeric(dtp$Incidence_Rate)
  dtp$Generation <- gen

  tableList[[gen]] <- dtp
}
obj_table <- rbindlist(tableList)  # Stack the list of tables into a single DT
obj_table$ID <- seq(1,nrow(obj_table),1)   # simple  ID
setkey(obj_table, ID)
tableList <- NULL                     # clear mem

# Create a data table with the pareto parameter values
tableList <- list()

for (line in pareto_param_lines){
  
  # Remove the gen id from the first param string
  i <- regexpr('\t', line)[1]    # index of first tab
  gen <- as.numeric(substr(line, 0, i-1))
    
  line <- substr(line, i+1, nchar(line))
  
  # Second split is the string of the param pairs for each individual
  paramstrings <- unlist(strsplit(line, ';'))
  
  param_list <- list()
  i <- 1
  for (ind_param in paramstrings){
    # parameters are tab delim
    parampairs <- unlist(strsplit(ind_param, '\t'))
    
    names <- c()     # parameter names
    vals <- c()      # parameter values
    for (param in parampairs){
      # Each param name/value pair is '=' delim
      name_val <- unlist(strsplit(param, '='))
      
      names <- c(names,name_val[1])
      vals <- c(vals, 100 * as.numeric(name_val[2]))   # scale vals to percent
    }
    param_list[[i]] <- vals   # transpose the vals
    i <- i+1
  }
  
  dtp <- as.data.table(do.call(rbind, param_list))
  names(dtp) <- names
  dtp$Generation <- gen
  
  tableList[[gen]] <- dtp
}
param_table <- rbindlist(tableList)  # Stack the list of tables into a single DT
param_table$ID <- seq(1,nrow(param_table),1)   # simple  ID
setkey(param_table, ID)
tableList <- NULL                           # clear mem

# Create the full table of pareto optimal solutions and parameters
pareto_table <- merge(obj_table,param_table, by=c("ID","Generation"))

# The incidence relative to year 2019 before treatment starts
pareto_table$Relative_Incidence <- pareto_table$Incidence_Rate / baseline_incidence

# Strip out verbose column text
names(pareto_table) <- gsub("treatment_enrollment_probability_","",names(pareto_table))
names <- gsub("treatment_enrollment_probability_","",names)
scaleFUN <- function(x) sprintf("%.2f", x)
scalePercent <- function(x) sprintf("%.0f %%", x)
scale0 <- function(x) sprintf("%.0f", x)
scale1 <- function(x) sprintf("%.1f", x)

# Plot all generations pareto front in a grid of plots
plotlist1 = list()
for (i in 1:20){
  d2 <- pareto_table[Generation == i]
  d2$Treatment_Count <- d2$Treatment_Count / 10000
  
  p <- ggplot(d2) +
    geom_point(aes(x=Treatment_Count, y=Relative_Incidence), size=1.0, color='black') +
    scale_y_continuous( limits=c(0,1), breaks=c(0,0.25,0.5,0.75,1)) +
    scale_x_continuous(limits=c(0.2,1.3), breaks=c(0.2,0.4,0.6,0.8,1.0,1.2)) +
    labs(y="Rel. Inc. Rate", x=expression("Treat. Count x 10"^"4")) +
#    labs(y="", x="", title=paste0("Generation ",i)) +
    theme_minimal() +
#    theme(panel.grid.major = element_line(colour = "black",size=0.5)) +
    theme(panel.border = element_rect(colour = "black", fill=NA, size=0.5)) + 
    theme(panel.grid.minor = element_blank()) +
    theme(panel.grid.major = element_blank()) +
    theme(axis.ticks = element_line(size = 0.5)) + 
    theme(axis.text=element_text(size=8),axis.title=element_text(size=8)) +
    annotate("text", x = 0.5, y = 0.2, label = paste0("Generation ", i), size=4)

  plotlist1[[i]] = p   
}
gridplot <- grid.arrange(grobs=plotlist1,ncol=4)
ggsave("Pareto grid.png", plot=gridplot, width=10, height=8)

# Color-blind palette with grey:
cbPalette <- c("#999999", "#E69F00", "#56B4E9", "#009E73", "#F0E442", "#0072B2", "#D55E00", "#CC79A7")

# Plot the individual generation pareto plots as pie charts with each treatment percent.
for (i in 1:20){
  
  # Plotting issues might be due to plots having references to same data d...
  # Perhaps try using a list of d
  
d <- pareto_table[Generation == i]
  d$Treatment_Count <- d$Treatment_Count / 10000
  d$radius <- sqrt(0.05*d$`treatment_enrollment_per_PY `)
  
  label_indices <- seq(1,nrow(d),3)  # Subset of point indices for callout labels
  
  p <- ggplot(d, aes(x=Treatment_Count, y=Relative_Incidence)) +   # needed for text_repel 
     geom_scatterpie(aes(x=Treatment_Count, y=Relative_Incidence, group=ID, r=0.03), 
                     data = d,
#                     show.legend = FALSE,
                     cols=names[2:6]) + coord_equal() +
    scale_y_continuous( limits=c(-0.1,1.1), breaks=c(0,0.25,0.5,0.75,1)) +
    scale_x_continuous(limits=c(0.2,1.3), breaks=c(0.2,0.4,0.6,0.8,1.0,1.2)) +
   
    scale_colour_manual(values=cbPalette) + 
    scale_fill_manual(values=cbPalette) + 
    
    theme_minimal() +
    theme(axis.text=element_text(size=20),axis.title=element_text(size=20)) +
    theme(panel.border = element_rect(colour = "black", fill=NA, size=0.5)) + 
    theme(panel.grid.minor =   element_blank()) +
    
   theme(text = element_text(size=20),
         legend.position = c(.25, .25), 
         legend.text=element_text(size=20),
         legend.background = element_rect(fill="white", size=0.5, linetype="solid", colour ="gray")) +
    labs(y="Relative Incidence (per 1000 person-years)", x=expression("Treatment Count x 10"^"4")) +
    guides(fill=guide_legend(title="Enrollment")) +

    annotate("text", x = 1.1, y = 1.1, label = paste0("Generation ", i), size=8) +
    
    geom_hline(yintercept=0.1, linetype="dashed", color="red", size=1, alpha=0.5) +
    
  # Add callout labels on selected points (modulo) for the Total enrollment  
    geom_text_repel(
#     data = d[ID %% 3 == 0],
#     aes(label = scale0(d[ID %% 3 == 0]$treatment_enrollment_per_PY)), 
#     data = d,
      aes(label = scale1(d$treatment_enrollment_per_PY)), 
      size = 5,
#     box.padding = unit(1.50, "lines")
      point.padding = unit(1.50, "lines")
  )

  ggsave(paste0("Pareto scatterpie grid ", i,".png"), plot=p, width=10, height=8)
 
}

  