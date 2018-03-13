/*
 * HarmReduction.cpp
 *
 *  Created on: Mar 12, 2018
 *      Author: nick
 */

#include <exception>

#include "boost/algorithm/string/predicate.hpp"

#include "HarmReduction.h"

namespace hepcep {

const HarmReduction HarmReduction::HARM_REDUCTION(HarmReduction::HR, "HR");
const HarmReduction HarmReduction::NON_HARM_REDUCTION(HarmReduction::nonHR, "nonHR");

const std::vector<HarmReduction> HarmReduction::values_({ HarmReduction::HARM_REDUCTION, HarmReduction::NON_HARM_REDUCTION});

bool HarmReduction::operator==(const HarmReduction& rhs) const {
    return val_ == rhs.val_;
}

bool HarmReduction::operator!=(const HarmReduction& rhs) const {
    return val_ != rhs.val_;
}

HarmReduction::HarmReduction(const Value& val, const std::string& string_val) :
        val_(val), string_val_(string_val) {
}

std::string HarmReduction::stringValue() const {
    return string_val_;
}

const std::vector<HarmReduction>& HarmReduction::values() {
    return HarmReduction::values_;
}

const HarmReduction::Value HarmReduction::value() const {
    return val_;
}

HarmReduction HarmReduction::valueOf(const std::string& string_val) {
    for (auto& harm : values()) {
        if (boost::iequals(string_val, harm.stringValue())) {
            return harm;
        }
    }

    throw std::invalid_argument("Unknown harm reduction value: " + string_val);
}

std::ostream& operator<<(std::ostream& out, const HarmReduction& val) {
    out << val.stringValue();
    return out;
}

} /* namespace hepcep */
