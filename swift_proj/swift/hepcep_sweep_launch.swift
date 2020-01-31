import io;
import sys;
import files;
import R;
import python;
import string;
import launch;
import unix;

// import hepcep_model;

string emews_root = getenv("EMEWS_PROJECT_ROOT");
string turbine_output = getenv("TURBINE_OUTPUT");

string config_file = argv("config_file");

string cnep_plus_file = "cnep_plus.file = %s/../data/cnep_plus_all_2018.02.13.csv" % emews_root;
string cnep_plus_early_file = "cnep_plus_early.file = %s/../data/cnep_plus_early_2018.02.13.csv" % emews_root;
string zones_file = "zones.file = %s/../data/zones.csv" % emews_root;
string zones_distance_file = "zones.distance.file = %s/../data/zones_distance.csv" % emews_root;

string model = "%s/../Release/hepcep_model-0.0" % emews_root;

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
    string instance_dir = "%s/run_%d" % (turbine_output, i);
    string output = "output.directory = %s" % instance_dir;
    string run_number = get_run_number(pl, i);

    string defaults = "%s%s\t%s\t%s\t%s\t%s\t%s\t%s" % (run_number, stats_output, events_output, output,
      cnep_plus_file, cnep_plus_early_file, zones_file, zones_distance_file);
    string line = "\"%s\t%s\"" % (defaults, pl);
    string args[] = ["-props", config_file, "-params", line];
    string envs[] = ["swift_chdir=%s" % instance_dir, "swift_launcher=foo"];
    mkdir(instance_dir) =>
    @par=1 launch_envs(model, args, envs) =>
    zs[i] = line;
		// zs[i] = @par=1 hepcep_model_run(config_file, line);
  }

	//results_file = "%s/results.csv" % (turbine_output);
  //file out<results_file> = write(join(zs, "\n") + "\n");
	v = propagate(size(zs));
}

main {
	run_model(); // =>
	//string args[] = ["", ""];
  //@par=1 launch(model, args);
}