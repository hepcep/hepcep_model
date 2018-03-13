/*
 * AgeGroup.h
 *
 *  Created on: Mar 13, 2018
 *      Author: nick
 */

#ifndef SRC_AGEGROUP_H_
#define SRC_AGEGROUP_H_

#include <string>
#include <vector>

namespace hepcep {

class AgeGroup {

public:
    enum Value {
        LEQ30, Over30
    };

    static const AgeGroup LEQ_30;
    static const AgeGroup OVER_30;

    bool operator==(const AgeGroup& rhs) const;
    bool operator!=(const AgeGroup& rhs) const;
    std::string stringValue() const;

    const AgeGroup::Value value() const;

    static const std::vector<AgeGroup>& values();

    static AgeGroup valueOf(const std::string& string_val);
    static AgeGroup getAgeGroup(double age);

private:

    Value val_;
    std::string string_val_;
    static const std::vector<AgeGroup> values_;

    AgeGroup(const Value& val, const std::string& string_val);
};

} /* namespace hepcep */

#endif /* SRC_AGEGROUP_H_ */
