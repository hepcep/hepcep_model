/*
 * Distributions.h
 *
 *  Created on: Mar 31, 2018
 *      Author: eric
 */

#ifndef SRC_DISTRIBUTIONS_H_
#define SRC_DISTRIBUTIONS_H_

#include "repast_hpc/Random.h"

namespace hepcep {


class Distributions {

private:
	static Distributions* instance_;
	Distributions(double attritionRate,double meanEdgeLifetime,double meanCareerDuration);

	repast::ExponentialGenerator lifespanGenerator;
	repast::ExponentialGenerator networkLifespanGenerator;
	repast::NormalGenerator cessationTimeGenerator;


public:
	static Distributions* instance();
	static void init(double attritionRate,double meanEdgeLifetime,double meanCareerDuration);

	virtual ~Distributions();

	double getLifespanRandom();
	double getNetworkLifespanRandom();
	double getCessationTimeRandom();

};

} /* namespace hepcep */

#endif /* SRC_DISTRIBUTIONS_H_ */
