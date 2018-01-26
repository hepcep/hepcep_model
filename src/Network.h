/*
 * Network.h
 *
 *  Created on: Nov 2, 2015
 *      Author: nick
 */

#ifndef SRC_NETWORK_H_
#define SRC_NETWORK_H_

#include <map>
#include <set>
#include <vector>
#include <string>

#include "boost/iterator/transform_iterator.hpp"

#include "Edge.h"

namespace hepcep {

template<typename V>
using EdgePtrT = std::shared_ptr<Edge<V>>;

template<typename V>
using VertexPtrT = std::shared_ptr<V>;

template<typename T>
struct GetVal {

	T operator()(const std::pair<unsigned int, T>& pair) const {
		return pair.second;
	}
};

template<typename V>
using EdgeMap = std::map<unsigned int, EdgePtrT<V>>;

template<typename V>
using EdgeIter = boost::transform_iterator<GetVal<EdgePtrT<V>>, typename EdgeMap<V>::iterator>;

template<typename V>
using VertexMap = std::map<unsigned int, VertexPtrT<V>>;

template<typename V>
using VertexIter = boost::transform_iterator<GetVal<VertexPtrT<V>>, typename VertexMap<V>::iterator>;

using EdgeListEntry = std::set<unsigned int>;
using EdgeList = std::map<unsigned int, EdgeListEntry>;

template<typename V>
class Network {

private:
	bool directed_;
	unsigned int edge_idx;

	VertexMap<V> vertices;
	// all edges: key -> edge id, value -> Edge
	EdgeMap<V> edges_;
	// EdgeList is map: key -> vertex id, value -> EdgeListEntry
	// oel: outgoing edge list, iel: incoming edge list
	EdgeList oel, iel;
	unsigned int edge_counts;

	EdgePtrT<V> doAddEdge(const std::shared_ptr<V>& source, const std::shared_ptr<V>& target);
	void removeEdges(unsigned int idx, EdgeList& el, std::vector<EdgePtrT<V>>& removed_edges);
	bool doHasEdge(unsigned int v1_idx, unsigned int v2_idx);
	EdgePtrT<V> doRemoveEdge(unsigned int v1_idx, unsigned int v2_idx);
	void gatherEdges(unsigned int id, EdgeList& el, std::vector<EdgePtrT<V>>& edges);
	EdgePtrT<V> findEdge(unsigned int v1, unsigned int v2, EdgeList& el);
	unsigned int doEdgeCount(unsigned int id, EdgeList& el);

public:

	Network(bool directed);
	virtual ~Network();

	void addVertex(const std::shared_ptr<V>& vertex);
	bool removeVertex(const std::shared_ptr<V>& vertex);
	bool removeVertex(unsigned int id);

	VertexIter<V> removeVertex(VertexIter<V> iter);

	/**
	 * Adds an edge between source and target, and returns that edge. If the edge already
	 * exists, then the existing edge is returned.
	 */
	EdgePtrT<V> addEdge(const std::shared_ptr<V>& source, const std::shared_ptr<V>& target);

	/**
     * Adds an edge between source and target, and returns that edge. If the edge already
     * exists, then the existing edge is returned.
     */
	EdgePtrT<V> addEdge(unsigned int source, unsigned int target);

	EdgePtrT<V> removeEdge(VertexPtrT<V> v1, VertexPtrT<V> v2);
	EdgePtrT<V> removeEdge(unsigned int v1_idx, unsigned int v2_idx);
	bool hasEdge(VertexPtrT<V> v1, VertexPtrT<V> v2);
	bool hasEdge(unsigned int v1_idx, unsigned int v2_idx);

	EdgeIter<V> edgesBegin();
	EdgeIter<V> edgesEnd();

	VertexIter<V> verticesBegin();
	VertexIter<V> verticesEnd();

	unsigned int vertexCount() const {
		return vertices.size();
	}

	unsigned int edgeCount() const {
		return edges_.size();
	}

