/*
 * utilities.cpp
 *
 *  Created on: Mar 2, 2018
 *      Author: nick
 */

#include <boost/algorithm/string/predicate.hpp>

#include "utilities.h"

#include "constants.h"

namespace hepcep {

std::string parse_gender(const std::string& str) {
    if (boost::iequals(str, MALE)) return MALE;
    if (boost::iequals(str, FEMALE)) return FEMALE;
    throw std::invalid_argument("Invalid gender: " + str);
}

} /* namespace hepcep */
