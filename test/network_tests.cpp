/*
 * NetworkTests.cpp
 *
 *  Created on: Nov 2, 2015
 *      Author: nick
 */
#include <vector>

#include "gtest/gtest.h"
#include "boost/filesystem.hpp"

#include "Network.h"
#include "Edge.h"
#include "network_utils.h"

using namespace hepcep;

class Agent {

private:
	unsigned int id_, age_;

public:

	Agent(unsigned int id, unsigned int age) :
			id_(id), age_(age) {
	}

	unsigned int id() const {
		return id_;
	}

	int age() const {
		return age_;
	}

	void setAge(int age) {
		age_ = age;
	}
};


typedef std::shared_ptr<Agent> AgentPtrT;


TEST(NetworkTests, TestAdds) {
	Network<Agent> net(false);

	AgentPtrT one = std::make_shared<Agent>(1, 1);
	AgentPtrT two = std::make_shared<Agent>(2, 1);
	AgentPtrT three = std::make_shared<Agent>(3, 1);
	AgentPtrT four = std::make_shared<Agent>(4, 1);

	net.addVertex(one);
	net.addVertex(two);
	ASSERT_EQ(2, net.vertexCount());
	ASSERT_EQ(0, net.edgeCount());

	EdgePtrT<Agent> e = net.addEdge(three, four);
	e->setWeight(14);
	ASSERT_EQ(4, net.vertexCount());
	int id = 1;
	for (auto iter = net.verticesBegin(); iter != net.verticesEnd(); ++iter) {
		AgentPtrT agent = (*iter);
		ASSERT_EQ(id, agent->id());
		++id;
	}

	ASSERT_EQ(1, net.edgeCount());
	int count = 0;
	for (auto iter = net.edgesBegin(); iter != net.edgesEnd(); ++iter) {
		++count;
		EdgePtrT<Agent> edge = (*iter);
		ASSERT_EQ(3, edge->v1()->id());
		ASSERT_EQ(4, edge->v2()->id());
		ASSERT_EQ(14, edge->weight());
	}
	ASSERT_EQ(1, count);

	net.addEdge(two, one);
	count = 0;
	for (auto iter = net.edgesBegin(); iter != net.edgesEnd(); ++iter) {
		if (count == 0) {
			EdgePtrT<Agent> edge = (*iter);
			ASSERT_EQ(3, edge->v1()->id());
			ASSERT_EQ(4, edge->v2()->id());
			ASSERT_EQ(14, edge->weight());
		} else {
			EdgePtrT<Agent> edge = (*iter);
			ASSERT_EQ(2, edge->v1()->id());
			ASSERT_EQ(1, edge->v2()->id());
			ASSERT_EQ(1, edge->weight());
		}
		++count;
	}
	ASSERT_EQ(2, count);

	net.addEdge(two, four);
	ASSERT_EQ(3, net.edgeCount());
	ASSERT_EQ(2, net.outEdgeCount(two));
	auto iter = net.edgesBegin();
	advance(iter, 2);
	EdgePtrT<Agent> ep = (*iter);
	ASSERT_EQ(2, ep->id());
	ASSERT_EQ(2, ep->v1()->id());
	ASSERT_EQ(4, ep->v2()->id());
	ASSERT_EQ(1, ep->weight());

	net.addEdge(two, three);
	ASSERT_TRUE(net.hasEdge(two, three));
	ASSERT_TRUE(net.hasEdge(three, two));
}


// tests behavior specific to undirected net.
TEST(NetworkTests, TestUndirected) {
    Network<Agent> net(false);

    AgentPtrT one = std::make_shared<Agent>(1, 1);
    AgentPtrT two = std::make_shared<Agent>(2, 1);
    AgentPtrT three = std::make_shared<Agent>(3, 1);
    AgentPtrT four = std::make_shared<Agent>(4, 1);

    EdgePtrT<Agent> e = net.addEdge(three, four);
    ASSERT_EQ(1, net.edgeCount());

    ASSERT_TRUE(net.hasEdge(three, four));
    ASSERT_TRUE(net.hasEdge(four, three));
    e = net.removeEdge(four, three);
    ASSERT_EQ(three, e->v1());
    ASSERT_EQ(0, net.edgeCount());

    net.addEdge(one, two);
    net.addEdge(three, one);
    ASSERT_EQ(2, net.inEdgeCount(one));
    std::vector<EdgePtrT<Agent>> vec;
    net.inEdges(one, vec);
    ASSERT_EQ(2, vec.size());

    ASSERT_EQ(3, vec[0]->v1()->id());
    ASSERT_EQ(1, vec[0]->v2()->id());
    ASSERT_EQ(1, vec[1]->v1()->id());
    ASSERT_EQ(2, vec[1]->v2()->id());

    vec.clear();
    net.outEdges(one, vec);
    ASSERT_EQ(2, vec.size());

    ASSERT_EQ(1, vec[0]->v1()->id());
    ASSERT_EQ(2, vec[0]->v2()->id());
    ASSERT_EQ(3, vec[1]->v1()->id());
    ASSERT_EQ(1, vec[1]->v2()->id());

    // add an edge that already exists under undirected semantics
    unsigned int count = net.edgeCount();
    net.addEdge(two, one);
    ASSERT_EQ(count, net.edgeCount());
}

