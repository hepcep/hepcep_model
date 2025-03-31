std <- function(x) sd(x)/sqrt(length(x))

new_chronic_incidence_plot <- function(data, start_year, end_year, treatment_start_year){
  #
  # Plot incidence of new chronic infections for provided data
  #
  
  # Calculate the yearly incidence rate per 1000 person-years which is the yearly sum of 
  #   the dt$incidence_daily by the population count
  incidenceYear <- data[Year %in% start_year:end_year, .(incidence=1000*sum(incidence_daily_chronic/(population_ALL-infected_ALL))), 
                        by=list(Year,treatment_enrollment_size, treatment_enrollment_per_PY,
                                treatment_nonadherence, max_num_daa_treatments, run)]
  
  # Calculate the mean and std of yearly incidence rate
  incidenceSummary <- incidenceYear[, list(mean=mean(incidence), sd=sd(incidence), std=std(incidence)), 
                                    by=list(Year,treatment_enrollment_size, treatment_enrollment_per_PY,
                                            treatment_nonadherence, max_num_daa_treatments)]
  
  # Change the enrollment rate and adherence into factors for nicer plotting and..
  #   convert DAA treatment non-adherence to adherence.
  incidenceSummary$Adherence <- factor (100 * (1 - as.numeric(incidenceSummary$treatment_nonadherence)))
  incidenceSummary$treatment_enrollment_per_PY <- factor (100 * as.numeric(incidenceSummary$treatment_enrollment_per_PY))
  incidenceSummary$treatment_enrollment_size <- factor(incidenceSummary$treatment_enrollment_size)
  
  # Create a combined percent - size factor for legend series (doesnt support tab!!)
  incidenceSummary[, combined_levels := paste0(treatment_enrollment_per_PY, "% (", treatment_enrollment_size, ")")]
  
  # Set what factor should be used for the figure legend series color
  incidenceSummary$series_group <- incidenceSummary$treatment_enrollment_per_PY
  
  incidenceSummaryBaseline <- incidenceSummary[treatment_enrollment_size == 0]
  
  # NOTE Haven't needed to recently subset the data...
  #incidenceSummarySubset <- incidenceSummary[treatment_enrollment_per_PY %in% c(2.5,5,7.5,10,20,40,60,80,100) & 
  #                                             Adherence %in% c(90, 80, 70, 60) &
  #                                             
  #                                             # Manually update the DAA treatment max
  #                                             
  #                                             max_num_daa_treatments %in% c(99999)]
  
  # Select the runs with an active DAA enrollment (> 0)
  incidenceSummarySubset <- incidenceSummary[treatment_enrollment_size != 0]
  
  # Relative incidence via the baseline normalization of the no-treatment mean in 2019
  baseline <- incidenceSummaryBaseline[Year==2019]$mean
  #baseline <- 1
  
  # optionally normalize the means relative to the untreated group
  #  ... we also normalize the sd by the baseline mean
  incidenceSummarySubset$mean <- incidenceSummarySubset$mean / baseline # incidenceSummaryBaseline$mean
  incidenceSummarySubset$sd <- incidenceSummarySubset$sd / baseline # / incidenceSummaryBaseline$sd
  incidenceSummarySubset$std <- incidenceSummarySubset$std / baseline # / incidenceSummaryBaseline$std
  
  # 95% CI
  z <- 1.960
  
  legend_title <- "Annual DAA\nEnrollment %  "
#  legend_title <- "Screening %"
  
  p <- ggplot(incidenceSummarySubset) +
    geom_line(aes(x=Year-treatment_start_year+1, y=mean, color=series_group), size=1) +
    geom_point(aes(x=Year-treatment_start_year+1, y=mean, color=series_group), size=2) +
    #  scale_x_continuous(limits = c(2020, endYear), breaks=seq(2020,2050,5)) +
    scale_x_continuous(limits = c(0, endYear-treatment_start_year), breaks=seq(0,endYear-treatment_start_year,5)) +
    scale_y_continuous(limits = c(0, 7)) +
 
    # TODO could pass in a treat start/end for this   
    # Shaded rect for highlighting enrollment period
#    annotate("rect", xmin = 0, xmax = 10, ymin = -Inf, ymax = Inf, alpha = .15) +
    
    geom_ribbon(aes(x=Year-treatment_start_year+1, ymin=mean-z*std, ymax=mean+z*std, fill=series_group),alpha=0.3,colour=NA) +
    
    geom_hline(yintercept=0.1, linetype="dashed", color = "red") +
    geom_hline(yintercept=1.0, linetype="dashed", color = "black") +
    
    #  facet_wrap(vars(Adherence), labeller = label_both) +
    
    labs(y="Relative Incidence", x="Year from DAA enrollment start", color="series_group") + #, title="All Incidence") +
    theme_bw() +
    #  theme_minimal() + 
    theme(text = element_text(size=22), 
          legend.position = c(.7, .7), 
          legend.text=element_text(size=22),
          legend.background = element_rect(fill="white", size=0.5, linetype="solid", colour ="gray")) +
    theme(axis.text=element_text(size=22),axis.title=element_text(size=22)) +
    
    guides(color=guide_legend(title=legend_title),fill=guide_legend(title=legend_title))
  
  # # Black & White symbol Version
  # p <- ggplot(incidenceSummarySubset) + geom_line(aes(x=Year+1, y=mean, group=treatment_enrollment_per_PY), size=1) +
  #   geom_point(aes(x=Year+1, y=mean, shape=treatment_enrollment_per_PY), size=5) +
  #   scale_x_continuous(limits = c(2020, endYear), breaks=c(2020, 2022, 2024, 2026, 2028, 2030)) +
  #   scale_y_continuous(limits = c(0, 6)) +
  #   
  #   geom_ribbon(aes(x=Year+1, ymin=mean-z*std, ymax=mean+z*std, group=treatment_enrollment_per_PY),alpha=0.3,colour=NA) +
  #   
  #   geom_hline(yintercept=0.1, linetype="dashed", color = "black") +
  #   
  #   #  facet_wrap(vars(Adherence), labeller = label_both) +
  #   
  #   scale_shape_manual(values=c(15, 16, 17, 18)) +
  #   
  #   labs(y="Incidence Relative to Year 2020", x="Year", color="treatment_enrollment_per_PY") + #, title="All Incidence") +
  #   theme_bw() +
  #   #  theme_minimal() + 
  #   theme(text = element_text(size=22), 
  #         legend.position = c(.85, .85), 
  #         legend.text=element_text(size=22),
  #         legend.background = element_rect(fill="white", size=0.5, linetype="solid", colour ="gray")) +
  #   theme(axis.text=element_text(size=22),axis.title=element_text(size=22)) +
  #   
  #   guides(shape=guide_legend(title="Enrollment %"),fill=guide_legend(title="Enrollment %"))
  # 
  
  
  return(p)
}

