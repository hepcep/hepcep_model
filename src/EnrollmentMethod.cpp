/*
 * EnrollmentMethod.cpp
 *
 *	Enum class for treatment enrollment methods.
 *
 * Author: Eric Tatara
 */

#include <exception>

#include "boost/algorithm/string/predicate.hpp"

#include <EnrollmentMethod.h>

namespace hepcep {

const EnrollmentMethod EnrollmentMethod::UNBIASED(EnrollmentMethod::unbiased, "unbiased");
const EnrollmentMethod EnrollmentMethod::HRP(EnrollmentMethod::hrp, "hrp");
const EnrollmentMethod EnrollmentMethod::FULLNETWORK(EnrollmentMethod::fullnetwork, "fullnetwork");
const EnrollmentMethod EnrollmentMethod::INPARTNER(EnrollmentMethod::inpartner, "inpartner");
const EnrollmentMethod EnrollmentMethod::OUTPARTNER(EnrollmentMethod::outpartner, "outpartner");


const std::vector<EnrollmentMethod> EnrollmentMethod::values_({
	EnrollmentMethod::UNBIASED,
	EnrollmentMethod::HRP,
	EnrollmentMethod::FULLNETWORK,
	EnrollmentMethod::INPARTNER,
	EnrollmentMethod::OUTPARTNER});

bool EnrollmentMethod::operator==(const EnrollmentMethod& rhs) const {
    return val_ == rhs.val_;
}

bool EnrollmentMethod::operator!=(const EnrollmentMethod& rhs) const {
    return val_ != rhs.val_;
}

bool EnrollmentMethod::operator<(const EnrollmentMethod& rhs) const{
	return val_ < rhs.val_;
}

EnrollmentMethod::EnrollmentMethod(const Value& val, const std::string& string_val) :
        val_(val), string_val_(string_val) {
}

std::string EnrollmentMethod::stringValue() const {
    return string_val_;
}

const std::vector<EnrollmentMethod>& EnrollmentMethod::values() {
    return EnrollmentMethod::values_;
}

const EnrollmentMethod::Value EnrollmentMethod::value() const {
    return val_;
}

EnrollmentMethod EnrollmentMethod::valueOf(const std::string& string_val) {
    for (auto& val : values()) {
        if (boost::iequals(string_val, val.stringValue())) {
            return val;
        }
    }

    throw std::invalid_argument("Unknown EnrollmentMethod type: " + string_val);
}

std::ostream& operator<<(std::ostream& out, const EnrollmentMethod& val) {
    out << val.stringValue();
    return out;
}

} /* namespace hepcep */
