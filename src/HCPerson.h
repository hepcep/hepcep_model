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
#include "OpiodTreatment.h"

namespace hepcep {

using AbsPersonT =  chi_sim::AbstractPerson<HCPlace, int>;
using PersonPtr = std::shared_ptr<HCPerson>;

class AttributeWriter;


// Holds Person Data loaded from persons input file and used to create HCPerson instances.
struct HCPersonData {
	double age;
	double ageStarted;
	std::string gender;
	std::string race;
	std::string syringeSource;
	unsigned int zipCode;
	HCVState hcvState = HCVState::UNKNOWN;
	unsigned int drug_inDegree;
	unsigned int drug_outDegree;
	double injectionIntensity;
	double fractionReceptSharing;
	bool early_career;

};

class HCPerson : public AbsPersonT {

private:
	friend void write_person(HCPerson* person, AttributeWriter& write, double);
	friend PersonPtr read_person(NamedListAttribute*, std::map<unsigned int,ZonePtr>&, double);

protected:

	std::string label;

	double age;         // age in years
	double ageStarted;
	Gender gender;
	Race race;
	HarmReduction syringeSource;
	unsigned int zipCode;
//	HCVState hcvState;
	unsigned int drug_inDegree;
	unsigned int drug_outDegree;
	double injectionIntensity;
	double fractionReceptSharing;
	double lastExposureDate;
	double lastInfectionDate;
	double deactivateAt;
    double injectionIntensityMultiplier;
	
    ZonePtr myZone;
	OpiodTreatment opiod_treatment;

	std::shared_ptr<Immunology> immunology;

	bool active = false;


public:
	HCPerson(unsigned int id);
	HCPerson(unsigned int id, HCPersonData& data);

	// for deserialization, don't use otherwise!!
	HCPerson(unsigned int id, HCPersonData& data, std::shared_ptr<Immunology> imm);

	virtual ~HCPerson();

	// not used in initial version
	void fillSendData(std::vector<int>& data) {}

	// not used in initial version
	void selectNextPlace(chi_sim::Calendar& cal, chi_sim::NextPlace<HCPlace>& next_act) {}

	void step(NetworkPtr<HCPerson> network);

	double getDemographicDistance(PersonPtr other) const;
	bool activate(double residualBurninDays, double elapsedCareerDays,double statusReportFrequency);
	bool scheduleEnd(double residualBurninDays, double elapsedCareerDays);

	void deactivate();
	void receive_equipment_or_drugs(NetworkPtr<HCPerson> network);
	void reportStatus();
	void startTreatment();
    void startOpioidTreatment();
	void endRelationship(PersonPtr buddy, NetworkPtr<HCPerson> network);

	unsigned int getDrugReceptDegree() const;
	unsigned int getDrugGivingDegree() const;
	ZonePtr getZone() const;
	void setZone(ZonePtr zone);
	unsigned int getZipcode() const;
	double getAge() const;
	void setAge(double age);
	double getAgeStarted() const;
	Race getRace() const;
	Gender getGender() const;
	HarmReduction getSyringeSource() const;
	HCVState getHCVState() const;
	void setHcvInitialState(HCVState hcvState, double tick);
	double getInjectionIntensity() const;
	double getFractionReceptSharing() const;
	void setLastExposureDate(double tick);
	double getLastExposureDate() const;
	void setLastInfectionDate(double tick);
	double getLastInfectionDate() const;
	bool isHcvABpos() const;
	bool isHcvRNA() const;
	bool isActive() const;
	bool isInfectedToday() const;

	bool isCured() const;
	bool isInTreatment() const;
	bool isInHarmReduction() const;
	bool isPostTreatment() const;
	bool isTreatable() const;
	bool getTestedHCV() const;

	double getDeactivateAt() const;

    bool isInOpioidTreatment() const;
    
	friend std::ostream& operator<<(std::ostream& os, const HCPerson& p);

};


void writePerson(HCPerson* person, AttributeWriter& write);

} /* namespace hepcep */

#endif /* SRC_HCPERSON_H_ */
