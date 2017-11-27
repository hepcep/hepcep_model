/*
 * HCPlace.h
 *
 *  Created on: Nov 27, 2017
 *      Author: nick
 */

#ifndef SRC_HCPLACE_H_
#define SRC_HCPLACE_H_

#include <memory>

namespace hepcep {

class HCPerson;

class HCPlace {

public:
    HCPlace();
    virtual ~HCPlace();

    // not used in initial version as persons don't move to places
    virtual void addPerson(const std::shared_ptr<HCPerson>& person, int act_type) {}

};

} /* namespace hepcep */

#endif /* SRC_HCPLACE_H_ */
