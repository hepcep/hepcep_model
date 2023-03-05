/*
 * LogType.cpp
 *
 *  Created on: Mar 14, 2018
 *      Author: nick
 */
#include <exception>

#include "boost/algorithm/string/predicate.hpp"

#include "LogType.h"

namespace hepcep {

const LogType LogType::ACTIVATED(LogType::activated, "ACTIVATED");
const LogType LogType::EXPOSED(LogType::exposed, "EXPOSED");
const LogType LogType::INFECTED(LogType::infected, "INFECTED");
const LogType LogType::INFECTIOUS(LogType::infectious, "INFECTIOUS");
const LogType LogType::CHRONIC(LogType::chronic, "CHRONIC");
const LogType LogType::RECOVERED(LogType::recovered, "RECOVERED");
const LogType LogType::DEACTIVATED(LogType::deactivated, "DEACTIVATED");
const LogType LogType::INFO(LogType::info, "INFO");
const LogType LogType::STATUS(LogType::status, "STATUS");
const LogType LogType::STARTED_TREATMENT(LogType::started_treatment, "STARTED_TREATMENT");
const LogType LogType::STARTED_OPIOID_TREATMENT(LogType::started_opioid_treatment, "STARTED_OPIOID_TREATMENT");
const LogType LogType::STOPPED_OPIOID_TREATMENT(LogType::stopped_opioid_treatment, "STOPPED_OPIOID_TREATMENT");
const LogType LogType::CURED(LogType::cured, "CURED");
const LogType LogType::REGULAR_STATUS(LogType::regular_status, "REGULAR_STATUS");
const LogType LogType::FAILED_TREATMENT(LogType::failed_treatment, "FAILED_TREATMENT");
const LogType LogType::HCVRNA_TEST(LogType::hcvrna_test, "HCVRNA_TEST");
const LogType LogType::VK_PROFILE(LogType::vk_profile, "VK_PROFILE");

const std::vector<LogType> LogType::values_({LogType::ACTIVATED,
    LogType::EXPOSED,
    LogType::INFECTED,
    LogType::INFECTIOUS,
    LogType::CHRONIC,
    LogType::RECOVERED,
    LogType::DEACTIVATED,
    LogType::INFO,
    LogType::STATUS,
    LogType::STARTED_TREATMENT,
    LogType::STARTED_OPIOID_TREATMENT,
    LogType::STOPPED_OPIOID_TREATMENT,
    LogType::CURED,
    LogType::REGULAR_STATUS,
    LogType::FAILED_TREATMENT,
	LogType::HCVRNA_TEST,
    LogType::VK_PROFILE});

bool LogType::operator==(const LogType& rhs) const {
    return val_ == rhs.val_;
}

bool LogType::operator!=(const LogType& rhs) const {
    return val_ != rhs.val_;
}

bool LogType::operator<(const LogType& rhs) const {
    return val_ < rhs.val_;
}

LogType::LogType(const Value& val, const std::string& string_val) :
        val_(val), string_val_(string_val) {
}

std::string LogType::stringValue() const {
    return string_val_;
}

const std::vector<LogType>& LogType::values() {
    return LogType::values_;
}

const LogType::Value LogType::value() const {
    return val_;
}

LogType LogType::valueOf(const std::string& string_val) {
    for (auto& log_type : values()) {
        if (boost::iequals(string_val, log_type.stringValue())) {
            return log_type;
        }
    }

    throw std::invalid_argument("Unknown log type: " + string_val);
}

std::ostream& operator<<(std::ostream& out, const LogType& val) {
    out << val.stringValue();
    return out;
}

} /* namespace hepcep */