// tests behavior specific to directed net.
TEST(NetworkTests, TestDirected) {
    Network<Agent> net(true);

    AgentPtrT one = std::make_shared<Agent>(1, 1);
    AgentPtrT two = std::make_shared<Agent>(2, 1);
    AgentPtrT three = std::make_shared<Agent>(3, 1);
    AgentPtrT four = std::make_shared<Agent>(4, 1);

    EdgePtrT<Agent> e = net.addEdge(three, four);
    ASSERT_EQ(1, net.edgeCount());

    ASSERT_TRUE(net.hasEdge(three, four));
    ASSERT_FALSE(net.hasEdge(four, three));
    e = net.removeEdge(four, three);
    ASSERT_EQ(nullptr, e.get());
    ASSERT_EQ(1, net.edgeCount());

    e = net.removeEdge(three, four);
    ASSERT_EQ(three, e->v1());
    ASSERT_EQ(0, net.edgeCount());

    net.addEdge(one, two);
    net.addEdge(three, one);
    ASSERT_EQ(1, net.inEdgeCount(one));
    std::vector<EdgePtrT<Agent>> vec;
    net.inEdges(one, vec);
    ASSERT_EQ(1, vec.size());

    ASSERT_EQ(3, vec[0]->v1()->id());
    ASSERT_EQ(1, vec[0]->v2()->id());

    vec.clear();
    net.outEdges(one, vec);
    ASSERT_EQ(1, vec.size());

    ASSERT_EQ(1, vec[0]->v1()->id());
    ASSERT_EQ(2, vec[0]->v2()->id());

    // add an edge that already exists under undirected semantics,
    // but not under directed
   unsigned int count = net.edgeCount();
   net.addEdge(two, one);
   ASSERT_EQ(count + 1, net.edgeCount());
}


TEST(NetworkTests, TestEdgeGet) {
	Network<Agent> net(false);

	AgentPtrT one = std::make_shared<Agent>(1, 1);
	AgentPtrT two = std::make_shared<Agent>(2, 1);
	AgentPtrT three = std::make_shared<Agent>(3, 1);
	AgentPtrT four = std::make_shared<Agent>(4, 1);

	net.addVertex(one);
	net.addEdge(three, one);
	net.addEdge(one, two);
	net.addEdge(one, four);
	net.addEdge(three, four);

	std::vector<EdgePtrT<Agent>> vec;
	net.edges(one, vec);

	ASSERT_EQ(3, vec.size());
	EdgePtrT<Agent> edge = vec[0];
	ASSERT_EQ(3, edge->v1()->id());
	ASSERT_EQ(1, edge->v2()->id());

	edge = vec[1];
	ASSERT_EQ(1, edge->v1()->id());
	ASSERT_EQ(2, edge->v2()->id());

	edge = vec[2];
	ASSERT_EQ(1, edge->v1()->id());
	ASSERT_EQ(4, edge->v2()->id());
}

