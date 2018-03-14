/*
 * HCV_State.cpp
 *
 *  Created on: Mar 2, 2018
 *      Author: nick
 */

#include <exception>

#include "boost/algorithm/string/predicate.hpp"

#include "HCVState.h"

namespace hepcep {

const HCVState HCVState::SUSCEPTIBLE(HCVState::susceptible, "susceptible");
const HCVState HCVState::EXPOSED(HCVState::exposed, "exposed");
const HCVState HCVState::INFECTIOUS_ACUTE(HCVState::infectious_acute, "infectiousacute");
const HCVState HCVState::RECOVERED(HCVState::recovered, "recovered");
const HCVState HCVState::CURED(HCVState::cured, "cured");
const HCVState HCVState::CHRONIC(HCVState::chronic, "chronic");
const HCVState HCVState::UNKNOWN(HCVState::unknown, "unknown");
const HCVState HCVState::ABPOS(HCVState::abpos, "ABPOS");

const std::vector<HCVState> HCVState::values_({ HCVState::SUSCEPTIBLE, HCVState::EXPOSED,
        HCVState::INFECTIOUS_ACUTE, HCVState::RECOVERED, HCVState::CURED, HCVState::CHRONIC,
        HCVState::UNKNOWN, HCVState::ABPOS });

bool HCVState::operator==(const HCVState& rhs) const {
    return val_ == rhs.val_;
}

bool HCVState::operator!=(const HCVState& rhs) const {
    return val_ != rhs.val_;
}

HCVState::HCVState(const Value& val, const std::string& string_val) :
        val_(val), string_val_(string_val) {
}

std::string HCVState::stringValue() const {
    return string_val_;
}

const std::vector<HCVState>& HCVState::values() {
    return HCVState::values_;
}

const HCVState::Value HCVState::value() const {
    return val_;
}

HCVState HCVState::valueOf(const std::string& string_val) {
    for (auto& hcv : values()) {
        if (boost::iequals(string_val, hcv.stringValue())) {
            return hcv;
        }
    }

    throw std::invalid_argument("Unknown hcv type: " + string_val);
}

std::ostream& operator<<(std::ostream& out, const HCVState& val) {
    out << val.stringValue();
    return out;
}


}

