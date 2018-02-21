#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/initialize_random.h"
#include "repast_hpc/io.h"
#include "repast_hpc/logger.h"

#include "HCModel.h"

using namespace repast;

void usage() {
	std::cerr << "usage: X  model_properties_file [log_config_file]" << std::endl;
	std::cerr << "\tmodel_properties_file: the path to the model properties file" << std::endl;
}

void runModel(std::string propsFile, int argc, char** argv) {
	boost::mpi::communicator comm;
	if (comm.rank() == 0) {
		std::string time;
		repast::timestamp(time);
		std::cout << "Start Time: " << time << std::endl;
	}

	Properties props(propsFile, argc, argv);
	initializeRandom(props);
	// chi_sim AbstractModel will fill the parameters from props
	// TODO change 0 to appropriate value if persons move between
	// processes
	hepcep::HCModel model(props, 0);

	if (comm.rank() == 0) {
	  std::string time;
	  repast::timestamp(time);
	  std::cout << "Schedule Start Time: " << time << std::endl;
	}

	RepastProcess::instance()->getScheduleRunner().run();

	if (comm.rank() == 0) {
		std::string time;
		repast::timestamp(time);
		std::cout << "End Time: " << time << std::endl;
	}
}

int main(int argc, char **argv) {
	boost::mpi::environment env(argc, argv);
	std::string props;

	boost::mpi::communicator world;

	// if there aren't enough arguments, warn the user and exit
	if (argc < 2) {
		usage();
		return -1;
	}

	// model props file
	props = argv[1];
	if (props.size() > 0) {
	    std::string log_config = "";
	    if (argc > 2) {
	        log_config = argv[2];
	    }
	    try {
			RepastProcess::init(log_config, &world);
			Log4CL::instance()->get_logger("hepcep").log(INFO, "Starting Model");
			runModel(props, argc, argv);
			Log4CL::instance()->get_logger("hepcep").log(INFO, "Ending Model");
			RepastProcess::instance()->done();

		} catch (std::exception& ex) {
			std::cerr << "Error while running the model: " << ex.what() << std::endl;
			throw ex;
		}
	} else {
		if (world.rank() == 0)
			usage();
	}


	return 0;
}
