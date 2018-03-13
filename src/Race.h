/*
 * Race.h
 *
 *  Created on: Mar 12, 2018
 *      Author: nick
 */

#ifndef SRC_RACE_H_
#define SRC_RACE_H_

#include <string>
#include <vector>
#include <ostream>

namespace hepcep {

class Race {

public:
    enum Value {
        NHWhite, NHBlack, Hispanic, Other
    };

    static const Race NH_WHITE;
    static const Race NH_BLACK;
    static const Race HISPANIC;
    static const Race OTHER;

    bool operator==(const Race& rhs) const;
    bool operator!=(const Race& rhs) const;
    std::string stringValue() const;

    const Race::Value value() const;

    static const std::vector<Race>& values();

    static Race valueOf(const std::string& string_val);

private:

    Value val_;
    std::string string_val_;
    static const std::vector<Race> values_;

    Race(const Value& val, const std::string& string_val);
};

std::ostream& operator<<(std::ostream& out, const Race& val);

} /* namespace hepcep */

#endif /* SRC_RACE_H_ */
