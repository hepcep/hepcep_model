
#include <exception>

#include "boost/algorithm/string/predicate.hpp"

#include "VKProfile.h"

namespace hepcep {

const VKProfile VKProfile::ACUTE_INFECTION_CLEARANCE(VKProfile::acute_infection_clearance, "acute_infection_clearance");
const VKProfile VKProfile::ACUTE_INFECTION_INCOMPLETE(VKProfile::acute_infection_incomplete, "acute_infection_incomplete");
const VKProfile VKProfile::ACUTE_INFECTION_PERSISTENCE(VKProfile::acute_infection_persistence, "acute_infection_persistence");
const VKProfile VKProfile::REINFECT_HIGH_CLEARANCE(VKProfile::reinfect_high_clearance, "reinfect_high_clearance");
const VKProfile VKProfile::REINFECT_LOW_CLEARANCE(VKProfile::reinfect_low_clearance, "reinfect_low_clearance");
const VKProfile VKProfile::REINFECT_CHRONIC(VKProfile::reinfect_chronic, "reinfect_chronic");
const VKProfile VKProfile::NONE(VKProfile::none, "none");
const VKProfile VKProfile::TREATMENT(VKProfile::treatment, "treatment");

const std::vector<VKProfile> VKProfile::values_({ VKProfile::ACUTE_INFECTION_CLEARANCE, VKProfile::ACUTE_INFECTION_INCOMPLETE,
        VKProfile::ACUTE_INFECTION_PERSISTENCE, VKProfile::REINFECT_HIGH_CLEARANCE, VKProfile::REINFECT_LOW_CLEARANCE, 
        VKProfile::REINFECT_CHRONIC, VKProfile::NONE, VKProfile::TREATMENT });

bool VKProfile::operator==(const VKProfile& rhs) const {
    return val_ == rhs.val_;
}

bool VKProfile::operator!=(const VKProfile& rhs) const {
    return val_ != rhs.val_;
}

VKProfile::VKProfile(const Value& val, const std::string& string_val) :
        val_(val), string_val_(string_val) {
}

const std::string VKProfile::stringValue() const {
    return string_val_;
}

const std::vector<VKProfile>& VKProfile::values() {
    return VKProfile::values_;
}

const VKProfile::Value VKProfile::value() const {
    return val_;
}

VKProfile VKProfile::valueOf(const std::string& string_val) {
    for (auto& hcv : values()) {
        if (boost::iequals(string_val, hcv.stringValue())) {
            return hcv;
        }
    }

    throw std::invalid_argument("Unknown hcv type: " + string_val);
}

std::ostream& operator<<(std::ostream& out, const VKProfile& val) {
    out << val.stringValue();
    return out;
}


}

