#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/initialize_random.h"
#include "repast_hpc/io.h"
#include "repast_hpc/logger.h"

#include "HCModel.h"
#include "run.h"

using namespace repast;

const std::string config_arg = "-config";
const std::string params_arg = "-params";
const std::string props_arg = "-props";

struct Args {
    std::string config, param_line, props;
};

void usage() {
    std::cerr << "usage: X -props <default model parameters file> [-params <tab seperated params>] [-config <config file>]" << std::endl;
}

void runModel(const std::string& propsFile, const std::string& param_line) {
    boost::mpi::communicator comm;
    if (comm.rank() == 0) {
        std::string time;
        repast::timestamp(time);
        std::cout << "Start Time: " << time << std::endl;
    }

    Properties props(propsFile);
    hepcep::parse_parameters(props, param_line);
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

bool parse_args(int argc, char**argv, Args& args) {
    args.props = args.config = args.param_line = "";
    if (argc % 2 == 1) {
        // check args
        for (int i = 1; i < argc; i += 2) {
            std::string arg = argv[i];
            if (arg == props_arg) {
                args.props = argv[i + 1];
            } else if (arg == config_arg) {
                args.config = argv[i + 1];
            } else if (arg == params_arg) {
                args.param_line = argv[i + 1];
            } else {
                return false;
            }
        }

        if (args.props == "") {
            return false;
        }
        return true;
    }

    return false;
}

int main(int argc, char **argv) {
    boost::mpi::environment env(argc, argv);
    std::string props;

    boost::mpi::communicator world;

    Args args;
    if (parse_args(argc, argv, args)) {
        try {
            RepastProcess::init(args.config, &world);
            Log4CL::instance()->get_logger("hepcep").log(INFO, "Starting Model");
            runModel(args.props, args.param_line);
            Log4CL::instance()->get_logger("hepcep").log(INFO, "Ending Model");
            RepastProcess::instance()->done();

        } catch (std::exception& ex) {
            std::cerr << "Error while running the model: " << ex.what() << std::endl;
            exit(1);
        }
    } else {
        if (world.rank() == 0) {
            std::cout << argc << std::endl;
            for (int i = 0; i < argc; i++) {
                std::cout << argv[i] << " ";
            }
            std::cout << std::endl;
            usage();
        }
    }

    return 0;
}
