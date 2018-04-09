/*
 * Distributions.cpp
 *
 *  Created on: Mar 31, 2018
 *      Author: eric
 */

#include <iostream>

#include <Distributions.h>

namespace hepcep {

Distributions* Distributions::instance_ = nullptr;

Distributions::Distributions(double attritionRate, double meanEdgeLifetime, double meanCareerDuration):
				lifespanGenerator{repast::Random::instance()->createExponentialGenerator(attritionRate/365.0)},
				networkLifespanGenerator{repast::Random::instance()->createExponentialGenerator((1.0/meanEdgeLifetime)/365.0)},
				cessationTimeGenerator{repast::Random::instance()->createNormalGenerator(meanCareerDuration*365.0, meanCareerDuration*365.0/3.0)},
				shuffleGenerator{repast::Random::instance()->createUniDoubleGenerator(0,1)}
				{

}

Distributions::~Distributions() {
}

Distributions* Distributions::instance(){
	if (instance_ == nullptr) {
		throw std::domain_error("Distributions must be initialized before use");
	}
	return instance_;
}

void Distributions::init(double attritionRate, double meanEdgeLifetime,
		double meanCareerDuration ){

	instance_ = new Distributions(attritionRate, meanEdgeLifetime, meanCareerDuration);
}

double Distributions::getLifespanRandom(){

	return lifespanGenerator.next();
}

double Distributions::getNetworkLifespanRandom(){
	return networkLifespanGenerator.next();
}

double Distributions::getCessationTimeRandom(){
	return cessationTimeGenerator.next();
}

repast::DoubleUniformGenerator Distributions::getShuffleGenerator(){
	return shuffleGenerator;
}

} // namespace hepcep
