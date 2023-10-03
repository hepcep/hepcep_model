/*
 * PersonCreator.cpp
 *
 *  
 */

#include "repast_hpc/RepastProcess.h"

#include "chi_sim/Parameters.h"

#include "parameters_constants.h"
#include "PersonDataLoader.h"
#include "SVReader.h"

namespace hepcep {

const int AGE_IDX = 0;				// Age (String)
const int AGE_STARTED_IDX = 1;		// Age started year (float double)
const int GENDER_IDX = 2;			// Gender : MALE | FEMALE (String)
const int RACE_IDX = 3;				// Race  (String)
const int SYRINGE_SOURCE_IDX = 4;	// Syringe Source (String)
const int ZIP_IDX = 5;				// Initial zip code (String) since some are not all numeric
const int HCV_STATE_IDX = 6;		// HCV State (String)  susceptible, exposed, infectiousacute, recovered, cured, chronic, unknown, ABPOS
const int DRUG_REC_DEG_IDX = 7;		// Network drug recept (in) degree (int)
const int DRUG_GIV_DEG_IDX = 8;		// Network drug giving (out) degree (int)
const int INJECT_INTENS_IDX = 9;	// Injection intensity (float double)
const int FRAC_REC_SHAR_IDX = 10;	// Fraction recept sharing (float double)

const int ERGM_VERTEX_NAME_IDX = 11; // Vertex name in ERGM network loading

void loadPersonData(const string& filename, std::vector<HCPersonData> & personData, const std::string& pwid_data_type) {
	SVReader reader(filename, ',');
	vector<std::string> line;

	std::cout << "PWID file: " << filename << std::endl;
	std::cout << "PWID input data type: " << pwid_data_type << std::endl;

	// Determines if person is early career
	double maturityThreshold = chi_sim::Parameters::instance()->getDoubleParameter(MATURITY_THRESHOLD);

	// Header
	// Age, Age_Started, Gender, Race, SyringeSource, Zip, HCV	Drug_in_degree,
	// 	Drug_out_degree, Daily_injection_intensity, Fraction_recept_sharing

	// skip header
	reader.next(line);

	while (reader.next(line)) {
		HCPersonData data;

		data.age = std::stod(line[AGE_IDX]);
		data.ageStarted = std::stod(line[AGE_STARTED_IDX]);
		data.gender = line[GENDER_IDX];
		data.race = line[RACE_IDX];
		data.syringeSource = line[SYRINGE_SOURCE_IDX];
		data.zipCode = std::stoul(line[ZIP_IDX]);
		data.hcvState = HCVState::valueOf(line[HCV_STATE_IDX]);

		data.drug_inDegree = std::stoul(line[DRUG_REC_DEG_IDX]);
		data.drug_outDegree = std::stoul(line[DRUG_GIV_DEG_IDX]);
		
		
		// Handle the input data for some attributes differently if loading CNEP+ vs ERGM
		if (pwid_data_type == "CNEP+"){
			data.injectionIntensity = std::stod(line[INJECT_INTENS_IDX]);
			data.ergm_injectionIntensity = "";  // not used
			data.ergm_vertex_name = 0;  // not used
		}
		else {  // ERGM input data type
			data.injectionIntensity = 0;  // Determined via ERGM injection intensity
			data.ergm_injectionIntensity = line[INJECT_INTENS_IDX]; // String injectinon frequency
			data.ergm_vertex_name = std::stoul(line[ERGM_VERTEX_NAME_IDX]);
		}
		data.fractionReceptSharing = std::stod(line[FRAC_REC_SHAR_IDX]);

		data.early_career = (data.age - data.ageStarted) < maturityThreshold;

		personData.push_back(data);
	}
}

/**
 * Load a list of PWID network edge data. Each row starts with a unique source person "V1"
 * followed by a variable number of "V2" target persons for which network edges should be
 * created.  V1 and V2 ID integers match the HCPersonData.ergm_vertex_name
 */
void load_pwid_edge_data(const std::string& filename, std::unordered_map<unsigned int, 
 		std::vector<int>> & edge_data) {

	SVReader reader(filename, ',');
	std::vector<std::string> line;

	// Header
	reader.next(line);

	// Each row represents a unique mapping of person V1 to persons V2_1, V2_2, ...etc
	// The first element is the person vertex ID matching HCPersonData.ergm_vertex_name
	int line_number = 1;
	while (reader.next(line)) {
		unsigned int source_person_vertex_id = std::stoul(line[0]);
		
		int rowlen = line.size();
 		std::vector<int> target_vertex_ids;
 		for (int i=1; i<rowlen; i++){            // start i=1
			// Catch exceptions with malformed data values in the input file
			try {
				target_vertex_ids.push_back(std::stoul(line[i]));
			}
			catch (std::exception& e){
				std::cout << "Error reading value from file: " << filename 
					<< " at line " << line_number << ", position " << i << std::endl;
				
				std::cout << e.what() << std::endl;
			}
 		}
 		edge_data[source_person_vertex_id] = target_vertex_ids;
		line_number++;
	}
}


}