TEST(NetworkTests, TestRemoves) {
	Network<Agent> net(false);

	AgentPtrT one = std::make_shared<Agent>(1, 1);
	AgentPtrT two = std::make_shared<Agent>(2, 1);
	AgentPtrT three = std::make_shared<Agent>(3, 1);
	AgentPtrT four = std::make_shared<Agent>(4, 1);

	net.addVertex(one);
	net.addEdge(three, one);
	net.addEdge(one, two);
	net.addEdge(one, four);
	net.addEdge(three, four);

	ASSERT_EQ(4, net.vertexCount());
	ASSERT_EQ(4, net.edgeCount());

	ASSERT_EQ(3, net.outEdgeCount(one));
	ASSERT_EQ(1, net.outEdgeCount(two));
	ASSERT_EQ(2, net.outEdgeCount(three));
	ASSERT_EQ(2, net.outEdgeCount(four));

	ASSERT_EQ(3, net.outEdgeCount(one));
    ASSERT_EQ(1, net.outEdgeCount(two));
    ASSERT_EQ(2, net.outEdgeCount(three));
    ASSERT_EQ(2, net.outEdgeCount(four));

	net.removeVertex(one);
	ASSERT_EQ(3, net.vertexCount());
	ASSERT_EQ(1, net.edgeCount());

	ASSERT_EQ(0, net.outEdgeCount(one));
	ASSERT_EQ(0, net.outEdgeCount(two));
	ASSERT_EQ(1, net.outEdgeCount(three));
	ASSERT_EQ(1, net.outEdgeCount(four));

	ASSERT_EQ(0, net.inEdgeCount(one));
	ASSERT_EQ(0, net.inEdgeCount(two));
	ASSERT_EQ(1, net.inEdgeCount(three));
	ASSERT_EQ(1, net.inEdgeCount(four));

	int count = 0;
	for (auto iter = net.edgesBegin(); iter != net.edgesEnd(); ++iter) {
		EdgePtrT<Agent> edge = (*iter);
		ASSERT_EQ(3, edge->v1()->id());
		ASSERT_EQ(4, edge->v2()->id());
		ASSERT_EQ(1, edge->weight());
		++count;
	}

	ASSERT_EQ(1, count);
	int id = 2;
	count = 0;
	for (auto iter = net.verticesBegin(); iter != net.verticesEnd(); ++iter) {
		AgentPtrT ap = (*iter);
		ASSERT_EQ(id, ap->id());
		++id;
		++count;
	}
	ASSERT_EQ(3, count);

	unsigned int edge_count = net.edgeCount();
	net.addEdge(two, one);
	ASSERT_EQ(edge_count + 1, net.edgeCount());
	ASSERT_TRUE(net.removeEdge(two->id(), one->id()));
	ASSERT_EQ(0, net.outEdgeCount(two));
	ASSERT_EQ(0, net.inEdgeCount(one));
	ASSERT_EQ(edge_count, net.edgeCount());
}

TEST(NetworkTests, TestRemovesByIter) {
	Network<Agent> net(true);

	AgentPtrT one = std::make_shared<Agent>(1, 1);
	AgentPtrT two = std::make_shared<Agent>(2, 1);
	AgentPtrT three = std::make_shared<Agent>(3, 1);
	AgentPtrT four = std::make_shared<Agent>(4, 1);

	net.addVertex(one);
	net.addEdge(three, one);
	net.addEdge(one, two);
	net.addEdge(one, four);
	net.addEdge(three, four);

	ASSERT_EQ(2, net.outEdgeCount(one));
	ASSERT_EQ(0, net.outEdgeCount(two));
	ASSERT_EQ(2, net.outEdgeCount(three));
	ASSERT_EQ(0, net.outEdgeCount(four));

	ASSERT_EQ(1, net.inEdgeCount(one));
	ASSERT_EQ(1, net.inEdgeCount(two));
	ASSERT_EQ(0, net.inEdgeCount(three));
	ASSERT_EQ(2, net.inEdgeCount(four));

	ASSERT_EQ(4, net.vertexCount());
	ASSERT_EQ(4, net.edgeCount());

	int i = 1;
	for (auto iter = net.verticesBegin(); iter != net.verticesEnd();) {
		AgentPtrT p = (*iter);
		ASSERT_EQ(i, p->id());
		if (i == 1) {
			iter = net.removeVertex(iter);
		} else {
			++iter;
		}
		++i;
	}

	ASSERT_EQ(3, net.vertexCount());
	ASSERT_EQ(1, net.edgeCount());

	ASSERT_EQ(0, net.outEdgeCount(one));
	ASSERT_EQ(0, net.outEdgeCount(two));
	ASSERT_EQ(1, net.outEdgeCount(three));
	ASSERT_EQ(0, net.outEdgeCount(four));

	ASSERT_EQ(0, net.inEdgeCount(one));
	ASSERT_EQ(0, net.inEdgeCount(two));
	ASSERT_EQ(0, net.inEdgeCount(three));
	ASSERT_EQ(1, net.inEdgeCount(four));

	int count = 0;
	for (auto iter = net.edgesBegin(); iter != net.edgesEnd(); ++iter) {
		EdgePtrT<Agent> edge = (*iter);
		ASSERT_EQ(3, edge->v1()->id());
		ASSERT_EQ(4, edge->v2()->id());
		ASSERT_EQ(1, edge->weight());
		++count;
	}

	ASSERT_EQ(1, count);
	int id = 2;
	count = 0;
	for (auto iter = net.verticesBegin(); iter != net.verticesEnd(); ++iter) {
		AgentPtrT ap = (*iter);
		ASSERT_EQ(id, ap->id());
		++id;
		++count;
	}
	ASSERT_EQ(3, count);
}