	/**
	 * Gets all the edges in which that specified vertex participates of the specified type and put them
	 * the specified vector.
	 */
	void edges(const VertexPtrT<V>& vert, std::vector<EdgePtrT<V>>& vec);

	/**
	 * Gets all the edges into the specified vertex of the specified type, and
	 * returns them in the specified vector.
	 */
	void inEdges(const VertexPtrT<V>& vert, std::vector<EdgePtrT<V>>& edges);


	/**
	 * Gets all the edges out of the specified vertex of the specified type, and
     * returns them in the specified vector.
	 */
	void outEdges(const VertexPtrT<V>& vert, std::vector<EdgePtrT<V>>& edges);

	/**
	 * Gets the number of edges into the specified vertex.
	 */
	unsigned int inEdgeCount(const VertexPtrT<V>& vert);

	/**
	 * Gets the number of edges out from the specified vertex.
	 */
	unsigned int outEdgeCount(const VertexPtrT<V>& vert);

	void clearEdges() {
		edges_.clear();
		oel.clear();
		iel.clear();
	}

	bool isDirected() const {
	    return directed_;
	}
};

template<typename V>
Network<V>::Network(bool directed) :
		directed_(directed), edge_idx(0), vertices(), edges_(), oel(), iel(), edge_counts{0} {
}

template<typename V>
Network<V>::~Network() {
}

template<typename V>
EdgeIter<V> Network<V>::edgesBegin() {
	GetVal<EdgePtrT<V>> gv;
	return EdgeIter<V>(edges_.begin(), gv);
}

template<typename V>
EdgeIter<V> Network<V>::edgesEnd() {
	GetVal<EdgePtrT<V>> gv;
	return EdgeIter<V>(edges_.end(), gv);
}

template<typename V>
VertexIter<V> Network<V>::verticesBegin() {
	GetVal<VertexPtrT<V>> gv;
	return VertexIter<V>(vertices.begin(), gv);
}

template<typename V>
VertexIter<V> Network<V>::verticesEnd() {
	GetVal<VertexPtrT<V>> gv;
	return VertexIter<V>(vertices.end(), gv);
}

template<typename V>
void Network<V>::gatherEdges(unsigned int id, EdgeList& el, std::vector<EdgePtrT<V>>& edges) {
    auto iter = el.find(id);
    if (iter != el.end()) {
        for (auto edge_idx : iter->second) {
            auto edge_iter = edges_.find(edge_idx);
            if (edge_iter == edges_.end())
                throw std::invalid_argument(
                        "Unexpectedly missing edge in inEdges(v1, v2): edge " + std::to_string(edge_idx) + " does not exist.");
            edges.push_back(edges_[edge_idx]);
        }
    }
}

template<typename V>
void Network<V>::edges(const VertexPtrT<V>& vertex, std::vector<EdgePtrT<V>>& edges) {
    gatherEdges(vertex->id(), iel, edges);
    gatherEdges(vertex->id(), oel, edges);
}

template<typename V>
void Network<V>::inEdges(const VertexPtrT<V>& vertex, std::vector<EdgePtrT<V>>& edges) {
    gatherEdges(vertex->id(), iel, edges);
    if (!directed_) {
        gatherEdges(vertex->id(), oel, edges);
    }
}

template<typename V>
void Network<V>::outEdges(const VertexPtrT<V>& vertex, std::vector<EdgePtrT<V>>& edges) {
    gatherEdges(vertex->id(), oel, edges);
    if (!directed_) {
        gatherEdges(vertex->id(), iel, edges);
    }
}

template<typename V>
unsigned int Network<V>::doEdgeCount(unsigned int id, EdgeList& el) {
    auto iter = el.find(id);
    return iter == el.end() ? 0 : iter->second.size();
}

template<typename V>
unsigned int Network<V>::outEdgeCount(const VertexPtrT<V>& vertex) {
    unsigned int count = doEdgeCount(vertex->id(), oel);
	if (!directed_) {
	    count += doEdgeCount(vertex->id(), iel);
    }

    return count;
}

template<typename V>
unsigned int Network<V>::inEdgeCount(const VertexPtrT<V>& vertex) {
    unsigned int count = doEdgeCount(vertex->id(), iel);
    if (!directed_) {
        count += doEdgeCount(vertex->id(), oel);
    }

    return count;
}

template<typename V>
void Network<V>::addVertex(const std::shared_ptr<V>& vertex) {
	vertices.emplace(std::make_pair(vertex->id(), vertex));
}

template<typename V>
bool Network<V>::hasEdge(VertexPtrT<V> v1, VertexPtrT<V> v2) {
	return hasEdge(v1->id(), v2->id());
}

template<typename V>
bool Network<V>::hasEdge(unsigned int v1_idx, unsigned int v2_idx) {
    bool has_edge = doHasEdge(v1_idx, v2_idx);
    if (!has_edge && !directed_) {
        has_edge = doHasEdge(v2_idx, v1_idx);
    }
    return has_edge;
}

template<typename V>
bool Network<V>::doHasEdge(unsigned int v1_idx, unsigned int v2_idx) {
	auto iter = oel.find(v1_idx);
	if (iter == oel.end()) return false;

    for (auto edge_idx : iter->second) {
        auto edge_iter = edges_.find(edge_idx);
        if (edge_iter == edges_.end())
            throw std::invalid_argument(
                    "Unexpectedly missing edge in hasEdge(v1, v2, type): edge " + std::to_string(edge_idx) + " does not exist.");
        EdgePtrT<V> edge = edge_iter->second;
        if (edge->v2()->id() == v2_idx) {
            return true;
        }
    }

	return false;
}

template<typename V>
EdgePtrT<V> Network<V>::removeEdge(VertexPtrT<V> v1, VertexPtrT<V> v2) {
    return removeEdge(v1->id(), v2->id());
}

template<typename V>
EdgePtrT<V> Network<V>::removeEdge(unsigned int v1_idx, unsigned int v2_idx) {
    EdgePtrT<V> edge = doRemoveEdge(v1_idx, v2_idx);
    if (!edge && !directed_) {
       edge = doRemoveEdge(v2_idx, v1_idx);
    }

    return edge;
}

template<typename V>
EdgePtrT<V> Network<V>::doRemoveEdge(unsigned int v1_idx, unsigned int v2_idx) {
	// defaults to nullptr
	EdgePtrT<V> edge;
	auto iter = oel.find(v1_idx);
	if (iter == oel.end()) {
		return edge;
	}

	for (auto edge_idx : iter->second) {
		auto edge_iter = edges_.find(edge_idx);
		if (edge_iter == edges_.end())
			throw std::invalid_argument(
					"Unable to delete edge: edge " + std::to_string(edge_idx) + " does not exist.");
		EdgePtrT<V> edge = edge_iter->second;
		if (edge->v2()->id() == v2_idx) {
			--edge_counts;
			edges_.erase(edge_iter);
			iter->second.erase(edge_idx);
			iel.at(v2_idx).erase(edge_idx);
			return edge;
		}
	}

	// defaults to nullptr
	return edge;
}

template<typename V>
void Network<V>::removeEdges(unsigned int idx, EdgeList& el, std::vector<EdgePtrT<V>>& removed_edges) {
	auto iter = el.find(idx);
	if (iter != el.end()) {
		// set of edges
		for (auto edge_idx_iter = iter->second.begin(); edge_idx_iter != iter->second.end();) {
			// remove the entry from mel
			auto mel_iter = edges_.find(*edge_idx_iter);
			if (mel_iter == edges_.end())
				throw std::invalid_argument(
						"Unable to delete edge: edge " + std::to_string(*edge_idx_iter) + " does not exist.");
			removed_edges.push_back(mel_iter->second);
			--edge_counts;

			// erase that edge
			edges_.erase(mel_iter);
			// remove the mel edge_id from the edge list set
			edge_idx_iter = iter->second.erase(edge_idx_iter);
		}
	}
}

template<typename V>
bool Network<V>::removeVertex(const std::shared_ptr<V>& vertex) {
	unsigned int id = vertex->id();
	return removeVertex(id);
}

template<typename V>
bool Network<V>::removeVertex(unsigned int id) {
	auto iter = vertices.find(id);
	if (iter != vertices.end()) {
		vertices.erase(iter);
		std::vector<EdgePtrT<V>> removed_edges;
		removeEdges(id, oel, removed_edges);
		// for each edge get v2 id, remove edge id from oel for v2 id
		for (auto& edge : removed_edges) {
			EdgeListEntry& data = iel.at(edge->v2()->id());
			data.erase(edge->id());
		}

		removed_edges.clear();
		removeEdges(id, iel, removed_edges);
		// for each edge get v1 id, remove edge id from iel for v1 id
		for (auto& edge : removed_edges) {
			EdgeListEntry& data = oel.at(edge->v1()->id());
			data.erase(edge->id());
		}
		return true;
	}

	return false;
}

template<typename V>
VertexIter<V> Network<V>::removeVertex(VertexIter<V> iter) {
	VertexPtrT<V> v = (*iter);
	unsigned int id = v->id();
	auto next_iter = ++iter;
	removeVertex(id);
	return next_iter;
}

template<typename V>
EdgePtrT<V> Network<V>::findEdge(unsigned int v1, unsigned int v2, EdgeList& el) {
    // nullptr
    EdgePtrT<V> edge;
    auto iter = el.find(v1);
    if (iter != el.end()) {
        for (unsigned int edge_idx : iter->second) {
            if (edges_[edge_idx]->v2()->id() == v2) {
                edge = edges_[edge_idx];
                break;
            }

        }
    }
    return edge;
}

template<typename V>
EdgePtrT<V> Network<V>::doAddEdge(const std::shared_ptr<V>& source, const std::shared_ptr<V>& target) {
	// assumes sanity checks have already occured
    EdgePtrT<V> edge = findEdge(source->id(), target->id(), oel);
    if (!directed_ && !edge) {
        edge = findEdge(target->id(), source->id(), oel);
    }
    if (edge) return edge;

	edge = std::make_shared<Edge<V>>(edge_idx, source, target);
	edges_.emplace(edge_idx, edge);

	auto out_iter = Network<V>::oel.find(source->id());
	if (out_iter == Network<V>::oel.end()) {
		Network<V>::oel.emplace(source->id(), EdgeListEntry{edge_idx});
	} else {
	    EdgeListEntry& data = out_iter->second;
		data.emplace(edge_idx);
	}

	auto in_iter = Network<V>::iel.find(target->id());
	if (in_iter == Network<V>::iel.end()) {
		Network<V>::iel.emplace(target->id(),  EdgeListEntry{edge_idx});
	} else {
	    EdgeListEntry& data = in_iter->second;
		data.emplace(edge_idx);
	}
	++edge_idx;
	++edge_counts;
	return edge;
}

template<typename V>
EdgePtrT<V> Network<V>::addEdge(unsigned int v1_idx, unsigned int v2_idx) {
	auto v1 = vertices.find(v1_idx);
	if (v1 == vertices.end())
		throw std::invalid_argument(
				"Unable to create edge: vertex " + std::to_string(v1_idx) + " not found in vertex map");
	auto v2 = vertices.find(v2_idx);
	if (v2 == vertices.end())
		throw std::invalid_argument(
				"Unable to create edge: vertex " + std::to_string(v2_idx) + " not found in vertex map");

	return doAddEdge(v1->second, v2->second);
}

template<typename V>
EdgePtrT<V> Network<V>::addEdge(const std::shared_ptr<V>& source, const std::shared_ptr<V>& target) {
	if (vertices.find(source->id()) == vertices.end()) {
		addVertex(source);
	}

	if (vertices.find(target->id()) == vertices.end()) {
		addVertex(target);
	}

	return doAddEdge(source, target);
}

}

/* namespace hepcep */

#endif /* SRC_NETWORK_H_ */
