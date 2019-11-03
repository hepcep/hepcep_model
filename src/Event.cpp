/*
 * Event.cpp
 *
 *  Created on: Mar 2, 2018
 *      Author: nick
 */

#include "Event.h"

namespace hepcep {


Event::Event(double scheduled_for, EventFuncType ef_type, repast::Functor* func) : scheduled_for_(scheduled_for),
    ef_type_(ef_type), func_(func), canceled(false) { }

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

double Event::scheduled_for() const {
    return scheduled_for_;
}

}


