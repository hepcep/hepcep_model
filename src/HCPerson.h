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

using namespace std;

namespace hepcep {

using AbsPersonT =  chi_sim::AbstractPerson<HCPlace, int>;

// Holds Person Data loaded from persons input file and used to create HCPerson instances.
struct HCPersonData {
	string label;
	unsigned int drug_outDegree;
	unsigned int drug_inDegree;
	double fractionReceptSharing;
	string gender;
	double ageStarted;
	string birthDate;
	string surveyDate;
	string hcvState;
	double injectionIntensity;
	string zipCode;
	
};

class HCPerson : public AbsPersonT {

protected:
	string label;
	unsigned int drug_outDegree;
	unsigned int drug_inDegree;
	double fractionReceptSharing;
	string gender;
	double ageStarted;
	string birthDate;
	string surveyDate;
	string hcvState;
	double injectionIntensity;
	string zipCode;

public:
    HCPerson(unsigned int id);
	HCPerson(unsigned int id, HCPersonData& data);
	
    virtual ~HCPerson();

    // not used in initial version
    void fillSendData(vector<int>& data) {}

    // not used in initial version
    void selectNextPlace(chi_sim::Calendar& cal, chi_sim::NextPlace<HCPlace>& next_act) {}

    void doSomething();
};

using PersonPtr = shared_ptr<HCPerson>;

} /* namespace hepcep */

#endif /* SRC_HCPERSON_H_ */
