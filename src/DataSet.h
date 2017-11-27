/*
 * DataSet.h
 *
 *  Created on: Mar 2, 2016
 *      Author: nick
 */

#ifndef SRC_DATASET_H_
#define SRC_DATASET_H_

#include <memory>

#include "boost/filesystem.hpp"
//#include "H5Cpp.h"

#include "DataSource.h"
#include "DataRecorder.h"
#include "ReducibleDataRecorder.h"

namespace hepcep {

/*
template<typename T>
struct h5_type_traits {
};

template<>
struct h5_type_traits<long> {
	static inline H5::PredType h5_type() {
		return H5::PredType::NATIVE_LONG;
	}
};

template<>
struct h5_type_traits<double> {
	static inline H5::PredType h5_type() {
		return H5::PredType::NATIVE_DOUBLE;
	}
};
*/

class DataSet {

public:
	DataSet() {
	}
	virtual ~DataSet() {
	}

	virtual void record(double tick) = 0;
	virtual void write(std::vector<double>& target) = 0;
};

template<typename T>
class ReducibleDataSet: public DataSet {

private:
	int rank;
	std::vector<std::shared_ptr<DataRecorder<T>>>recorders_;
	std::vector<double> ticks;
	std::vector<std::string> col_names;

public:
	ReducibleDataSet(std::vector<std::shared_ptr<DataRecorder<T>>> recorders);
	~ReducibleDataSet();

