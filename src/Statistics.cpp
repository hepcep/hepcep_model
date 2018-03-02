/*
 * Statistics.cpp
 */
#include <iostream>
#include <memory>

#include "Statistics.h"
#include "HCPerson.h"

namespace hepcep {

Statistics* Statistics::instance_ = new Statistics();

Statistics::Statistics() {
}

Statistics::~Statistics() {
}

Statistics* Statistics::instance() {
    return instance_;
}

void Statistics::logStatusChange(LogType logType, HCPerson* person, const std::string& msg) {

}

} /* namespace seir */


