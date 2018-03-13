/*
 * AgeGroup.cpp
 *
 *  Created on: Mar 13, 2018
 *      Author: nick
 */
#include <exception>

#include "boost/algorithm/string/predicate.hpp"

#include "AgeGroup.h"

namespace hepcep {

const AgeGroup AgeGroup::LEQ_30(AgeGroup::LEQ30, "LEQ_30");
const AgeGroup AgeGroup::OVER_30(AgeGroup::Over30, "OVER_30");

const std::vector<AgeGroup> AgeGroup::values_({ AgeGroup::LEQ_30, AgeGroup::OVER_30});

bool AgeGroup::operator==(const AgeGroup& rhs) const {
    return val_ == rhs.val_;
}

bool AgeGroup::operator!=(const AgeGroup& rhs) const {
    return val_ != rhs.val_;
}

AgeGroup::AgeGroup(const Value& val, const std::string& string_val) :
        val_(val), string_val_(string_val) {
}

std::string AgeGroup::stringValue() const {
    return string_val_;
}

const std::vector<AgeGroup>& AgeGroup::values() {
    return AgeGroup::values_;
}

const AgeGroup::Value AgeGroup::value() const {
    return val_;
}

AgeGroup AgeGroup::valueOf(const std::string& string_val) {
    for (auto& ag : values()) {
        if (boost::iequals(string_val, ag.stringValue())) {
            return ag;
        }
    }

    throw std::invalid_argument("Unknown age group type: " + string_val);
}

AgeGroup AgeGroup::getAgeGroup(double age) {
    return (age <= 30)? AgeGroup::LEQ_30 : AgeGroup::OVER_30;
}

std::ostream& operator<<(std::ostream& out, const AgeGroup& val) {
    out << val.stringValue();
    return out;
}

} /* namespace hepcep */
