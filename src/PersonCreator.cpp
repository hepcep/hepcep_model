/*
 * PersonCreator.cpp
 *
 *  
 */

#include "repast_hpc/RepastProcess.h"

#include "PersonCreator.h"
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

void create_persons(const std::string& filename, std::map<unsigned int, PersonPtr>& persons) {
    SVReader reader(filename, ',');
    std::vector<std::string> line;

	// Header
	// DB_Label,Drug_Giving_Degree,Drug_Recept_Degree,Fraction_Recept_Sharing,Gender,
	//   Age_Started,Birth_Date,Survey_Date,HCV_State,Injection_Intensity,Prelimary_Zip,Race,Syringe_Source
	
    // skip header
    reader.next(line);

    // not strictly necessary in initial version, as world size is always 1
    // but kept here to illustrate how to only create "local persons"
    int my_rank = repast::RepastProcess::instance()->rank();
    int world_size = repast::RepastProcess::instance()->worldSize();

	unsigned int id = 0;
	int p_rank = 0;
	
	unsigned int person_count = 5;
	
    while (reader.next(line) && id <= person_count) {
		
		// Assign the ID based on read line number (increment)
//        unsigned int id = std::stoul(line[ID_IDX]);

		std::string label = line[DB_LABEL_IDX];
		unsigned int drug_outDegree = std::stoul(line[DRUG_GIV_DEG_IDX]);
		unsigned int drug_inDegree = std::stoul(line[DRUG_REC_DEG_IDX]);
		double fractionReceptSharing = std::stod(line[FRAC_REC_SHAR_IDX]);
		std::string gender = line[GENDER_IDX];
		double ageStarted = std::stod(line[AGE_STARTED_IDX]);
		std::string birthDate = line[BIRTH_DATE_IDX];
		std::string surveyDate = line[SURVEY_DATE_IDX];
		std::string hcvState = line[HCV_STATE_IDX];
		double injectionIntensity = std::stod(line[INJECT_INTENS_IDX]);
		std::string zipCode = line[PRELIM_ZIP_IDX];

	// TODO use constant rank 0 for now
//        int p_rank = std::stoi(line[RANK_IDX]);
        
		if (p_rank == my_rank || world_size == 1) {
            persons.emplace(id, std::make_shared<HCPerson>(id));
        }
		id++;  // increment id count
    }
}

}