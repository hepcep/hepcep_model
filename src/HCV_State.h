/*
 * HCV_State.h
 *
 *  Created on: Mar 2, 2018
 *      Author: nick
 */

#ifndef SRC_HCV_STATE_H_
#define SRC_HCV_STATE_H_

#include <string>

namespace hepcep {

enum class HCV_State{susceptible, exposed, infectiousacute, recovered, cured, chronic, unknown, ABPOS};

std::string hcv_state_to_string(const HCV_State state);

HCV_State string_to_hcv_state(const std::string& str);

}




#endif /* SRC_HCV_STATE_H_ */
