/*
 * Race.cpp
 *
 *  Created on: Mar 12, 2018
 *      Author: nick
 */

#include "Race.h"

#include <exception>
#include "boost/algorithm/string/predicate.hpp"

namespace hepcep {

const Race Race::NH_WHITE(Race::NHWhite, "NHWhite");
const Race Race::NH_BLACK(Race::NHBlack, "NHBlack");
const Race Race::HISPANIC(Race::Hispanic, "Hispanic");
const Race Race::OTHER(Race::Other, "Other");

const std::vector<Race> Race::values_(
        { Race::NH_WHITE, Race::NH_BLACK, Race::HISPANIC, Race::OTHER });

bool Race::operator==(const Race& rhs) const {
    return val_ == rhs.val_;
}

bool Race::operator!=(const Race& rhs) const {
    return val_ != rhs.val_;
}

Race::Race(const Value& val, const std::string& string_val) :
        val_(val), string_val_(string_val) {
}

std::string Race::stringValue() const {
    return string_val_;
}

const std::vector<Race>& Race::values() {
    return Race::values_;
}

const Race::Value Race::value() const {
    return val_;
}

Race Race::valueOf(const std::string& string_val) {
    for (auto& race : values()) {
        if (boost::iequals(string_val, race.stringValue())) {
            return race;
        }
    }

    throw std::invalid_argument("Unknown race type: " + string_val);
}

} /* namespace hepcep */
