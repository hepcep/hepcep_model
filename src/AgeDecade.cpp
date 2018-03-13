/*
 * AgeDecade.cpp
 *
 *  Created on: Mar 12, 2018
 *      Author: nick
 */

#include <exception>

#include "boost/algorithm/string/predicate.hpp"

#include "AgeDecade.h"

namespace hepcep {

const AgeDecade AgeDecade::AGE_LEQ_20(AgeDecade::AgeLEQ20, "AGE_LEQ_20");
const AgeDecade AgeDecade::AGE_21_30(AgeDecade::Age21to30, "AGE_21_30");
const AgeDecade AgeDecade::AGE_31_40(AgeDecade::Age31to40, "AGE_31_40");
const AgeDecade AgeDecade::AGE_41_50(AgeDecade::Age41to50, "AGE_41_50");
const AgeDecade AgeDecade::AGE_51_60(AgeDecade::Age51to60, "AGE_51_60");
const AgeDecade AgeDecade::AGE_OVER_60(AgeDecade::AgeOver60, "AGE_OVER_60");

const std::vector<AgeDecade> AgeDecade::values_( { AgeDecade::AGE_LEQ_20, AgeDecade::AGE_21_30,
        AgeDecade::AGE_31_40, AgeDecade::AGE_41_50, AgeDecade::AGE_51_60, AgeDecade::AGE_OVER_60 });

bool AgeDecade::operator==(const AgeDecade& rhs) const {
    return val_ == rhs.val_;
}

bool AgeDecade::operator!=(const AgeDecade& rhs) const {
    return val_ != rhs.val_;
}

AgeDecade::AgeDecade(const Value& val, const std::string& string_val) :
        val_(val), string_val_(string_val) {
}

std::string AgeDecade::stringValue() const {
    return string_val_;
}

const std::vector<AgeDecade>& AgeDecade::values() {
    return AgeDecade::values_;
}

const AgeDecade::Value AgeDecade::value() const {
    return val_;
}

AgeDecade AgeDecade::valueOf(const std::string& string_val) {
    for (auto& gender : values()) {
        if (boost::iequals(string_val, gender.stringValue())) {
            return gender;
        }
    }

    throw std::invalid_argument("Unknown gender type: " + string_val);
}

AgeDecade AgeDecade::getAgeDecade(double age) {
    if (age <= 20) {
        return AgeDecade::AGE_LEQ_20;
    } else if (age <= 30) {
        return AgeDecade::AGE_21_30;
    } else if (age <= 40) {
        return AgeDecade::AGE_31_40;
    } else if (age <= 50) {
        return AgeDecade::AGE_41_50;
    } else if (age <= 60) {
        return AgeDecade::AGE_51_60;
    } else {
        return AgeDecade::AGE_OVER_60;
    }
}

std::ostream& operator<<(std::ostream& out, const AgeDecade& val) {
    out << val.stringValue();
    return out;
}

} /* namespace hepcep */
