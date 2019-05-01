
#ifndef SRC_GML_H_
#define SRC_GML_H_

#include <string>
#include <vector>

namespace hepcep {

enum class AttributeType{INT, FLOAT, STRING, NODE, EDGE};


struct Attribute {
    AttributeType type_;
    std::string name_;

    Attribute(AttributeType type, const std::string& name);
    virtual void evaluate(const std::string& prefix) = 0;
};


struct IntAttribute : public Attribute {
    int value_;
    IntAttribute(const std::string& name, int value);
    void evaluate(const std::string& prefix);
};

struct FloatAttribute : public Attribute {
    double value_;
    FloatAttribute(const std::string& name, double value);
    void evaluate(const std::string& prefix);
};

struct StringAttribute : public Attribute {
    std::string value_;
    StringAttribute(const std::string& name, const char* value);
    void evaluate(const std::string& prefix);
};

struct ListAttribute : public Attribute {
    std::vector<Attribute*>* attributes_;
    ListAttribute(AttributeType type, const std::string& name, std::vector<Attribute*>* list);
    void evaluate(const std::string& prefix);
};


std::vector<Attribute*>* make_attribute_list(Attribute* a);

struct Graph {
    std::vector<Attribute*> nodes;
    std::vector<Attribute*> edges;
    std::vector<Attribute*> attributes;

    Graph(std::vector<Attribute*>* contents);
    void evaluate();
};

void read_gml(const std::string& fname);

}

#endif