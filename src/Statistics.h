/*
 * Statistics.h
 */

#ifndef SRC_STATISTICS_H_
#define SRC_STATISTICS_H_

#include <memory>
#include <vector>

#include "DataSource.h"

namespace hepcep {

using DSDoublePtrT = std::shared_ptr<DataSource<double>>;

struct Stats {

	double val;
	Stats() : val{0} {}
};

class Statistics {

private:
	static Statistics* instance_;

	Stats stats;

	Statistics();
public:
	static Statistics* instance();
	virtual ~Statistics();
	void increment(long amount);
	void reset();
	void print(double tick);

	std::vector<DSDoublePtrT> createDataSources();
};



} /* namespace seir */

#endif /* SRC_STATISTICS_H_ */
