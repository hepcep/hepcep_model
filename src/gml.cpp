#include <iostream>

#include "gml.h"
#include "gml.tab.h"


extern FILE* yyin;
extern hepcep::Graph* gml_graph;

namespace hepcep {

Attribute::Attribute(AttributeType type, const std::string& name) : type_(type), name_(name) {}

IntAttribute::IntAttribute(const std::string& name, int value) : Attribute(AttributeType::INT, name),
    value_(value) {}

void IntAttribute::evaluate(const std::string& prefix) {
    std::cout << prefix << name_ << ": " << value_ << std::endl;
}

FloatAttribute::FloatAttribute(const std::string& name, double value) : Attribute(AttributeType::FLOAT, name),
    value_(value) {}

void FloatAttribute::evaluate(const std::string& prefix) {
    std::cout << prefix << name_ << ": " << value_ << std::endl;
}

StringAttribute::StringAttribute(const std::string& name, const char* value) : 
    Attribute(AttributeType::STRING, name), value_(value) {
        value_ = value_.substr(1, value_.size() - 2);
    }

void StringAttribute::evaluate(const std::string& prefix) {
    std::cout << prefix << name_ << ": " << value_ << std::endl;
}

NamedListAttribute::NamedListAttribute(AttributeType type, const std::string& name, std::vector<Attribute*>* list): 
    Attribute(type, name), attributes_() {
        for (auto a : (*list)) {
            attributes_.emplace(a->name_, a);
        }
    }


NamedListAttribute::~NamedListAttribute() {
    for (auto item : attributes_) {
        delete item.second;
    }
}

void NamedListAttribute::evaluate(const std::string& prefix) {
    std::cout << prefix << name_ << std::endl;
    const std::string indent = prefix + "  ";
    for (auto a : attributes_) {
        a.second->evaluate(indent);
    }
}

Graph::Graph(std::vector<Attribute*>* contents) : nodes(), edges(), attributes() {
    for (auto atb : (*contents)) {
        if (atb->type_ == AttributeType::NODE) {
            nodes.push_back(atb);
        } else if (atb->type_ == AttributeType::EDGE) {
            edges.push_back(atb);
        } else {
            attributes.push_back(atb);
        }
    }
}

Graph::~Graph() {
    for (auto item : nodes) {
        delete item;
    }

    for (auto item : edges) {
        delete item;
    }

    for (auto item : attributes) {
        delete item;
    }
}

void Graph::evaluate() {
    std::cout << "graph" << std::endl;
    
    for (auto a : nodes) {
        a->evaluate("  ");
    }

    for (auto a : edges) {
        a->evaluate("  ");
    }

    for (auto a : attributes) {
        a->evaluate("  ");
    }
}

std::vector<Attribute*>* make_attribute_list(Attribute* a) {
    return new std::vector<Attribute*>({a});
}

NamedListAttribute *make_list(const std::string& id, std::vector<Attribute*>* attributes) {
    std::string name = id;
    AttributeType type = AttributeType::LIST;
    if (id == "node") {
        type = AttributeType::NODE;
    } else if (id == "edge") {
        type = AttributeType::EDGE;
    }
    return new NamedListAttribute(type, id, attributes);
}

Graph* read_gml(const std::string& fname) {
    yyin = fopen(fname.c_str(), "r");
    yyparse();
    fclose(yyin);
    return gml_graph;
    //gml_graph->evaluate();
}



template<>
int NamedListAttribute::getAttribute<int>(const std::string& name) const {
    return dynamic_cast<IntAttribute*>(attributes_.at(name))->value_;
}

template<>
double NamedListAttribute::getAttribute(const std::string& name) const {
    return dynamic_cast<FloatAttribute*>(attributes_.at(name))->value_;
}

template<>
std::string NamedListAttribute::getAttribute(const std::string& name) const {
    return dynamic_cast<StringAttribute*>(attributes_.at(name))->value_;
}

bool NamedListAttribute::hasAttribute(const std::string& name) const {
    return attributes_.find(name) != attributes_.end();
}

}
 
