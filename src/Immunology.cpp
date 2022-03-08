/**
 * @file Immunology.cpp
 * 
 * Immunology abstract base class.  Implementing subclasses should
 * define the infection mechanics.
 * 
 */

#include "Immunology.h"

namespace hepcep {

// Constructor
Immunology::Immunology(HCPerson* idu) : idu_(idu){

}

// void Immunology::leaveExposed() {
//     hcv_state = HCVState::INFECTIOUS_ACUTE;
//     Statistics::instance()->logStatusChange(LogType::INFECTIOUS, idu_, "");
// }

}