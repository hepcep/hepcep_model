/*
 * network_utils.h
 *
 *  Created on: Jan 26, 2018
 *      Author: nick
 */

#ifndef SRC_NETWORK_UTILS_H_
#define SRC_NETWORK_UTILS_H_

#include <fstream>

#include "Network.h"

const std::string INDENT_1 = "  ";
const std::string INDENT_2 = "    ";

namespace hepcep {

class AttributeWriter {
private:
    std::ofstream& out_;

public:
    AttributeWriter(std::ofstream& out);
    ~AttributeWriter();

    void operator() (const std::string& name, double value);
    void operator() (const std::string& name, const std::string& value);
};

void open_ofstream(const std::string& fname, std::ofstream& out);

template<typename VertexType>
void vnoop_writer(VertexType* vertex, AttributeWriter& write) {}

template<typename VertexType>
void enoop_writer(Edge<VertexType>*, AttributeWriter& write) {}


template<typename VertexType>
using EdgeAttribWriter = void (*)(Edge<VertexType>*, AttributeWriter& write);

template<typename VertexType>
using VertexAttribWriter = void (*)(VertexType*, AttributeWriter& write);


template<typename VertexType>
void write_network(const std::string& fname, Network<VertexType>& network,
        VertexAttribWriter<VertexType> vwriter, EdgeAttribWriter<VertexType> ewriter) {

    std::ofstream out;
    AttributeWriter write(out);
    open_ofstream(fname, out);

    out << "graph [\n";
    out << INDENT_1 << "directed " << (network.isDirected() ? 1 : 0) << "\n";
    for (auto iter = network.verticesBegin(); iter != network.verticesEnd(); ++iter) {
        out << INDENT_1 << "node [\n" << INDENT_2 << "id " << (*iter)->id() << "\n";
        vwriter((*iter).get(), write);
        out << INDENT_1 << "]\n";

    }

    for (auto iter = network.edgesBegin(); iter != network.edgesEnd(); ++iter) {
        out << INDENT_1 << "edge [\n" << INDENT_2 << "source " << (*iter)->v1()->id() << "\n";
        out << INDENT_2 << "target " << (*iter)->v2()->id() << "\n";
        ewriter((*iter).get(), write);
        out << INDENT_1 << "]\n";
    }

    out << "]\n";
    out.flush();
    out.close();

}

template<typename VertexType>
void write_network(const std::string& fname, Network<VertexType>& network) {
    write_network(fname, network, &vnoop_writer<VertexType>, &enoop_writer<VertexType>);
}


}

#endif /* SRC_NETWORK_UTILS_H_ */
