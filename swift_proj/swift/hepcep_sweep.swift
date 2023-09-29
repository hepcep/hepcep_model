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
string data_dir = argv("data_dir");

(string run_param) get_run_number(string params, int line_num) {
  if (find(params, "run.number", 0, -1) == -1) {
    run_param = "run.number=%d\t" % line_num;
  } else {
    run_param = "";
  }
}

(void v) run_model()
{
	string param_file = argv("f"); // e.g. -f="model_params.txt"
	string param_lines[] = file_lines(input(param_file));

	string zs[];
  foreach pl,i in param_lines {
    string stats_output = "stats.output.file = stats.csv";
    string events_output = "events.output.file = events.csv";
    string output = "output.directory = %s/run_%d" % (turbine_output, i);
    string input = "data.dir = %s" % data_dir;
    string run_number = get_run_number(pl, i);

    string defaults = "%s%s\t%s\t%s\t%s" % (run_number, stats_output, events_output, output, input);
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
