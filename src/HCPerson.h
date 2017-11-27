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

namespace hepcep {

using AbsPersonT =  chi_sim::AbstractPerson<HCPlace, int>;

class HCPerson : public AbsPersonT {

public:
    HCPerson(unsigned int id);
    virtual ~HCPerson();

    // not used in initial version
    void fillSendData(std::vector<int>& data) {}

    // not used in initial version
    void selectNextPlace(chi_sim::Calendar& cal, chi_sim::NextPlace<HCPlace>& next_act) {}

    void doSomething();
};

using PersonPtr = std::shared_ptr<HCPerson>;

} /* namespace hepcep */

#endif /* SRC_HCPERSON_H_ */
