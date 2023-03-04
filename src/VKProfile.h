

#ifndef SRC_VKPROFILE_H_
#define SRC_VKPROFILE_H_

#include <string>
#include <vector>
#include <ostream>

namespace hepcep {

class VKProfile {

public:
    enum Value {
        acute_infection_clearance, acute_infection_incomplete, acute_infection_persistence,
        reinfect_high_clearance, reinfect_low_clearance, reinfect_chronic, none
    };

    static const VKProfile ACUTE_INFECTION_CLEARANCE;    // Acute self-clearance
    static const VKProfile ACUTE_INFECTION_INCOMPLETE;   // Chronic type 1
    static const VKProfile ACUTE_INFECTION_PERSISTENCE;  // Chronic type 2
    static const VKProfile REINFECT_HIGH_CLEARANCE;      // Reinfection with self-clearance w high titer
    static const VKProfile REINFECT_LOW_CLEARANCE;       // Reinfection with self-clearance w Low titer
    static const VKProfile REINFECT_CHRONIC;             // Reinfection with chronic
    static const VKProfile NONE;

    bool operator==(const VKProfile& rhs) const;
    bool operator!=(const VKProfile& rhs) const;
    std::string stringValue() const;

    const VKProfile::Value value() const;

    static const std::vector<VKProfile>& values();

    static VKProfile valueOf(const std::string& string_val);

private:

    Value val_;
    std::string string_val_;
    static const std::vector<VKProfile> values_;

    VKProfile(const Value& val, const std::string& string_val);

};

std::ostream& operator<<(std::ostream& out, const VKProfile& val);


}

#endif /* SRC_VKPROFILE_H_ */