
#ifndef REDUCEABLEDATASOURCE_H_
#define REDUCEABLEDATASOURCE_H_

#include <vector>
#include <boost/mpi.hpp>

#include "repast_hpc/TDataSource.h"
#include "repast_hpc/RepastProcess.h"

#include "DataRecorder.h"
#include "DataSource.h"

namespace hepcep {

/**
 * Source of data and a reduction operation. Used internally by a SVDataSet to
 * store the data sources. their associated ops etc.
 */
template <typename Op, typename T>
class ReducibleDataRecorder : public DataRecorder<T> {

private:
	Op op_;
	std::vector<T> data;
	std::shared_ptr<DataSource<T>> data_source_;
	int rank;

public:
	ReducibleDataRecorder(std::shared_ptr<DataSource<T>>& data_source);
	virtual ~ReducibleDataRecorder();

	virtual void record() override;
	virtual void write(std::vector<T>* var) override;

	const std::string name() const override {
		return data_source_->name();
	}

	size_t size() const override {
		return data.size();
	}
};

template<typename Op, typename T>
ReducibleDataRecorder<Op, T>::ReducibleDataRecorder(std::shared_ptr<DataSource<T>>& data_source) : DataRecorder<T>{},
	op_{}, data{}, data_source_{data_source}, rank{repast::RepastProcess::instance()->rank()} {
}

template<typename Op, typename T>
ReducibleDataRecorder<Op, T>::~ReducibleDataRecorder() {

}

template<typename Op, typename T>
void ReducibleDataRecorder<Op, T>::record() {
	data.push_back(data_source_->getData());
}

template<typename Op, typename T>
void ReducibleDataRecorder<Op, T>::write(std::vector<T>* var) {
	boost::mpi::communicator* comm = repast::RepastProcess::instance()->getCommunicator();
	if (rank == 0) {
		size_t size = data.size();
		size_t var_end = var->size();
		var->resize(var_end + size);
		reduce(*comm, &data[0], size, &(var->data()[var_end]), op_, 0);
	} else {
		reduce(*comm, &data[0], data.size(), op_, 0);
	}
	data.clear();
}

}

#endif /* REDUCEABLEDATASOURCE_H_ */
