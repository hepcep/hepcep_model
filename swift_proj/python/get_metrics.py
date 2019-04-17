import os, sys, glob, statistics

def get_model_outputs(instance_dir):
    """
    @return count value from fname or -2, if file doesn't exist, or
    -1 if run terminated prematurely.
    """
    
    # Open the model event log and count event occurrences
    test_count = '-2'
    treatment_count = '-2'

    fname = '{}/events.csv'.format(instance_dir)
    if os.path.exists(fname):
        with open(fname) as f_in:
            test_c = 0
            treatment_c = 0
            
            # Each log line is only one type of model event
            for line in f_in:
                if ('HCVRNA_TEST' in line):
                    test_c += 1
                if ('STARTED_TREATMENT' in line):
                    treatment_c += 1
           

            test_count = str(test_c)
            treatment_count = str(treatment_c)
    
    # Open the model stats file and return relevant stats
    fname = '{}/stats.csv'.format(instance_dir)
    
    # The yearly incidence rate per 1000-PY is calculated as:
    #  incidence = 1000*sum(incidence_daily/(population_ALL-infected_ALL))
    yearly_incidence = '-2'
    
    # Columns in the model stats.csv output
    incidence_col = 175      # col number in excel for daily 
    population_col = 138
    infected_col = 57
    
    if os.path.exists(fname):
        with open(fname) as f_in:
            # read the last 365 lines for yearly totals
            lines = f_in.readlines()[-365:]
                        
            inc_total = 0
            for line in lines:
                vals = line.strip().split(",")
                
                incidence_daily = int(vals[incidence_col-1])
                population_ALL = int(vals[population_col-1])
                infected_ALL = int(vals[infected_col-1])
                
                inc_total += incidence_daily / (population_ALL - infected_ALL)
            
            inc_total *= 1000            
            yearly_incidence = str(inc_total)
            
    return test_count + ',' + treatment_count + ',' + yearly_incidence

# Combines the scores input string which is a colon-delmitted set of comma-separated
# stochastic model output metrics that should be merged, eg. return the mean value.
#
#  For example, scores = "2627,1467,684:2519,1437,672"

def combine_scores(scores):
    all_scores = scores.split(":")    # colon-separated all run scores
    
    test_count = []
    treatment_count = []
    yearly_incidence = []
    
    if len(all_scores) == 1:    # if only run stochastic model run just return
        return all_scores
    
    for run, rs in enumerate(all_scores):
        run_scores = rs.split(",")  # comma-separated scores for single run
        
        print(run_scores)
        
        test_count.append(float(run_scores[0]))
        treatment_count.append(float(run_scores[1]))
        yearly_incidence.append(float(run_scores[2]))
            
    combined_scores = ""    
    
    # Omit test counts for now
#    combined_scores += str(statistics.mean(test_count))
#    combined_scores += ","
    combined_scores += str(statistics.mean(treatment_count))
    combined_scores += ","
    combined_scores += str(statistics.mean(yearly_incidence))  
            
    return combined_scores
    

