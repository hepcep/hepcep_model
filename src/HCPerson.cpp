/*
 * HCPerson.cpp
 *
 *  Created on: Nov 27, 2017
 *      Author: nick
 */

#include "HCPerson.h"

namespace hepcep {

HCPerson::HCPerson(unsigned int id) : AbsPersonT(id) {
}

HCPerson::HCPerson(unsigned int id, HCPersonData& data) : AbsPersonT(id) {
	drug_outDegree = data.drug_outDegree;
	drug_inDegree = data.drug_inDegree;
	fractionReceptSharing = data.fractionReceptSharing;
	
	// TODO intialize other as per APK IDUBuilder
	
}

HCPerson::~HCPerson() {
}

void HCPerson::doSomething() {
    std::cout << id_ << ": hello " << std::endl;
}

} /* namespace hepcep */
