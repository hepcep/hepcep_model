/*
 * Distributions.h
 *
 *	Holds statistical distributions for convienient access.
 *
 *	TODO might be better to use store re-used generators in the Repast
 *				random instance via repast::Random::instance()->putGenerator()
 *
 *      Author: Eric Tatara
 */

#ifndef SRC_DISTRIBUTIONS_H_
#define SRC_DISTRIBUTIONS_H_

#include "boost/random.hpp"
#include "repast_hpc/Random.h"
#include <boost/random/discrete_distribution.hpp>

namespace hepcep {

using PoissonGen = boost::variate_generator<boost::mt19937&, boost::poisson_distribution<>>;
using DiscreteGen = boost::variate_generator<boost::mt19937&, boost::random::discrete_distribution<>>;

class Distributions {

private:
	static Distributions* instance_;
	Distributions(double attritionRate,double meanEdgeLifetime,double meanCareerDuration);

	repast::ExponentialGenerator lifespanGenerator;
	repast::ExponentialGenerator networkLifespanGenerator;
	repast::NormalGenerator cessationTimeGenerator;

	repast::DoubleUniformGenerator shuffleGenerator;  // used to shuffle collections


public:
	static Distributions* instance();
	static void init(double attritionRate,double meanEdgeLifetime,double meanCareerDuration);

	virtual ~Distributions();

	double getLifespanRandom();
	double getNetworkLifespanRandom();
	double getCessationTimeRandom();

	repast::DoubleUniformGenerator getShuffleGenerator();

};

} /* namespace hepcep */

#endif /* SRC_DISTRIBUTIONS_H_ */
