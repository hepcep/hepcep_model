/*
 * HCV_State.h
 *
 *  Created on: Mar 2, 2018
 *      Author: nick
 */

#ifndef SRC_HCVSTATE_H_
#define SRC_HCVSTATE_H_

#include <string>
#include <vector>
#include <ostream>

namespace hepcep {

class HCVState {

public:
    enum Value {
        susceptible, exposed, infectious_acute, recovered, cured, chronic, unknown, abpos
    };

    static const HCVState SUSCEPTIBLE;
    static const HCVState EXPOSED;
    static const HCVState INFECTIOUS_ACUTE;
    static const HCVState RECOVERED;
    static const HCVState CURED;
    static const HCVState CHRONIC;
    static const HCVState UNKNOWN;
    static const HCVState ABPOS;

    bool operator==(const HCVState& rhs) const;
    bool operator!=(const HCVState& rhs) const;
    std::string stringValue() const;

    const HCVState::Value value() const;

    static const std::vector<HCVState>& values();

    static HCVState valueOf(const std::string& string_val);

private:

    Value val_;
    std::string string_val_;
    static const std::vector<HCVState> values_;

    HCVState(const Value& val, const std::string& string_val);

};

std::ostream& operator<<(std::ostream& out, const HCVState& val);


}

#endif /* SRC_HCVSTATE_H_ */
