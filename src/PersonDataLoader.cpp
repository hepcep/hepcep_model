/*
 * PersonCreator.cpp
 *
 *  
 */

#include "repast_hpc/RepastProcess.h"

#include "PersonDataLoader.h"
#include "SVReader.h"

namespace hepcep {

const int DB_LABEL_IDX = 0;			// DB label (String)
const int DRUG_GIV_DEG_IDX = 1;		// Network drug giving degree (int)
const int DRUG_REC_DEG_IDX = 2;		// Network drug recept degree (int)
const int FRAC_REC_SHAR_IDX = 3;	// Fraction recept sharing (float double)
const int GENDER_IDX = 4;			// Gender : MALE | FEMALE (String)
const int AGE_STARTED_IDX = 5;		// Age started year (float double)
const int BIRTH_DATE_IDX = 6;		// Birth date YYYY-MM-DD (String)
const int SURVEY_DATE_IDX = 7;		// Survey date YYYY-MM-DD (String)
const int HCV_STATE_IDX = 8;		// HCV State (String)  susceptible, exposed, infectiousacute, recovered, cured, chronic, unknown, ABPOS
const int INJECT_INTENS_IDX = 9;	// Injection intensity (float double)
const int PRELIM_ZIP_IDX = 10;		// Initial zip code (String) since some are not all numeric

void loadPersonData(const string& filename, deque<HCPersonData> & personData) {
    SVReader reader(filename, ',');
    vector<std::string> line;

	// Header
	// DB_Label,Drug_Giving_Degree,Drug_Recept_Degree,Fraction_Recept_Sharing,Gender,
	//   Age_Started,Birth_Date,Survey_Date,HCV_State,Injection_Intensity,Prelimary_Zip,Race,Syringe_Source
	
    // skip header
    reader.next(line);

    // not strictly necessary in initial version, as world size is always 1
    // but kept here to illustrate how to only create "local persons"
    int my_rank = repast::RepastProcess::instance()->rank();
    int world_size = repast::RepastProcess::instance()->worldSize();

//	unsigned int id = 0;
//	int p_rank = 0;
	
//	unsigned int person_count = 5;
	
    while (reader.next(line)) {
//	while (reader.next(line) && id <= person_count) {
		HCPersonData data;
		
		// Assign the ID based on read line number (increment)
//        unsigned int id = std::stoul(line[ID_IDX]);

		data.label = line[DB_LABEL_IDX];
		data.drug_outDegree = std::stoul(line[DRUG_GIV_DEG_IDX]);
		data.drug_inDegree = std::stoul(line[DRUG_REC_DEG_IDX]);
		data.fractionReceptSharing = std::stod(line[FRAC_REC_SHAR_IDX]);
		data.gender = line[GENDER_IDX];
		data.ageStarted = std::stod(line[AGE_STARTED_IDX]);
		data.birthDate = line[BIRTH_DATE_IDX];
		data.surveyDate = line[SURVEY_DATE_IDX];
		data.hcvState = line[HCV_STATE_IDX];
		data.injectionIntensity = std::stod(line[INJECT_INTENS_IDX]);
		data.zipCode = line[PRELIM_ZIP_IDX];

		personData.push_back(data);
    }
}

}
