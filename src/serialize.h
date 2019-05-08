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

#include "HCPerson.h"
#include "Network.h"
#include "gml.h"
#include "Zone.h"

namespace hepcep {



PersonPtr read_person(NamedListAttribute* node, std::map<std::string,ZonePtr>& zoneMap);

void read_edge(NamedListAttribute* edge_list, NetworkPtr<HCPerson>& net, 
    std::map<unsigned int, std::shared_ptr<HCPerson>>& vertex_map);

void write_person(HCPerson* person, AttributeWriter& write); 

void write_edge(Edge<HCPerson>* edge, AttributeWriter& write);

void read_immunology(NamedListAttribute* list, std::shared_ptr<Immunology> imm, 
    HCPerson* person);

void write_immunology(std::shared_ptr<Immunology> imm, AttributeWriter& write);

void write_event(int idx, boost::shared_ptr<Event> evt, AttributeWriter& write);


}



#endif /* SRC_CREATORS_H_ */
