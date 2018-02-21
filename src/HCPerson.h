/*
 * HCPerson.h
 *
 *  Created on: Nov 27, 2017
 *      Author: nick
 */

#ifndef SRC_HCPERSON_H_
#define SRC_HCPERSON_H_

#include <vector>
#include <memory>

#include "chi_sim/AbstractPerson.h"
#include "chi_sim/Calendar.h"
#include "chi_sim/NextPlace.h"

#include "HCPlace.h"
#include "Zone.h"

namespace hepcep {

using AbsPersonT =  chi_sim::AbstractPerson<HCPlace, int>;
using PersonPtr = std::shared_ptr<HCPerson>;

enum class HCV_State{susceptible, exposed, infectiousacute, recovered, cured, chronic, unknown, ABPOS};

// Holds Person Data loaded from persons input file and used to create HCPerson instances.
struct HCPersonData {
	//	std::string label;
	//	unsigned int drug_outDegree;
	//	unsigned int drug_inDegree;
	//	double fractionReceptSharing;
	//	std::string gender;
	//	double ageStarted;
	//	std::string birthDate;
	//	std::string surveyDate;
	//	std::string hcvState;
	//	double injectionIntensity;
	//	std::string zipCode;

	double age;
	double ageStarted;
	std::string gender;
	std::string race;
	std::string syringeSource;
	std::string zipCode;
	HCV_State hcvState;
	unsigned int drug_inDegree;
	unsigned int drug_outDegree;
	double injectionIntensity;
	double fractionReceptSharing;

};

class HCPerson : public AbsPersonT {

protected:
	std::string label;

	double age;
	double ageStarted;
	std::string gender;
	std::string race;
	std::string syringeSource;
	std::string zipCode;
	HCV_State hcvState;
	unsigned int drug_inDegree;
	unsigned int drug_outDegree;
	double injectionIntensity;
	double fractionReceptSharing;

	ZonePtr myZone;

public:
	HCPerson(unsigned int id);
	HCPerson(unsigned int id, HCPersonData& data);

	virtual ~HCPerson();

	// not used in initial version
	void fillSendData(std::vector<int>& data) {}

	// not used in initial version
	void selectNextPlace(chi_sim::Calendar& cal, chi_sim::NextPlace<HCPlace>& next_act) {}

	void doSomething();

	double getDemographicDistance(PersonPtr other);

	unsigned int getDrugReceptDegree() const {
		return drug_inDegree;
	}

	unsigned int getDrugGivingDegree() const {
		return drug_outDegree;
	}

	ZonePtr getZone() const {
		return myZone;
	}

	void setZone(ZonePtr zone){
		myZone = zone;
	}

	std::string getZipcode() const {
		return zipCode;
	}

	double getAge() const {
		return age;
	}

	double getAgeStarted() const {
		return ageStarted;
	}

	std::string getRace() const {
		return race;
	}

	std::string getGender() const {
		return gender;
	}

	std::string getSyringeSource() const {
		return syringeSource;
	}

	HCV_State getHCVState(){
		return hcvState;
	}

	double getInjectionIntensity() const {
		return injectionIntensity;
	}

	double getFractionReceptSharing() const {
		return fractionReceptSharing;
	}

	static std::string HCVStateToString(HCV_State state){
		switch(state){
			case HCV_State::susceptible: return "susceptible";
			case HCV_State::exposed: return "exposed";
			case HCV_State::infectiousacute: return "infectiousacute";
			case HCV_State::recovered: return "recovered";
			case HCV_State::cured: return "cured";
			case HCV_State::chronic: return "chronic";
			case HCV_State::ABPOS: return "ABPOS";
			case HCV_State::unknown: return "unknown";
		}
		return "unknown";
	}

	static HCV_State stringToHCVState(const std::string& str){

			if (str.empty()) return HCV_State::unknown;

			else if(str == "ABPOS") return HCV_State::ABPOS;
	    else if(str == "susceptible") return HCV_State::susceptible;

	    else if(str == "exposed") return HCV_State::exposed;
	    else if(str == "infectiousacute") return HCV_State::infectiousacute;
	    else if(str == "recovered") return HCV_State::recovered;
	    else if(str == "cured") return HCV_State::cured;
	    else if(str == "chronic") return HCV_State::chronic;

	    return HCV_State::unknown;
	}
};
} /* namespace hepcep */

#endif /* SRC_HCPERSON_H_ */
