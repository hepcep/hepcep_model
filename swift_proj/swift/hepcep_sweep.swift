import io;
import sys;
import files;
import R;
import python;
import string;

import hepcep_model;

string emews_root = getenv("EMEWS_PROJECT_ROOT");
string turbine_output = getenv("TURBINE_OUTPUT");

string config_file = argv("config_file");

string cnep_plus_file = "cnep_plus.file = %s/../data/cnep_plus_all_2018.02.13.csv" % emews_root;
string cnep_plus_early_file = "cnep_plus_early.file = %s/../data/cnep_plus_early_2018.02.13.csv" % emews_root;
string zones_file = "zones.file = %s/../data/zones.csv" % emews_root;
string zones_distance_file = "zones.distance.file = %s/../data/zones_distance.csv" % emews_root;

(void v) run_model()
{
	string param_file = argv("f"); // e.g. -f="model_params.txt"
	string param_lines[] = file_lines(input(param_file));

	string zs[];
  foreach pl,i in param_lines {
    string stats_output = "stats.output.file = stats.csv";
    string events_output = "events.output.file = events.csv";
    string output = "output.directory = %s/run_%d" % (turbine_output, i);

    string defaults = "%s\t%s\t%s\t%s\t%s\t%s\t%s" % (stats_output, events_output, output,
      cnep_plus_file, cnep_plus_early_file, zones_file, zones_distance_file);
    string line = "%s\t%s" % (defaults, pl);
		printf(line);
		zs[i] = @par=1 hepcep_model_run(config_file, line);
  }

	//results_file = "%s/results.csv" % (turbine_output);
  //file out<results_file> = write(join(zs, "\n") + "\n");
	v = propagate(size(zs));
}

main {
	run_model() =>
	@par=1 hepcep_model_run("", "");
}
