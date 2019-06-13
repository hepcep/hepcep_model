
library(data.table)
library(ggplot2)
library(scales)
library(scatterpie)

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
  
  dt <- data.frame(do.call(rbind, strsplit(objstrings,',')), stringsAsFactors = FALSE) 
  
  names(dt) <- c('Treatment_Count','Incidence_Rate')
  dt$Treatment_Count <- as.numeric(dt$Treatment_Count)
  dt$Incidence_Rate <- as.numeric(dt$Incidence_Rate)
  dt$Generation <- gen

  tableList[[gen]] <- dt
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
      vals <- c(vals, 100 * as.numeric(name_val[2]))
    }
    param_list[[i]] <- vals   # transpose the vals
    i <- i+1
  }
  
  dt <- as.data.table(do.call(rbind, param_list))
  names(dt) <- names
  dt$Generation <- gen
  
  tableList[[gen]] <- dt
}
param_table <- rbindlist(tableList)  # Stack the list of tables into a single DT
param_table$ID <- seq(1,nrow(param_table),1)   # simple  ID
setkey(param_table, ID)
tableList <- NULL                           # clear mem

# Create the full table of pareto optimal solutions and parameters
pareto_table <- merge(obj_table,param_table, by=c("ID","Generation"))

scaleFUN <- function(x) sprintf("%.2f", x)

# Plot all generations pareto front in a grid of plots
plotlist = list()
for (i in 1:20){
  p <- ggplot(pareto_table[Generation==i]) +
    geom_point(aes(x=Treatment_Count, y=Incidence_Rate), size=2) +
    scale_y_continuous(labels=scaleFUN, limits=c(0,13)) +
    scale_x_continuous(limits=c(3000,12500)) +
    labs(y="Incidence Rate", x="Treatment Count", title=paste0("Generation ",i)) +
    theme_minimal()

  plotlist[[i]] = p   
}
gridplot <- grid.arrange(grobs=plotlist,ncol=4)

d <- pareto_table[Generation==1]
d$Treatment_Count <- d$Treatment_Count / 1000
d$radius <- sqrt(0.05*d$`treatment_enrollment_per_PY `)
p <- ggplot() + 
     geom_scatterpie(aes(x=Treatment_Count, y=Incidence_Rate, group=ID, r=radius), 
                     data = d,
                 
                     cols=names[2:6]) + coord_equal() +
  scale_y_continuous(limits=c(-2,12)) +
  scale_x_continuous(limits=c(0,14)) +
  theme_minimal() +
  theme(text = element_text(size=10), 
        legend.position = c(.25, .25), 
        legend.text=element_text(size=10),
        legend.background = element_rect(fill="white", size=0.5, linetype="solid", colour ="gray")) +
  labs(y="Incidence (per 1000 person-years)", x="Treatment Count x10^4", title="Generation 1 Pareto Optimal Solutions")
#  geom_scatterpie_legend(d$radius, x=1, y=1)
show(p)
  