	void record(double tick) override;
	void write(std::vector<double>& target) override;
	const std::vector<string>& columnNames() const;
};

template<typename T>
ReducibleDataSet<T>::ReducibleDataSet(std::vector<std::shared_ptr<DataRecorder<T>>>recorders) :
rank {repast::RepastProcess::instance()->rank()}, recorders_ {recorders},ticks {}, col_names{}{
	for (auto recorder : recorders_) {
		col_names.push_back(recorder->name());
	}
}

template<typename T>
ReducibleDataSet<T>::~ReducibleDataSet() {}

template<typename T>
const std::vector<string>& ReducibleDataSet<T>::columnNames() const {
	return col_names;
}

template<typename T>
void ReducibleDataSet<T>::record(double tick) {

	if (rank == 0)
		ticks.push_back(tick);

	for (auto& recorder : recorders_) {
		recorder->record();
	}
}

template<typename T>
void ReducibleDataSet<T>::write(std::vector<double>& target) {
	size_t col_count = recorders_.size() + 1;
	size_t total = col_count * ticks.size();
	target.clear();
	target.assign(total, 0);

	if (rank == 0) {
		size_t idx = 0;
		for (auto t : ticks) {
			target[idx] = t;
			idx += col_count;
		}
	}

	std::vector<T> vec_data;
	size_t r_offset = 1;
	for (auto& recorder : recorders_) {
		recorder->write(&vec_data);
		if (rank == 0) {
			//std::cout << "write: " << vec_data.size() << std::endl;
			size_t idx = 0;
			for (auto v : vec_data) {
				target[idx + r_offset] = v;
				idx += col_count;
			}
		}
		++r_offset;
		vec_data.clear();
	}
}

//template<typename T>
//class H5ReducibleDataSet: public DataSet {
//
//private:
//	int rank;
//	H5::H5File* file;
//	H5::DataSet* data_set, *tick_data_set;
//	std::vector<std::shared_ptr<DataRecorder<T>>>recorders_;
//	unsigned int buffer_size_;
//	hsize_t offset;
//	std::vector<double> ticks;
//
//	void write();
//	void write_ticks();
//
//public:
//	H5ReducibleDataSet(std::vector<std::shared_ptr<DataRecorder<T>>> recorders, const std::string& fname,
//			unsigned int buffer_size, const std::string& data_set_name);
//	virtual ~H5ReducibleDataSet();
//
//	void record(double tick) override;
//	void write(std::vector<double>& target) override;
//
//};
//
//template<typename T>
//H5ReducibleDataSet<T>::H5ReducibleDataSet(std::vector<std::shared_ptr<DataRecorder<T>>>recorders, const std::string& fname,
//unsigned int buffer_size, const std::string& data_set_name) : rank {repast::RepastProcess::instance()->rank()}, file {nullptr},
//data_set {nullptr}, tick_data_set {nullptr}, recorders_ {recorders}, buffer_size_(buffer_size), offset(0), ticks {}
//
//{
//	if (rank == 0) {
//		std::string file_name(unique_file_name(fname));
//		boost::filesystem::path filepath(file_name);
//		if (!boost::filesystem::exists(filepath.parent_path()))
//		boost::filesystem::create_directories(filepath.parent_path());
//
//		hsize_t col_count = recorders_.size();
//		hsize_t dims[2] = {0, col_count};
//		hsize_t max_dims[2] = {H5S_UNLIMITED, col_count};
//		H5::DataSpace data_space(2, dims, max_dims);
//		file = new H5::H5File(file_name, H5F_ACC_TRUNC);
//
//		H5::DSetCreatPropList cparms;
//		hsize_t chunk_dims[2] = {buffer_size_, col_count};
//		cparms.setChunk(2, chunk_dims);
//
//		T fill_val(0);
//		cparms.setFillValue(h5_type_traits<T>::h5_type(), &fill_val);
//		data_set = new H5::DataSet(file->createDataSet(data_set_name, h5_type_traits<T>::h5_type(), data_space,
//				cparms));
//
//		const char* names[recorders_.size()];
//		size_t i = 0;
//		for (auto& r : recorders_) {
//			names[i] = r->name().c_str();
//			++i;
//		}
//
//		hsize_t names_dim[1] = {recorders_.size()};
//		H5::DataSpace col_names_ds = H5::DataSpace(1, names_dim);
//		H5::StrType str_type(H5::PredType::C_S1);
//		str_type.setSize(H5T_VARIABLE);
//		H5::Attribute attribute = data_set->createAttribute("names", str_type, col_names_ds);
//		attribute.write(str_type, names);
//
//		hsize_t tick_dims[1] = {0};
//		hsize_t tick_max_dims[1] = {H5S_UNLIMITED};
//		H5::DataSpace tick_data_space(1, tick_dims, tick_max_dims);
//		hsize_t tick_chunk_dims[1] = {buffer_size_};
//		cparms.setChunk(1, tick_chunk_dims);
//		double tick_fill_val(0);
//		cparms.setFillValue(H5::PredType::NATIVE_DOUBLE, &tick_fill_val);
//		tick_data_set = new H5::DataSet(file->createDataSet(data_set_name + " ticks", H5::PredType::NATIVE_DOUBLE, tick_data_space, cparms));
//	}
//}
//
//template<typename T>
//H5ReducibleDataSet<T>::~H5ReducibleDataSet() {
//	write();
//	if (rank == 0) {
//		data_set->close();
//		file->close();
//		delete data_set;
//		delete file;
//	}
//}
//
//template<typename T>
//void H5ReducibleDataSet<T>::record(double tick) {
//
//	if (rank == 0)
//		ticks.push_back(tick);
//
//	for (auto& recorder : recorders_) {
//		recorder->record();
//	}
//
//	if (recorders_[0]->size() == buffer_size_) {
//		write();
//	}
//}
//
//template<typename T>
//void H5ReducibleDataSet<T>::write_ticks() {
//	if (rank == 0) {
//		hsize_t size[1] = { offset + ticks.size() };
//		tick_data_set->extend(size);
//
//		hsize_t offsets[1] = { offset };
//		hsize_t dim[1] = { ticks.size() };
//		H5::DataSpace fspace = tick_data_set->getSpace();
//		// write the data into that column
//		fspace.selectHyperslab(H5S_SELECT_SET, dim, offsets);
//		H5::DataSpace memspace(1, dim);
//		tick_data_set->write(ticks.data(), H5::PredType::NATIVE_DOUBLE, memspace, fspace);
//	}
//	ticks.clear();
//}
//
//template<typename T>
//void H5ReducibleDataSet<T>::write(std::vector<double>& target) {
//	std::cerr << "shouldn't call write(vector<double>) on H5ReducibleDataSet " << std::endl;
//}
//
//template<typename T>
//void H5ReducibleDataSet<T>::write() {
//	if (recorders_[0]->size() > 0) {
//
//		hsize_t col_count = recorders_.size();
//		size_t rec_size = recorders_[0]->size();
//
//		hsize_t offsets[2] = { 0, 0 };
//		offsets[0] = offset;
//		hsize_t dims[2] = { rec_size, 1 };
//
//		if (rank == 0) {
//			hsize_t size[2] = { 0, col_count };
//			size[0] = rec_size + offset;
//			data_set->extend(size);
//			write_ticks();
//		}
//
//		std::vector<T> vec_data;
//		int col = 0;
//		for (auto& recorder : recorders_) {
//			recorder->write(&vec_data);
//			if (rank == 0) {
//				offsets[1] = col++;
//				H5::DataSpace fspace = data_set->getSpace();
//				// write the data into that column
//				fspace.selectHyperslab(H5S_SELECT_SET, dims, offsets);
//				H5::DataSpace memspace(2, dims);
//				data_set->write(vec_data.data(), h5_type_traits<T>::h5_type(), memspace, fspace);
//			}
//			vec_data.clear();
//		}
//
//		offset += rec_size;
//		ticks.clear();
//	}
//}

template<typename T>
class DataSetBuilder {
private:
	std::vector<std::shared_ptr<DataRecorder<T>>>recorders;

public:
	DataSetBuilder();
	virtual ~DataSetBuilder();

	template<typename Op>
	void addDataSource(std::shared_ptr<DataSource<T>> ds, Op op);

	std::shared_ptr<ReducibleDataSet<T>> createReducibleDataSet();

	std::shared_ptr<DataSet> createH5ReducibleDataSet(const std::string& name, unsigned int buffer,
				const std::string& data_set_name);
};

template<typename T>
DataSetBuilder<T>::DataSetBuilder() {
}

template<typename T>
DataSetBuilder<T>::~DataSetBuilder() {
}

template<typename T>
template<typename Op>
void DataSetBuilder<T>::addDataSource(std::shared_ptr<DataSource<T>> ds, Op op) {
	recorders.push_back(std::make_shared<ReducibleDataRecorder<Op, T>>(ds));
}

template<typename T>
std::shared_ptr<ReducibleDataSet<T>> DataSetBuilder<T>::createReducibleDataSet() {
	return std::make_shared<ReducibleDataSet<T>>(recorders);
}

//template<typename T>
//std::shared_ptr<DataSet> DataSetBuilder<T>::createH5ReducibleDataSet(const std::string& fname,
//		unsigned int buffer, const std::string& data_set_name) {
//	return std::make_shared<H5ReducibleDataSet<T>>(recorders, fname, buffer, data_set_name);
//}

} /* namespace seir */

#endif /* SRC_DATASET_H_ */
