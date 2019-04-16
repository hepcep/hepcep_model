import files;
import string;
import sys;
import io;
import stats;
import python;
import math;
import location;
import assert;

import EQPy;

import hepcep_model;

string emews_root = getenv("EMEWS_PROJECT_ROOT");
string turbine_output = getenv("TURBINE_OUTPUT");

string config_file = argv("config_file");

string resident_work_ranks = getenv("RESIDENT_WORK_RANKS");
string r_ranks[] = split(resident_work_ranks,",");

string cnep_plus_file = "cnep_plus.file = %s/../data/cnep_plus_all_2018.02.13.csv" % emews_root;
string cnep_plus_early_file = "cnep_plus_early.file = %s/../data/cnep_plus_early_2018.02.13.csv" % emews_root;
string zones_file = "zones.file = %s/../data/zones.csv" % emews_root;
string zones_distance_file = "zones.distance.file = %s/../data/zones_distance.csv" % emews_root;

string ga_params = argv("ga_params");
float mut_prob = string2float(argv("mutation_prob", "0.2"));

string result_template =
"""
import get_metrics

scores = '%s'

combined_scores = get_metrics.combine_scores(scores)
""";

string model_out_template =
"""
import get_metrics

instance_dir = '%s'

model_outs = get_metrics.get_model_outputs(instance_dir)
""";

(string zs) run(string params, string instance)
{
    string stats_output = "stats.output.file = stats.csv";
    string events_output = "events.output.file = events.csv";
    string output = "output.directory = %s" % (instance);
    string run_number = "";
	
	string defaults = "%s%s\t%s\t%s\t%s\t%s\t%s\t%s" % (run_number, stats_output, events_output, output,
      cnep_plus_file, cnep_plus_early_file, zones_file, zones_distance_file);
	  
	string line = "%s\t%s" % (defaults, params);
	printf(line);
	
	zs = @par=1 hepcep_model_run(config_file, line);
}

// Returns a comma-separated event count from a single model instance
(string model_outs) read_model_out(string instance) {
  code = model_out_template % instance;
  model_outs = python_persist(code, "model_outs");

  printf("model outs: " + model_outs);
}

// The returned score is a comma-separated set of mean objective values for a single
//   GA individual.  The values are averaged over the set of trials.  
(string score) obj(string params, int ga_iter, int param_iter, int trials) {
    string results[];  
	
	// i is used as random seed in input
    foreach i in [0:trials-1:1] {
      string instance = "%s/instance_%i_%i_%i/" % (turbine_output, ga_iter, param_iter, i+1);
      
      // Add the model run random seed parameter based on the trial count
      string seedparam = "random.seed = %i" % i;
      string param_line = "%s\t%s" % (params, seedparam);
      
      make_dir(instance) => {    // create instance dir
        // run model instance
        run(param_line, instance) =>
        results[i] = read_model_out(instance);
      }
    }

	// Combine the results for each random trial in a colon-separated list
    all_scores = string_join(results, ":");
    
    printf("scores: " + all_scores);

    string code = result_template % all_scores;
    string combined_scores = python_persist(code, "combined_scores");
    printf("combined scores: " + combined_scores);
  
    score = combined_scores;  
//    score = "1.5842";   //testing - changed to combined scores
}

(void v) loop (location ME, int trials) {
    for (boolean b = true, int i = 1;
       b;
       b=c, i = i + 1)
  {
    // gets the model parameters from the python algorithm
    string params =  EQPy_get(ME);
    boolean c;
	
	// Edit the finished flag, if necessary.
    // when the python algorithm is finished it should
    // pass "DONE" into the queue, and then the
    // final set of parameters. If your python algorithm
    // passes something else then change "DONE" to that
    if (params == "DONE")
    {
        string finals =  EQPy_get(ME);
		
        // TODO if appropriate
        // split finals string and join with "\\n"
        // e.g. finals is a ";" separated string and we want each
        // element on its own line:
        // multi_line_finals = join(split(finals, ";"), "\\n");
        
		string fname = "%s/final_result" % (turbine_output);
        file results_file <fname> = write(finals) =>
        printf("Writing final result to %s", fname) =>
        // printf("Results: %s", finals) =>
        v = make_void() =>
        c = false;
    }
	 else if (params == "EQPY_ABORT")
     {
        printf("EQPy Aborted");
        string why = EQPy_get(ME);
        // TODO handle the abort if necessary
        // e.g. write intermediate results ...
        printf("%s", why) =>
        v = propagate() =>
        c = false;
    }
    else
    {
        string param_array[] = split(params, ";");
        string results[];
        foreach p, j in param_array
        {
            results[j] = obj(p, i, j, trials);
        }

        // The returned multi-objective is a semicolon-separated list for each
        // individual, and each individual is a comma-separated list of objective.
        
        string res = join(results, ";");
        printf("passing result: %s", res);
        
        EQPy_put(ME, res) => c = true;
    }
  }
}

(void o) start (int ME_rank, int num_iter, int pop_size, int trials, int seed) {
  location deap_loc = locationFromRank(ME_rank);
  
  algo_params = "%d,%d,%d,%f,'%s'" %  (num_iter, pop_size, seed, mut_prob, ga_params);
	
  printf("start: %s", algo_params);	
	
    EQPy_init_package(deap_loc,"deap_ga") =>
    EQPy_get(deap_loc) =>
    EQPy_put(deap_loc, algo_params) =>
      loop(deap_loc, trials) => {
        EQPy_stop(deap_loc);
        o = propagate();
    }
}

// deletes the specified directory
app (void o) rm_dir(string dirname) {
  "rm" "-rf" dirname;
}

// call this to create any required directories
app (void o) make_dir(string dirname) {
  "mkdir" "-p" dirname;
}

main() {
  int random_seed = toint(argv("seed", "0"));
  int num_iter = toint(argv("ni","100")); // -ni=100
  int num_trials = toint(argv("nv", "5"));
  int num_pop = toint(argv("np","100")); // -np=100;

  printf("NI: %i # num_iter", num_iter);
  printf("NV: %i # num_variations", num_trials);
  printf("NP: %i # num_pop", num_pop);
  printf("MUTPB: %f # mut_prob", mut_prob);

  // PYTHONPATH needs to be set for python code to be run
  assert(strlen(getenv("PYTHONPATH")) > 0, "Set PYTHONPATH!");
  assert(strlen(emews_root) > 0, "Set EMEWS_PROJECT_ROOT!");

  int rank = string2int(r_ranks[0]);
  start(rank, num_iter, num_pop, num_trials, random_seed); // =>
//	@par=1 hepcep_model_run("", "");
}