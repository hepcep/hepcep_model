/*
 * Gender.cpp
 *
 *  Created on: Mar 12, 2018
 *      Author: nick
 */

#include <exception>

#include "boost/algorithm/string/predicate.hpp"

#include "Gender.h"

namespace hepcep {

const Gender Gender::MALE(Gender::male, "MALE");
const Gender Gender::FEMALE(Gender::female, "FEMALE");

const std::vector<Gender> Gender::values_({ Gender::MALE, Gender::FEMALE});

bool Gender::operator==(const Gender& rhs) const {
    return val_ == rhs.val_;
}

bool Gender::operator!=(const Gender& rhs) const {
    return val_ != rhs.val_;
}

Gender::Gender(const Value& val, const std::string& string_val) :
        val_(val), string_val_(string_val) {
}

std::string Gender::stringValue() const {
    return string_val_;
}

const std::vector<Gender>& Gender::values() {
    return Gender::values_;
}

const Gender::Value Gender::value() const {
    return val_;
}

Gender Gender::valueOf(const std::string& string_val) {
    for (auto& gender : values()) {
        if (boost::iequals(string_val, gender.stringValue())) {
            return gender;
        }
    }

    throw std::invalid_argument("Unknown gender type: " + string_val);
}

std::ostream& operator<<(std::ostream& out, const Gender& val) {
    out << val.stringValue();
    return out;
}

} /* namespace hepcep */
