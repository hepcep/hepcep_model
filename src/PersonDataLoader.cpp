/*
 * PersonCreator.cpp
 *
 *  
 */

#include "repast_hpc/RepastProcess.h"

#include "PersonDataLoader.h"
#include "SVReader.h"

namespace hepcep {

const int AGE_IDX = 0;						// Age (String)
const int AGE_STARTED_IDX = 1;		// Age started year (float double)
const int GENDER_IDX = 2;					// Gender : MALE | FEMALE (String)
const int RACE_IDX = 3;						// Race  (String)
const int SYRINGE_SOURCE_IDX = 4;	// Syringe Source (String)
const int ZIP_IDX = 5;						// Initial zip code (String) since some are not all numeric
const int HCV_STATE_IDX = 6;			// HCV State (String)  susceptible, exposed, infectiousacute, recovered, cured, chronic, unknown, ABPOS
const int DRUG_REC_DEG_IDX = 7;		// Network drug recept (in) degree (int)
const int DRUG_GIV_DEG_IDX = 8;		// Network drug giving (out) degree (int)
const int INJECT_INTENS_IDX = 9;	// Injection intensity (float double)
const int FRAC_REC_SHAR_IDX = 10;	// Fraction recept sharing (float double)

void loadPersonData(const string& filename, std::vector<HCPersonData> & personData) {
	SVReader reader(filename, ',');
	vector<std::string> line;

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
		data.zipCode = line[ZIP_IDX];
		data.hcvState = HCVState::valueOf(line[HCV_STATE_IDX]);

		//		data.label = line[DB_LABEL_IDX];
		data.drug_inDegree = std::stoul(line[DRUG_REC_DEG_IDX]);
		data.drug_outDegree = std::stoul(line[DRUG_GIV_DEG_IDX]);
		data.injectionIntensity = std::stod(line[INJECT_INTENS_IDX]);
		data.fractionReceptSharing = std::stod(line[FRAC_REC_SHAR_IDX]);

		personData.push_back(data);
	}
}

}
