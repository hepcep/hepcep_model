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

    ListAttribute::ListAttribute(AttributeType type, const std::string& name, std::vector<Attribute*>* list): 
        Attribute(type, name), attributes_(list) {}

    void ListAttribute::evaluate(const std::string& prefix) {
        std::cout << prefix << name_ << std::endl;
        const std::string indent = prefix + "  ";
        for (auto a : (*attributes_)) {
            a->evaluate(indent);
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

    void read_gml(const std::string& fname) {
        yyin = fopen(fname.c_str(), "r");
        yyparse();
        fclose(yyin);
        gml_graph->evaluate();

    }
}
 
