/*
 * Edge.h
 *
 *  Created on: Nov 2, 2015
 *      Author: nick
 */

#ifndef SRC_EDGE_H_
#define SRC_EDGE_H_

#include <memory>

namespace hepcep {

template<typename V>
class Edge {

private:
	unsigned int id_;
	std::shared_ptr<V> v1_, v2_;
	double weight_;
	std::map<std::string, double> attributes;

public:
	/**
	 * Creates an edge from v1 to v2.
	 */
	Edge(unsigned int id, std::shared_ptr<V> v1, std::shared_ptr<V>, double weight = 1.0);
	virtual ~Edge();

	double getAttribute(const std::string& name, double default_value);
	void putAttribute(const std::string& name, double value);

	std::shared_ptr<V> v1() const {
		return v1_;
	}

	std::shared_ptr<V> v2() const {
		return v2_;
	}

	double weight() const {
		return weight_;
	}

	void setWeight(double val) {
		weight_ = val;
	}

	unsigned int id() const {
		return id_;
	}

};

template<typename V>
Edge<V>::Edge(unsigned int id, std::shared_ptr<V> v1, std::shared_ptr<V> v2, double weight) :
	id_(id), v1_(v1), v2_(v2), weight_(weight), attributes() {
}

template<typename V>
Edge<V>::~Edge() {
}

template<typename V>
double Edge<V>::getAttribute(const std::string& name, double default_value) {
    auto iter = attributes.find(name);
    if (iter == attributes.end()) return default_value;
    return iter->second;
}

template<typename V>
void Edge<V>::putAttribute(const std::string& name, double value) {
    attributes[name] = value;
}




} /* namespace TransModel */

#endif /* SRC_EDGE_H_ */