prevalence_plot <- function(data, start_year, end_year, treatment_start_year){
  #
  # Plot incidence of HCV prevalence for provided data
  #
  
  ########## Calculate the Prevalence
  
  # Mean annual prevalence (could also use last day of year) for each run
  prevalenceYear <- data[Year %in% startYear:endYear, .(prevalence=mean(RNApreval_ALL)), 
                         by=list(Year,treatment_enrollment_per_PY, treatment_nonadherence, max_num_daa_treatments, run)]
  
  # Calculate the mean and std of yearly prevalence rate across runs
  prevalenceSummary <- prevalenceYear[, list(mean=mean(prevalence), sd=sd(prevalence), std=std(prevalence)), 
                                      by=list(Year,treatment_enrollment_per_PY, treatment_nonadherence, max_num_daa_treatments)]
  # Change the enrollment rate and adherence into factors for nicer plotting and..
  #   convert DAA treatment non-adherence to adherence.
  prevalenceSummary$Adherence <- factor (100 * (1 - as.numeric(prevalenceSummary$treatment_nonadherence)))
  prevalenceSummary$treatment_enrollment_per_PY <- factor (100 * as.numeric(prevalenceSummary$treatment_enrollment_per_PY))
  
  # Set what factor should be used for the figure legend series color
  prevalenceSummary$series_group <- prevalenceSummary$treatment_enrollment_per_PY
  
  prevalenceSummaryBaseline <- prevalenceSummary[treatment_enrollment_per_PY == 0]
  
  # Select the runs with an active DAA enrollment (> 0)
  prevalenceSummarySubset <- prevalenceSummary[treatment_enrollment_per_PY != 0]
  
  #prevalenceSummarySubset <- prevalenceSummary[treatment_enrollment_per_PY %in% c(2.5,5,7.5,10) & 
  #                                             Adherence %in% c(90, 80, 70, 60) &
  #                                             
  #                                             # Manually update the DAA treatment max
  #                                             
  #                                             max_num_daa_treatments %in% c(99999)]
  
  # Relative prevalence via the baseline normalization of the no-treatment mean in 2019
  #baseline <- prevalenceSummaryBaseline[Year==2019]$mean
  baseline <- 1
  
  # optionally normalize the means relative to the untreated group
  #  ... we also normalize the sd by the baseline mean
  prevalenceSummarySubset$mean <- prevalenceSummarySubset$mean / baseline # prevalenceSummaryBaseline$mean
  prevalenceSummarySubset$sd <- prevalenceSummarySubset$sd / baseline # / prevalenceSummaryBaseline$sd
  prevalenceSummarySubset$std <- prevalenceSummarySubset$std / baseline # / prevalenceSummaryBaseline$std
  
  # 95% CI
  z <- 1.960
  
  legend_title <- "Annual DAA\nEnrollment %  "
#  legend_title <- "Screening %"
  
  # Color Version
  p <- ggplot(prevalenceSummarySubset) + 
    geom_line(aes(x=Year-treatement_start_year+1, y=mean, color=series_group), size=1) +
    geom_point(aes(x=Year-treatement_start_year+1, y=mean, color=series_group), size=2) +
    
    scale_x_continuous(limits = c(0, endYear-treatment_start_year), breaks=seq(0,endYear-treatment_start_year,5)) +
    scale_y_continuous(limits = c(0, 0.4)) +
    
    # TODO could pass in a treat start/end for this   
    # Shaded rect for highlighting enrollment period
        annotate("rect", xmin = 0, xmax = 10, ymin = -Inf, ymax = Inf, alpha = .15) +
    
    geom_ribbon(aes(x=Year-treatement_start_year+1, ymin=mean-z*std, ymax=mean+z*std, fill=series_group),alpha=0.3,colour=NA) +
    
    labs(y="HCV RNA Prevalence", x="Year from DAA enrollment start", color="series_group") + 
    theme_bw() +
    #  theme_minimal() + 
    theme(text = element_text(size=22), 
          legend.position = c(.7, .7), 
          legend.text=element_text(size=22),
          legend.background = element_rect(fill="white", size=0.5, linetype="solid", colour ="gray")) +
    theme(axis.text=element_text(size=22),axis.title=element_text(size=22)) +
    
    guides(color=guide_legend(title=legend_title),fill=guide_legend(title=legend_title))
  
  
  # Black & White symbol Version
  # p <- ggplot(prevalenceSummarySubset) + geom_line(aes(x=Year+1, y=mean, group=treatment_enrollment_per_PY), size=1) +
  #   geom_point(aes(x=Year+1, y=mean, shape=treatment_enrollment_per_PY), size=5) +
  #   #  scale_x_continuous(limits = c(2020, endYear), breaks=c(2020, 2022, 2024, 2026, 2028, 2030)) +
  #   scale_y_continuous(limits = c(0, 0.4)) +
  #   
  #   geom_ribbon(aes(x=Year+1, ymin=mean-z*std, ymax=mean+z*std, group=treatment_enrollment_per_PY),alpha=0.3,colour=NA) +
  #   
  #   scale_shape_manual(values=c(15, 16, 17, 18)) +
  #   
  #   labs(y="prevalence Relative to Year 2020", x="Year", color="treatment_enrollment_per_PY") + #, title="All prevalence") +
  #   theme_bw() +
  #   #  theme_minimal() + 
  #   theme(text = element_text(size=22), 
  #         legend.position = c(.85, .85), 
  #         legend.text=element_text(size=22),
  #         legend.background = element_rect(fill="white", size=0.5, linetype="solid", colour ="gray")) +
  #   theme(axis.text=element_text(size=22),axis.title=element_text(size=22)) +
  #   
  #   guides(shape=guide_legend(title="Enrollment %"),fill=guide_legend(title="Enrollment %"))
  
  
  return(p)
}