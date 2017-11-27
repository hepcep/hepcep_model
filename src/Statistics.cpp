/*
 * Statistics.cpp
 */
#include <iostream>

#include "Statistics.h"

namespace hepcep {

Statistics* Statistics::instance_ = new Statistics();

Statistics::Statistics() : stats() {
}

Statistics::~Statistics() {
}

Statistics* Statistics::instance() {
	return instance_;
}

std::vector<DSDoublePtrT> Statistics::createDataSources() {
    std::vector<DSDoublePtrT> vec;

    vec.push_back(std::make_shared<SimpleDataSource<double>>("val", &stats.val));
    return vec;
}

void Statistics::increment(long amount) {
    stats.val += amount;
}

void Statistics::reset() {
    stats.val = 0;
}

void Statistics::print(double tick) {
	std::cout << tick << ": " << stats.val << std::endl;
}

} /* namespace seir */
