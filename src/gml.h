
#ifndef SRC_GML_H_
#define SRC_GML_H_

#include <string>
#include <vector>
#include <map>

namespace hepcep {

enum class AttributeType{INT, FLOAT, STRING, NODE, EDGE, LIST};


struct Attribute {
    AttributeType type_;
    std::string name_;

    Attribute(AttributeType type, const std::string& name);
    virtual ~Attribute() {}
    virtual void evaluate(const std::string& prefix) = 0;
};


struct IntAttribute : public Attribute {
    int value_;
    IntAttribute(const std::string& name, int value);
    virtual ~IntAttribute() {}
    void evaluate(const std::string& prefix);
};

struct FloatAttribute : public Attribute {
    double value_;
    FloatAttribute(const std::string& name, double value);
    virtual ~FloatAttribute() {}
    void evaluate(const std::string& prefix);
};

struct StringAttribute : public Attribute {
    std::string value_;
    StringAttribute(const std::string& name, const char* value);
    virtual ~StringAttribute() {}
    void evaluate(const std::string& prefix);
};

struct NamedListAttribute : public Attribute {
    std::map<std::string, Attribute*> attributes_;
    NamedListAttribute(AttributeType type, const std::string& name, std::vector<Attribute*>* list);
    virtual ~NamedListAttribute();
    void evaluate(const std::string& prefix);

    template<typename T>
    T getAttribute(const std::string& name) const;
    bool hasAttribute(const std::string& name) const;
};

std::vector<Attribute*>* make_attribute_list(Attribute* a);

NamedListAttribute* make_list(const std::string& id, std::vector<Attribute*>* attributes);

struct Graph {
    std::vector<Attribute*> nodes;
    std::vector<Attribute*> edges;
    std::vector<Attribute*> attributes;

    Graph(std::vector<Attribute*>* contents);
    virtual ~Graph();
    void evaluate();
};

Graph* read_gml(const std::string& fname);

template<>
std::string NamedListAttribute::getAttribute(const std::string& name) const;

template<>
int NamedListAttribute::getAttribute<int>(const std::string& name) const;

template<>
double NamedListAttribute::getAttribute(const std::string& name) const;

template<typename T>
T NamedListAttribute::getAttribute(const std::string& name) const {
    return dynamic_cast<T>(attributes_.at(name));
}

}

#endif