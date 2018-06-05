require(xml2)

files <- list.files(path=".", pattern="*.xml")

numruns <-9999
x <- ""
i <- 0
for (f in files){
  print(f)
  i = i + 1
  b_params <- read_xml(f)

  b_params_children <- xml_find_all(b_params,".//parameter")

  x <- paste0(x,"run.number=",i,"\t")
  x <- paste0(x,"random.seed=",i)
  
  for (node in b_params_children){
    name <- xml_attr(node,"name")
    value <- xml_attr(node,"value")
    
    if (name == "run_length" || name == "status_report_frequency" ){
#      name <- "stop.at"
#      value <- 2000 # TEST TEST TEST
      
      # don't include these since we set via model.props
    }
    else{
      x <- paste0(x,"\t",name,"=",value)  
    }
  }
  x <- paste0(x,"\n")
  
  if (i >= numruns) break
}
write(x, file="upf_plos_lhs.txt")
