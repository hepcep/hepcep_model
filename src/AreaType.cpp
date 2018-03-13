/*
 * AreaType.cpp
 *
 *  Created on: Mar 13, 2018
 *      Author: nick
 */

#include <exception>

#include "boost/algorithm/string/predicate.hpp"


#include "AreaType.h"

namespace hepcep {

const AreaType AreaType::CITY(AreaType::City, "CITY");
const AreaType AreaType::SUBURBAN(AreaType::Suburban, "SUBURBAN");

const std::vector<AreaType> AreaType::values_({ AreaType::CITY, AreaType::SUBURBAN});

bool AreaType::operator==(const AreaType& rhs) const {
    return val_ == rhs.val_;
}

bool AreaType::operator!=(const AreaType& rhs) const {
    return val_ != rhs.val_;
}

AreaType::AreaType(const Value& val, const std::string& string_val) :
        val_(val), string_val_(string_val) {
}

std::string AreaType::stringValue() const {
    return string_val_;
}

const std::vector<AreaType>& AreaType::values() {
    return AreaType::values_;
}

const AreaType::Value AreaType::value() const {
    return val_;
}

AreaType AreaType::valueOf(const std::string& string_val) {
    for (auto& at : values()) {
        if (boost::iequals(string_val, at.stringValue())) {
            return at;
        }
    }

    throw std::invalid_argument("Unknown area type: " + string_val);
}

AreaType AreaType::getAreaType(const std::string& zip_code) {
    if (zip_code.length() > 3 && zip_code.substr(0, 3) == "606") {
        return AreaType::CITY;
    }
    return AreaType::SUBURBAN;
}

std::ostream& operator<<(std::ostream& out, const AreaType& val) {
    out << val.stringValue();
    return out;
}


} /* namespace hepcep */
