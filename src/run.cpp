#include "omp.h"

#include "repast_hpc/io.h"
#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/initialize_random.h"

#include "boost/tokenizer.hpp"
#include "boost/algorithm/string.hpp"

#include "chi_sim/Parameters.h"

#include "HCModel.h"
#include "run.h"

namespace hepcep {

const std::string OUTPUT_DIRECTORY = "output.directory";

void parse_parameters(int rank, repast::Properties& props, const std::string& parameters) {
    boost::char_separator<char> tab_sep("\t");
    boost::tokenizer<boost::char_separator<char> > tab_tok(parameters, tab_sep);

    for (auto item : tab_tok) {

        size_t pos = item.find_first_of("=");
        if (pos == std::string::npos) {
            throw invalid_argument("Invalid parameter: " + item);
        }

        string key(item.substr(0, pos));
        boost::trim(key);
        if (key.length() == 0) {
            throw invalid_argument("Invalid parameter: " + item);
        }

        string val(item.substr(pos + 1, item.length()));
        boost::trim(val);
        if (val.length() == 0) {
            throw invalid_argument("Invalid parameter: " + item);
        }

        props.putProperty(key, val);
    }
}


void run_model(int rank, repast::Properties& props) {
    boost::mpi::communicator comm;
    if (comm.rank() == 0) {
        std::string time;
        repast::timestamp(time);
        std::cout << "Start Time: " << time << std::endl;
    }

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


    props.writeToPropsFile(props.getProperty(OUTPUT_DIRECTORY) + "/parameters.txt", "");

    repast::RepastProcess::instance()->getScheduleRunner().run();

    if (comm.rank() == 0) {
        std::string time;
        repast::timestamp(time);
        std::cout << "End Time: " << time << std::endl;
    }
}

std::string hepcep_model_run(MPI_Comm comm, const std::string& props_file, const std::string& parameters) {
    char arg0[] = "main";
    char* argv[] = { &arg0[0], nullptr };
    int argc = (int) (sizeof(argv) / sizeof(argv[0])) - 1;
    // need the tmp in here because environment takes a reference
    char** tmp = &argv[0];
    boost::mpi::environment env(argc, tmp);

    std::string ret;
    if (parameters.size() == 0) {
        ret = "";
    } else {
        boost::mpi::communicator boost_comm(comm, boost::mpi::comm_attach);

        repast::Properties props(props_file);
        parse_parameters(boost_comm.rank(), props, parameters);

        //std::cout << rank << ": " << props.getProperty("incubation.duration.max") << std::endl;
        repast::RepastProcess::init("", &boost_comm);
        run_model(boost_comm.rank(), props);
        repast::RepastProcess::instance()->done();
        ret="run done";
    }
    return ret;
}

}
