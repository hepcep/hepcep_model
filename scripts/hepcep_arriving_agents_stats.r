#
# Analysis of hepcep arriving agets properties
#
# Eric Tatara
#

library(data.table)
library(ggplot2)

# The agent stats file contains properties for each arriving agent during a model
#   run with each row representing a unique agent.
fileName <- "../output/arriving_agents_3.csv"
dt <- fread(fileName)