TEST(NetworkTests, TestRemovesByVertexIds) {
	Network<Agent> net(true);

	AgentPtrT one = std::make_shared<Agent>(1, 1);
	AgentPtrT two = std::make_shared<Agent>(2, 1);
	AgentPtrT three = std::make_shared<Agent>(3, 1);
	AgentPtrT four = std::make_shared<Agent>(4, 1);

	net.addVertex(one);
	net.addEdge(three, one);
	net.addEdge(one, two);
	net.addEdge(one, four);
	net.addEdge(three, four);

	ASSERT_EQ(4, net.edgeCount());

	ASSERT_EQ(2, net.outEdgeCount(one));
	ASSERT_EQ(0, net.outEdgeCount(two));
	ASSERT_EQ(2, net.outEdgeCount(three));
	ASSERT_EQ(0, net.outEdgeCount(four));

	ASSERT_EQ(1, net.inEdgeCount(one));
    ASSERT_EQ(1, net.inEdgeCount(two));
    ASSERT_EQ(0, net.inEdgeCount(three));
    ASSERT_EQ(2, net.inEdgeCount(four));

	net.removeEdge(1, 2);

	ASSERT_EQ(3, net.edgeCount());

	auto eiter = net.edgesBegin();
	EdgePtrT<Agent> edge = (*eiter);
	ASSERT_EQ(3, edge->v1()->id());
	ASSERT_EQ(1, edge->v2()->id());

	++eiter;
	edge = (*eiter);
	ASSERT_EQ(1, edge->v1()->id());
	ASSERT_EQ(4, edge->v2()->id());

	++eiter;
	edge = (*eiter);
	ASSERT_EQ(3, edge->v1()->id());
	ASSERT_EQ(4, edge->v2()->id());

	ASSERT_EQ(1, net.outEdgeCount(one));
	ASSERT_EQ(0, net.outEdgeCount(two));
	ASSERT_EQ(2, net.outEdgeCount(three));
	ASSERT_EQ(0, net.outEdgeCount(four));

	ASSERT_EQ(1, net.inEdgeCount(one));
	ASSERT_EQ(0, net.inEdgeCount(two));
	ASSERT_EQ(0, net.inEdgeCount(three));
	ASSERT_EQ(2, net.inEdgeCount(four));
}

TEST(NetworkTests, testSimpleWriting) {
    std::string fname("../test_output/net.gml");
    boost::filesystem::path filepath(fname);
    if (boost::filesystem::exists(filepath)) {
        boost::filesystem::remove(filepath);
    }

    NetworkPtr<Agent> net = std::make_shared<Network<Agent>>(true);

    AgentPtrT one = std::make_shared<Agent>(1, 1);
    AgentPtrT two = std::make_shared<Agent>(2, 1);
    AgentPtrT three = std::make_shared<Agent>(3, 1);
    AgentPtrT four = std::make_shared<Agent>(4, 1);

    net->addVertex(one);
    net->addEdge(three, one);
    net->addEdge(one, two);
    net->addEdge(one, four);
    net->addEdge(three, four);

    write_network(fname, net);
}


void write_agent(Agent* agent, AttributeWriter& write) {
    write("age", agent->age());
}

void write_edge(Edge<Agent>* edge, AttributeWriter& write) {
    write("distance", edge->getAttribute("distance", 0));
}

TEST(NetworkTests, testAttribWriting) {
    std::string fname("../test_output/net_2.gml");
    boost::filesystem::path filepath(fname);
    if (boost::filesystem::exists(filepath)) {
        boost::filesystem::remove(filepath);
    }

    NetworkPtr<Agent> net = std::make_shared<Network<Agent>>(true);

    AgentPtrT one = std::make_shared<Agent>(1, 3);
    AgentPtrT two = std::make_shared<Agent>(2, 12);
    AgentPtrT three = std::make_shared<Agent>(3, 2);
    AgentPtrT four = std::make_shared<Agent>(4, 15);

    net->addVertex(one);
    net->addEdge(three, one)->putAttribute("distance", 12.343);
    net->addEdge(one, two)->putAttribute("distance", 2.343);
    net->addEdge(one, four)->putAttribute("distance", 1832.0);
    net->addEdge(three, four)->putAttribute("distance", 324);

    write_network(fname, net, &write_agent, &write_edge);
}

TEST(NetworkTests, testReadNetwork) {
	std::string fname("../test_data/net_2.gml");
	read_network<Agent>(fname);
}


