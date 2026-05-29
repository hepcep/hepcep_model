/*
 * EndHarmReductionFunctor.cpp
 *
 * Used to schedule the end of DAA-related harm reduction behvaior.
 */

#include "EndHarmReductionFunctor.h"

namespace hepcep {

EndHarmReductionFunctor::EndHarmReductionFunctor(HCPerson* person, double intensity) : person(person), inject_intensity(intensity)  {
}

EndHarmReductionFunctor::~EndHarmReductionFunctor() {
}

void EndHarmReductionFunctor::operator()() {

	// Reset the person's injection intensity back to the orginal value prior to starting harm reduction.
	person->setInjectionIntensity(inject_intensity);
}

} /* namespace hepcep */
