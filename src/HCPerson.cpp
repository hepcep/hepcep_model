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

HCPerson::~HCPerson() {
}

void HCPerson::doSomething() {
    std::cout << id_ << ": hello " << std::endl;
}

} /* namespace hepcep */
