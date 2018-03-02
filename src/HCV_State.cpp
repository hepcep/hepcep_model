/*
 * HCV_State.cpp
 *
 *  Created on: Mar 2, 2018
 *      Author: nick
 */

#include "HCV_State.h"

namespace hepcep {

std::string hcv_state_to_string(const HCV_State state) {
    switch (state) {
    case HCV_State::susceptible:
        return "susceptible";
    case HCV_State::exposed:
        return "exposed";
    case HCV_State::infectiousacute:
        return "infectiousacute";
    case HCV_State::recovered:
        return "recovered";
    case HCV_State::cured:
        return "cured";
    case HCV_State::chronic:
        return "chronic";
    case HCV_State::ABPOS:
        return "ABPOS";
    case HCV_State::unknown:
        return "unknown";
    }
    return "unknown";
}

HCV_State string_to_hcv_state(const std::string& str) {

    if (str.empty())
        return HCV_State::unknown;

    else if (str == "ABPOS")
        return HCV_State::ABPOS;
    else if (str == "susceptible")
        return HCV_State::susceptible;

    else if (str == "exposed")
        return HCV_State::exposed;
    else if (str == "infectiousacute")
        return HCV_State::infectiousacute;
    else if (str == "recovered")
        return HCV_State::recovered;
    else if (str == "cured")
        return HCV_State::cured;
    else if (str == "chronic")
        return HCV_State::chronic;

    return HCV_State::unknown;
}

}

