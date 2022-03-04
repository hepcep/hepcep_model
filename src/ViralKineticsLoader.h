/*
 * ViralKineticsLoader.h
 *
 *  
 */

#ifndef SRC_VIRALKINETICSLOADER_H_
#define SRC_VIRALKINETICSLOADER_H_

#include <map>
#include <string>

namespace hepcep {


/**
 * Load zones data from the specified file
 */
void loadTransmissionProbabilities(const std::string& filename, std::map<double, double> & transmit_prob_map);


}

#endif /* SRC_VIRALKINETICSLOADER_H_ */
