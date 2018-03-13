/*
 * AgeDecade.h
 *
 *  Created on: Mar 12, 2018
 *      Author: nick
 */

#ifndef SRC_AGEDECADE_H_
#define SRC_AGEDECADE_H_

#include <string>
#include <vector>

namespace hepcep {

class AgeDecade {

public:
    enum Value {
        AgeLEQ20, Age21to30, Age31to40, Age41to50, Age51to60, AgeOver60
    };

    static const AgeDecade AGE_LEQ_20;
    static const AgeDecade AGE_21_30;
    static const AgeDecade AGE_31_40;
    static const AgeDecade AGE_41_50;
    static const AgeDecade AGE_51_60;
    static const AgeDecade AGE_OVER_60;

    bool operator==(const AgeDecade& rhs) const;
    bool operator!=(const AgeDecade& rhs) const;
    std::string stringValue() const;

    const AgeDecade::Value value() const;

    static const std::vector<AgeDecade>& values();

    static AgeDecade valueOf(const std::string& string_val);
    static AgeDecade getAgeDecade(double age);

private:

    Value val_;
    std::string string_val_;
    static const std::vector<AgeDecade> values_;

    AgeDecade(const Value& val, const std::string& string_val);

};

} /* namespace hepcep */

#endif /* SRC_AGEDECADE_H_ */
