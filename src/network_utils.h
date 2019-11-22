/*
 * network_utils.h
 *
 *  Created on: Jan 26, 2018
 *      Author: nick
 */

#ifndef SRC_NETWORK_UTILS_H_
#define SRC_NETWORK_UTILS_H_

#include <fstream>
#include <iterator>
#include <iostream>
#include <memory>
#include <map>


#include "boost/tokenizer.hpp"
#include "boost/filesystem.hpp"

#include "Network.h"
#include "Zone.h"
#include "gml.h"


namespace fs = boost::filesystem; 


namespace hepcep {

const std::string INDENT_1 = "  ";
const std::string INDENT_2 = "    ";

class AttributeWriter {
private:
    std::ofstream& out_;

public:
    AttributeWriter(std::ofstream& out);
    ~AttributeWriter();

    void operator() (const std::string& name, double value);
    void operator() (const std::string& name, const std::string& value);
    void operator() (const std::string& name, int value);
    void operator() (const std::string& name, unsigned int value);
};

void open_ofstream(const std::string& fname, std::ofstream& out);

template<typename VertexType>
void vnoop_writer(VertexType* vertex, AttributeWriter& write) {}

template<typename VertexType>
void enoop_writer(Edge<VertexType>*, AttributeWriter& write) {}


template<typename VertexType>
using EdgeAttribWriter = void (*)(Edge<VertexType>*, AttributeWriter& write);

template<typename VertexType>
using VertexAttribWriter = void (*)(VertexType*, AttributeWriter& write, double);

template<typename VertexType>
using VertexReader = std::shared_ptr<VertexType> (*)(NamedListAttribute*, std::map<unsigned int,ZonePtr>&, double);

template<typename VertexType>
using EdgeReader = void (*)(NamedListAttribute*, NetworkPtr<VertexType>& ,
    std::map<unsigned int, std::shared_ptr<VertexType>>&);


template<typename VertexType>
void write_network(const std::string& fname, double tick, NetworkPtr<VertexType> network,
        VertexAttribWriter<VertexType> vwriter, EdgeAttribWriter<VertexType> ewriter) {

    std::ofstream out;
    AttributeWriter write(out);
    open_ofstream(fname, out);

    out << "graph [\n";
    out << INDENT_1 << "directed " << (network->isDirected() ? 1 : 0) << "\n";
    write("tick", tick); 
    for (auto iter = network->verticesBegin(); iter != network->verticesEnd(); ++iter) {
        if ((*iter)->isActive()) {
            out << INDENT_1 << "node [\n" << INDENT_2 << "id " << (*iter)->id() << "\n";
            vwriter((*iter).get(), write, tick);
            out << INDENT_1 << "]\n";
        }

    }

    for (auto iter = network->edgesBegin(); iter != network->edgesEnd(); ++iter) {
        if ((*iter)->v1()->isActive() && (*iter)->v2()->isActive()) {
            out << INDENT_1 << "edge [\n" << INDENT_2 << "source " << (*iter)->v1()->id() << "\n";
            out << INDENT_2 << "target " << (*iter)->v2()->id() << "\n";
            ewriter((*iter).get(), write);
            out << INDENT_1 << "]\n";
        }
    }

    out << "]\n";
    out.flush();
    out.close();
}

template<typename VertexType>
void write_network(const std::string& fname, NetworkPtr<VertexType> network) {
    write_network(fname, 0, network, &vnoop_writer<VertexType>, &enoop_writer<VertexType>);
}

/**
 *
 * @param serialized_at output parameter that will contain the tick at which the network
 * was serialized.
 */
template<typename VertexType>
NetworkPtr<VertexType> read_network(const std::string& fname, VertexReader<VertexType> vertex_reader, 
    EdgeReader<VertexType> edge_reader, std::map<unsigned int,ZonePtr>& zone_map, double* serialized_at) 
{
    fs::path p(fname);
    if (fs::exists(p)) {
        Graph* gml_graph = read_gml(fname);
        bool directed = false;
        double tick = -1;
        for (auto attribute : gml_graph->attributes) {
            if (attribute->name_ == "tick") {
                tick = dynamic_cast<FloatAttribute*>(attribute)->value_;
                (*serialized_at) = tick;
            } else if (attribute->name_ == "directed") {
                directed = (bool)dynamic_cast<IntAttribute*>(attribute)->value_;
            }
        }

        if (tick == -1) {
            throw std::invalid_argument("Serialized network file " + fname + " does not contain tick attribute");
        }

        NetworkPtr<VertexType> net = std::make_shared<Network<VertexType>>(directed);
        std::map<unsigned int, std::shared_ptr<VertexType>> vertex_map;
        for (auto n : gml_graph->nodes) {
            std::shared_ptr<VertexType> person = vertex_reader(dynamic_cast<NamedListAttribute*>(n), zone_map, tick);
            net->addVertex(person);
            vertex_map.emplace(person->id(), person);
        }

        for (auto e : gml_graph->edges) {
            edge_reader(dynamic_cast<NamedListAttribute*>(e), net, vertex_map);
        }
        delete gml_graph;

        return net;
    } 
    
    throw std::invalid_argument("Serialized network file " + fname + " not found.");
    
}

}

#endif /* SRC_NETWORK_UTILS_H_ */
