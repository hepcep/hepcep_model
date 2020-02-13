/*
 * HCModel.h
 *
 *  Created on: Nov 27, 2017
 *      Author: nick
 */

#ifndef SRC_HCMODEL_H_
#define SRC_HCMODEL_H_

#include <unordered_map>

#include "chi_sim/AbstractModel.h"

#include "HCPerson.h"
#include "Zone.h"
#include "EnrollmentMethod.h"
#include "FileSink.h"
#include "Network.h"
#include "Edge.h"
#include "network_utils.h"
#include "PersonCreator.h"

namespace hepcep {

using AbsModelT = chi_sim::AbstractModel<HCPerson, HCPlace, int>;

class HCModel: public AbsModelT {

private:
	int run;
//    FileSink<double> file_sink;

	std::shared_ptr<PersonCreator> personCreator;

	double netInflow;

protected:
	NetworkPtr<HCPerson> network;

	// PersonData vector used for generating new HCPerson instances.
	std::vector<HCPersonData> personData;

	std::map<unsigned int,ZonePtr> zoneMap;
	std::unordered_map<unsigned int, std::unordered_map<unsigned int,double>> zoneDistanceMap;
	std::map<unsigned int, std::vector<PersonPtr>> effectiveZonePopulation;

	std::map<EnrollmentMethod, double> treatmentEnrollmentProb;
	std::map<EnrollmentMethod, double> treatmentEnrollmentResidual;

    std::map<DrugName, double> opioidTreatmentEnrollmentProb;
    std::map<DrugName, double> opioidTreatmentEnrollmentResidual;
    
	int totalIDUPopulation = 0;

	double interactionHomeCutoff;
	double interactionRateDrugSites;
	double interactionRateExzone;
	double interactionRateConst;
	double treatmentEnrollPerPY;
    double opioidTreatmentEnrollPerPY;
	double linkingTimeWindow;
	double homophily;
    double burnInDays;

	// not used in initial version
	void nextActSelected(PersonPtr& person, chi_sim::NextPlace<HCPlace>& next_act) override {
	}

	/**
	 * Create a person from the specified parameters.
	 *
	 * @param index the index of the start of the block of data for this Person
	 */
	// not used in initial version
	PersonPtr createPerson(unsigned int p_id, int index, int* data) override {
		return std::shared_ptr<HCPerson>();
	}

	/**
	 * Update the state of the specified person with the data in data.
	 *
	 * @param index the index of the start of the block of data for this Person
	 */
	// not used in initial version
	virtual void updatePerson(PersonPtr& person, int index, int* data) override {
	}

	void generateArrivingPersons();
	void burnInControl();
	void burnInEnd();
	void performInitialLinking();
	void performLinking();
	double interactionRate(const ZonePtr& zone1, const ZonePtr& zone2);
	void zoneCensus();
	void linkZones(const ZonePtr& zone1, const ZonePtr& zone2);
	void tryConnect(const PersonPtr& person1, const PersonPtr& person2);
	void daa_treatment();
    void opioid_treatment();
	void treatmentSelection(EnrollmentMethod mthd, std::vector<PersonPtr>& candidates,
			std::vector<PersonPtr>& enrolled, double enrollmentTarget);
            
    void opioidTreatmentSelection(DrugName drug, std::vector<PersonPtr>& candidates,
			std::vector<PersonPtr>& enrolled, double enrollmentTarget);

public:
	HCModel(repast::Properties& props, unsigned int moved_data_size);
	virtual ~HCModel();

	void atEnd();
	void step();


};

// random generator function used in std lib functions that need a random generator
int myrandom (int i);

} /* namespace hepcep */

#endif /* SRC_HCMODEL_H_ */
