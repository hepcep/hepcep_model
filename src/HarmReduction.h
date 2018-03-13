/*
 * HarmReduction.h
 *
 *  Created on: Mar 12, 2018
 *      Author: nick
 */

#ifndef SRC_HARMREDUCTION_H_
#define SRC_HARMREDUCTION_H_

#include <string>
#include <vector>
#include <ostream>

namespace hepcep {

class HarmReduction {

public:
    enum Value {
        HR, nonHR
    };

    static const HarmReduction HARM_REDUCTION;
    static const HarmReduction NON_HARM_REDUCTION;

    bool operator==(const HarmReduction& rhs) const;
    bool operator!=(const HarmReduction& rhs) const;
    std::string stringValue() const;

    const HarmReduction::Value value() const;

    static const std::vector<HarmReduction>& values();

    static HarmReduction valueOf(const std::string& string_val);

private:

    Value val_;
    std::string string_val_;
    static const std::vector<HarmReduction> values_;

    HarmReduction(const Value& val, const std::string& string_val);
};

std::ostream& operator<<(std::ostream& out, const HarmReduction& val);

} /* namespace hepcep */

#endif /* SRC_HARMREDUCTION_H_ */
