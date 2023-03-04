/*
 * @file ViralKintecsLoader.cpp
 * Loader for viral kinetics files
 *
 * @author Eric Tatara
 */
#include <exception>

#include "ViralKinetics.h"
#include "SVReader.h"

namespace hepcep {

ViralKinetics* ViralKinetics::instance_ = nullptr;

void ViralKinetics::init(const std::string& data_dir){

	if (ViralKinetics::instance_ != nullptr){
		delete ViralKinetics::instance_;
	}

	instance_ = new ViralKinetics(data_dir);
}

/**
 * Load a map viral load to transmission probability
 */
void loadTransmissionProbabilities(const std::string& filename, std::map<double, 
		double> & transmit_prob_map) {

	SVReader reader(filename, ',');
	std::vector<std::string> line;

	// Header
	reader.next(line);

	// Each row represents the mapping between the viral load and transmission probability
	while (reader.next(line)) {
		double viral_load = std::stod(line[0]);
		double transmit_prob = std::stod(line[1]);

		transmit_prob_map[viral_load] = transmit_prob;
	}
}

/**
 * Loads a collection of viral load profiles (time series)
 */
 void loadViralLoadSeries(const std::string& filename, std::unordered_map<unsigned int, 
 		std::vector<double>> & viral_loads) {

	SVReader reader(filename, ',');
	std::vector<std::string> line;

	// No header in the input file!

	// Each row represents a unique time series of viral load.  The first element is the
	//   seried ID number.
	int line_number = 1;
	while (reader.next(line)) {
		unsigned int series_id = std::stoul(line[0]);
		
		int rowlen = line.size();
 		std::vector<double> load_data;
 		for (int i=1; i<rowlen; i++){            // start i=1
			// Catch exceptions with malformed data values in the input file
			try {
				load_data.push_back(std::stod(line[i]));
			}
			catch (std::exception& e){
				std::cout << "Error reading double value from file: " << filename 
					<< " at line " << line_number << ", position " << i << std::endl;
				
				std::cout << e.what() << std::endl;
			}
 		}
 		viral_loads[series_id] = load_data;
		line_number++;
	}
}

ViralKinetics::ViralKinetics(const std::string& data_dir) :
	transmit_prob_map(),
	viral_loads_acute_infection_clearance(),
	viral_loads_acute_infection_incomplete(),
	viral_loads_acute_infection_persistence(),
	viral_loads_reinfectin_chronic(),
	viral_loads_reinfection_low_titer_clearance(),
	viral_loads_reinfection_high_titer_clearance(),
	viral_loads_treated() {
	
	std::string transmit_prob_file = data_dir + "/" + chi_sim::Parameters::instance()->getStringParameter(VK_TRANSMIT_PROB_FILE);
    // std::cout << "VK transmit prob file: " << transmit_prob_file << std::endl;
    
	loadTransmissionProbabilities(transmit_prob_file, transmit_prob_map);

	std::string file = data_dir + "/" + chi_sim::Parameters::instance()->getStringParameter(VK_ACUTE_INFECT_CLEAR_FILE);
    // std::cout << "VK acute infection clearance file: " << file << std::endl;
	loadViralLoadSeries(file, viral_loads_acute_infection_clearance);

	file = data_dir + "/" + chi_sim::Parameters::instance()->getStringParameter(VK_ACUTE_INFECT_INCOMP_FILE);
    // std::cout << "VK acute infection incomplete file: " << file << std::endl;
	loadViralLoadSeries(file, viral_loads_acute_infection_incomplete);

	file = data_dir + "/" + chi_sim::Parameters::instance()->getStringParameter(VK_ACUTE_INFECT_PERSIST_FILE);
    // std::cout << "VK acute infection persistence file: " << file << std::endl;
	loadViralLoadSeries(file, viral_loads_acute_infection_persistence);

	file = data_dir + "/" + chi_sim::Parameters::instance()->getStringParameter(VK_REINFECT_CHRONIC_FILE);
    // std::cout << "VK re-infection chronic file: " << file << std::endl;
	loadViralLoadSeries(file, viral_loads_reinfectin_chronic);

	file = data_dir + "/" + chi_sim::Parameters::instance()->getStringParameter(VK_REINFECT_HIT_CLEAR_FILE);
    // std::cout << "VK re-infection high titer clearance file: " << file << std::endl;
	loadViralLoadSeries(file, viral_loads_reinfection_high_titer_clearance);

	file = data_dir + "/" + chi_sim::Parameters::instance()->getStringParameter(VK_REINFECT_LOWT_CLEAR_FILE);
    // std::cout << "VK re-infection low titer clearance file: " << file << std::endl;
	loadViralLoadSeries(file, viral_loads_reinfection_low_titer_clearance);

	file = data_dir + "/" + chi_sim::Parameters::instance()->getStringParameter(VK_TREATMENT_FILE);
    // std::cout << "VK treatment file: " << file << std::endl;
	loadViralLoadSeries(file, viral_loads_treated);
}

/**
 * @brief Find the closest map key to the provided key argument and return the corresponding
 *        value.  The transmi probability map uses viral load as the keys, so this finds
 *        the closest key and returns the transmit prob.
 * 
 * @param viral_load the viral load
 * @param transmit_prob_map the map of viral load -> transmit probability
 * @return double the transmisiont probability
 */
double ViralKinetics::get_transmission_probability(double viral_load){
  	double val = 0;

	auto it = transmit_prob_map.lower_bound(viral_load);
	
	// If iterator is the end, return the end-1 map valu
	if (it == transmit_prob_map.end()){
		val = std::prev(transmit_prob_map.end())->second;
	}
	// Otherwise need to compare the lower bound key with the --lower bound key
	// to see which is closed to the key argument
	else{
		double a = (it) -> first ;
		double a_val = it -> second;
		double b = (--it) -> first ;
		double b_val = it -> second;
		
		double x  = fabs(viral_load-a);
		double y  = fabs(viral_load-b);

		if (x < y) val = a_val;
		else val = b_val;
	
		return val;
	}
}

ViralKinetics::~ViralKinetics(){

}

}
