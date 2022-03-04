/*
 * @file ViralKintecsLoader.cpp
 * Loader for viral kinetics files
 *
 * @author Eric Tatara
 */
#include <exception>

#include "ViralKineticsLoader.h"
#include "SVReader.h"

namespace hepcep {

/**
 * Creates a map viral load to transmission probability
 */
void loadTransmissionProbabilities(const std::string& filename, std::map<double, 
		double> & transmit_prob_map) {

	SVReader reader(filename, ',');
	std::vector<std::string> line;

	// Header
	reader.next(line);

	// Each row represents the mapping between the viral load and transmission probability
	while (reader.next(line)) {
		double viral_load = std::stod(line[0]);
		double transmit_prob = std::stod(line[1]);

		transmit_prob_map[viral_load] = transmit_prob;
	}
}

}
