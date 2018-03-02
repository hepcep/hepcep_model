/*
 * Event.cpp
 *
 *  Created on: Mar 2, 2018
 *      Author: nick
 */

#include "Event.h"

namespace hepcep {


Event::Event(repast::Functor* func) : func_(func), canceled(false) { }

Event::~Event(){
    delete func_;
}

void Event::cancel() {
    canceled = true;
}

void Event::operator()() {
    if (!canceled) {
        (*func_)();
    }
}


}


