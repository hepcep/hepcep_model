/*
 * serialize.h
 *
 *  Created on: Nov 27, 2017
 *      Author: nick
 */

#ifndef SRC_SERIALIZE_H_
#define SRC_SERIALIZE_H_

#include <map>
#include <string>
#include <sstream>

#include "HCPerson.h"
#include "Network.h"
#include "gml.h"
#include "Zone.h"

namespace hepcep {



PersonPtr read_person(NamedListAttribute* node, std::map<unsigned int,ZonePtr>& zoneMap, double);

void read_edge(NamedListAttribute* edge_list, NetworkPtr<HCPerson>& net, 
    std::map<unsigned int, std::shared_ptr<HCPerson>>& vertex_map);

void write_person(HCPerson* person, AttributeWriter& write, double tick); 

void write_edge(Edge<HCPerson>* edge, AttributeWriter& write);

void read_immunology(NamedListAttribute* list, std::shared_ptr<Immunology> imm, 
    HCPerson* person, double serialized_at);

void write_immunology(std::shared_ptr<Immunology> imm, AttributeWriter& write, double tick);

void write_event(int idx, boost::shared_ptr<Event> evt, std::stringstream& ss);


}



#endif /* SRC_CREATORS_H_ */
