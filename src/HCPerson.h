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

#include "EndHarmReductionFunctor.h"
#include "HCPlace.h"
#include "HCVState.h"
#include "HarmReduction.h"
#include "APK_Immunology.h"
#include "VK_Immunology.h"
#include "Gender.h"
#include "Network.h"
#include "Race.h"
#include "Zone.h"
#include "OpioidTreatmentDrug.h"

namespace hepcep {

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

	unsigned int person_id; // Needed for chisim 0.4.2 but not used here
	int place_id;  			// Needed for chisim 0.4.2 but not used here
    int act_type;			// Needed for chisim 0.4.2 but not used here

	// Person attributes specific to ERGM type input data
	std::string ergm_injectionIntensity;
	int ergm_vertex_name;

};

using AbsPersonT =  chi_sim::AbstractPerson<HCPlace, HCPersonData>;
using PersonPtr = std::shared_ptr<HCPerson>;

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

	double last_hcv_screened_date;   // Day (tick) person was last screening

	// Person attributes specific to ERGM type input data
	std::string ergm_injectionIntensity;
	int ergm_vertex_name;
	
    ZonePtr myZone;

	std::shared_ptr<Immunology> immunology;

	std::vector<boost::shared_ptr<Event>> scheduled_actions;

	bool active = false;
	bool in_opioid_treatment = false;
    double last_opioid_treatment_start_time = 0;
    
    DrugName currentOpioidTreatmentDrug;


public:
	HCPerson(unsigned int id);
	HCPerson(unsigned int id, HCPersonData& data);

	// for deserialization, don't use otherwise!!
	HCPerson(unsigned int id, HCPersonData& data, std::shared_ptr<Immunology> imm);

	virtual ~HCPerson();

	// not used in initial version
	void fillSendData(HCPersonData& data) {}

	// not used in initial version
	void selectNextPlace(chi_sim::Calendar& cal, chi_sim::NextPlace<HCPlace>& next_act) {}

	void step(NetworkPtr<HCPerson> network);

	double getDemographicDistance(PersonPtr other) const;
	bool activate(double residualBurninDays, double elapsedCareerDays);
	bool scheduleEnd(double residualBurninDays, double elapsedCareerDays);

	void deactivate();
	void purgeActions();
	void receive_equipment_or_drugs(NetworkPtr<HCPerson> network);
	void reportStatus();
	void startTreatment();
	void endRelationship(PersonPtr buddy, NetworkPtr<HCPerson> network);

	unsigned int getDrugReceptDegree() const;
	unsigned int getDrugGivingDegree() const;
	void setDrugReceptDegree(unsigned int d);
	void setDrugGivingDegree(unsigned int d);
	
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
	void setInjectionIntensity(double i);
	double getFractionReceptSharing() const;
	void setLastExposureDate(double tick);
	double getLastExposureDate() const;
	void setLastInfectionDate(double tick);
	double getLastInfectionDate() const;
	bool isHcvABpos() const;
	bool isHcvRNA() const;
	bool isActive() const;
	bool isInfectedToday() const;
	std::string get_ergm_injectionIntensity() const;
	int get_ergm_vertex_name() const;

	VKProfile getVKProfile() const;
	bool isInTreatment() const;
	bool isInHarmReduction() const;
	bool isPostTreatment() const;
	bool isTreatable() const;
	bool getTestedHCV() const;
	double get_transmissibility() const;
	double get_viral_load() const;
	double getDeactivateAt() const;

    bool isInOpioidTreatment() const;
	void setInOpioidTreatment(bool val);
	
	void set_last_hcv_screen_date(double tick);
	bool is_eligible_for_hcv_screeening(double tick, int screening_interval_days) const;
      
    double getLastOpioidTreatmentStartTime() const {
        return last_opioid_treatment_start_time;
    }

    void setLastOpioidTreatmentStartTime(double t){
        last_opioid_treatment_start_time = t;
    }
    
    
    DrugName getCurrentOpioidTreatmentDrug() const;
    void setCurrentOpioidTreatmentDrug(DrugName drug);

	void setInjectionIntensityMultiplier(double val) {
		injectionIntensityMultiplier = val;
	}
    
	friend std::ostream& operator<<(std::ostream& os, const HCPerson& p);

};


void writePerson(HCPerson* person, AttributeWriter& write);

} /* namespace hepcep */

#endif /* SRC_HCPERSON_H_ */
