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
#include "HCVState.h"
#include "HarmReduction.h"
#include "Immunology.h"
#include "Gender.h"
#include "Network.h"
#include "Race.h"
#include "Zone.h"

namespace hepcep {

using AbsPersonT =  chi_sim::AbstractPerson<HCPlace, int>;
using PersonPtr = std::shared_ptr<HCPerson>;


// Holds Person Data loaded from persons input file and used to create HCPerson instances.
struct HCPersonData {
	double age;
	double ageStarted;
	std::string gender;
	std::string race;
	std::string syringeSource;
	std::string zipCode;
	HCVState hcvState = HCVState::UNKNOWN;
	unsigned int drug_inDegree;
	unsigned int drug_outDegree;
	double injectionIntensity;
	double fractionReceptSharing;

};

class HCPerson : public AbsPersonT {

protected:
	std::string label;

	double age;         // age in years
	double ageStarted;
	Gender gender;
	Race race;
	HarmReduction syringeSource;
	std::string zipCode;
//	HCVState hcvState;
	unsigned int drug_inDegree;
	unsigned int drug_outDegree;
	double injectionIntensity;
	double fractionReceptSharing;
	double lastExposureDate;

	ZonePtr myZone;

	std::shared_ptr<Immunology> immunology;

	bool active = false;

public:
	HCPerson(unsigned int id);
	HCPerson(unsigned int id, HCPersonData& data);

	virtual ~HCPerson();

	// not used in initial version
	void fillSendData(std::vector<int>& data) {}

	// not used in initial version
	void selectNextPlace(chi_sim::Calendar& cal, chi_sim::NextPlace<HCPlace>& next_act) {}

	void step(NetworkPtr<HCPerson> network);

	double getDemographicDistance(PersonPtr other) const;

	bool activate(double residual_burnin_days, double elapsed_career_days,
			double status_report_frequency);

	void deactivate();
	void receive_equipment_or_drugs(NetworkPtr<HCPerson> network);
	void reportStatus();
	void startTreatment();
	void endRelationship(PersonPtr buddy, NetworkPtr<HCPerson> network);

	unsigned int getDrugReceptDegree() const;
	unsigned int getDrugGivingDegree() const;
	ZonePtr getZone() const;
	void setZone(ZonePtr zone);
	std::string getZipcode() const;
	double getAge() const;
	void setAge(double age);
	double getAgeStarted() const;
	Race getRace() const;
	Gender getGender() const;
	HarmReduction getSyringeSource() const;
	HCVState getHCVState() const;
	double getInjectionIntensity() const;
	double getFractionReceptSharing() const;
	void setLastExposureDate(double tick);
	double getLastExposureDate() const;
	bool isHcvABpos() const;
	bool isHcvRNA() const;

	bool isCured() const;
	bool isInTreatment() const;
	bool isInHarmReduction() const;
	bool isPostTreatment() const;
	bool isTreatable() const;

	friend std::ostream& operator<<(std::ostream& os, const HCPerson& p);

};
} /* namespace hepcep */

#endif /* SRC_HCPERSON_H_ */
