/*
 * Gender.h
 *
 *  Created on: Mar 12, 2018
 *      Author: nick
 */

#ifndef SRC_GENDER_H_
#define SRC_GENDER_H_

#include <string>
#include <vector>

namespace hepcep {

class Gender {

public:
    enum Value {male, female};

    static const Gender MALE;
    static const Gender FEMALE;

        bool operator==(const Gender& rhs) const;
        bool operator!=(const Gender& rhs) const;
        std::string stringValue() const;

        const Gender::Value value() const;

        static const std::vector<Gender>& values();

        static Gender valueOf(const std::string& string_val);

    private:

        Value val_;
        std::string string_val_;
        static const std::vector<Gender> values_;

        Gender(const Value& val, const std::string& string_val);
};

} /* namespace hepcep */

#endif /* SRC_GENDER_H_ */
