/*
 * AreaType.h
 *
 *  Created on: Mar 13, 2018
 *      Author: nick
 */

#ifndef SRC_AREATYPE_H_
#define SRC_AREATYPE_H_

#include <string>
#include <vector>
#include <ostream>

namespace hepcep {

class AreaType {

public:
    enum Value {
        City, Suburban
    };

    static const AreaType CITY;
    static const AreaType SUBURBAN;

    bool operator==(const AreaType& rhs) const;
    bool operator!=(const AreaType& rhs) const;
    std::string stringValue() const;

    const AreaType::Value value() const;

    static const std::vector<AreaType>& values();

    static AreaType valueOf(const std::string& string_val);
    static AreaType getAreaType(const std::string& zip_code);

private:

    Value val_;
    std::string string_val_;
    static const std::vector<AreaType> values_;

    AreaType(const Value& val, const std::string& string_val);
};

std::ostream& operator<<(std::ostream& out, const AreaType& val);

} /* namespace hepcep */

#endif /* SRC_AREATYPE_H_ */
