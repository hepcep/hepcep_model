/*
 * EndRelationshipFunctor.cpp
 *
 *  Created on: Apr 2, 2018
 *      Author: eric
 */

#include <EndRelationshipFunctor.h>

namespace hepcep {

// TODO this Functor may be holding on to the PersonPtrs after Persons become inactive.

EndRelationshipFunctor::EndRelationshipFunctor(PersonPtr source, PersonPtr other, NetworkPtr<HCPerson> net) :
	sourcePerson(source), buddy(other), network(net){

}

EndRelationshipFunctor::~EndRelationshipFunctor() {

}

void EndRelationshipFunctor::operator()(){
	sourcePerson->endRelationship(buddy,network);
}

} /* namespace hepcep */
