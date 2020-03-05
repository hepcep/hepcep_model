/*
 * LogType.h
 *
 *  Created on: Mar 14, 2018
 *      Author: nick
 */

#ifndef SRC_LOGTYPE_H_
#define SRC_LOGTYPE_H_

#include <string>
#include <vector>
#include <ostream>

namespace hepcep {

class LogType {

public:
    enum Value {
        activated,
        exposed,
        infected,
        infectious,
        chronic,
        recovered,
        deactivated,
        info,
        status,
        started_treatment,
        started_opioid_treatment,
        cured,
        regular_status,
        failed_treatment,
				hcvrna_test
    };

    static const LogType ACTIVATED;
    static const LogType EXPOSED;
    static const LogType INFECTED;
    static const LogType INFECTIOUS;
    static const LogType CHRONIC;
    static const LogType RECOVERED;
    static const LogType DEACTIVATED;
    static const LogType INFO;
    static const LogType STATUS;
    static const LogType STARTED_TREATMENT;
    static const LogType STARTED_OPIOID_TREATMENT;
    static const LogType CURED;
    static const LogType REGULAR_STATUS;
    static const LogType FAILED_TREATMENT;
    static const LogType HCVRNA_TEST;

    bool operator==(const LogType& rhs) const;
    bool operator!=(const LogType& rhs) const;
    bool operator<(const LogType& rhs) const;
    
    std::string stringValue() const;

    const LogType::Value value() const;

    static const std::vector<LogType>& values();

    static LogType valueOf(const std::string& string_val);


private:

    Value val_;
    std::string string_val_;
    static const std::vector<LogType> values_;

    LogType(const Value& val, const std::string& string_val);
};

std::ostream& operator<<(std::ostream& out, const LogType& val);


} /* namespace hepcep */

#endif /* SRC_LOGTYPE_H_ */
