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

const HCVState HCVState::SUSCEPTIBLE(HCVState::susceptible, "SUSCEPTIBLE");
const HCVState HCVState::EXPOSED(HCVState::exposed, "EXPOSED");
const HCVState HCVState::INFECTIOUS_ACUTE(HCVState::infectious_acute, "INFECTIOUS_ACUTE");
const HCVState HCVState::RECOVERED(HCVState::recovered, "RECOVERED");
const HCVState HCVState::CURED(HCVState::cured, "CURED");
const HCVState HCVState::CHRONIC(HCVState::chronic, "CHRONIC");
const HCVState HCVState::UNKNOWN(HCVState::unknown, "UNKNOWN");
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

/*
 std::string hcv_state_to_string(const HCV_State state) {
 switch (state) {
 case HCV_State::susceptible:
 return "susceptible";
 case HCV_State::exposed:
 return "exposed";
 case HCV_State::infectiousacute:
 return "infectiousacute";
 case HCV_State::recovered:
 return "recovered";
 case HCV_State::cured:
 return "cured";
 case HCV_State::chronic:
 return "chronic";
 case HCV_State::ABPOS:
 return "ABPOS";
 case HCV_State::unknown:
 return "unknown";
 }
 return "unknown";
 }

 HCV_State string_to_hcv_state(const std::string& str) {

 if (str.empty())
 return HCV_State::unknown;

 else if (str == "ABPOS")
 return HCV_State::ABPOS;
 else if (str == "susceptible")
 return HCV_State::susceptible;

 else if (str == "exposed")
 return HCV_State::exposed;
 else if (str == "infectiousacute")
 return HCV_State::infectiousacute;
 else if (str == "recovered")
 return HCV_State::recovered;
 else if (str == "cured")
 return HCV_State::cured;
 else if (str == "chronic")
 return HCV_State::chronic;

 return HCV_State::unknown;
 }

 const std::string HCV_STATES[] = {hcv_state_to_string(HCV_State::susceptible), hcv_state_to_string(HCV_State::exposed), hcv_state_to_string(HCV_State::infectiousacute),
 hcv_state_to_string(HCV_State::recovered), hcv_state_to_string(HCV_State::cured), hcv_state_to_string(HCV_State::chronic),
 hcv_state_to_string(HCV_State::ABPOS)};
 */

}

