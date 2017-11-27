/*
 * DataSource.h
 *
 *  Created on: Mar 2, 2016
 *      Author: nick
 */

#ifndef SRC_DATASOURCE_H_
#define SRC_DATASOURCE_H_

#include <string>

namespace hepcep {

template<typename T>
class DataSource {

protected:
	std::string name_;
public:
	DataSource(const std::string& name);
	virtual ~DataSource();
	virtual T getData() = 0;

	const std::string name() const {
		return name_;
	}

};

template<typename T>
DataSource<T>::DataSource(const std::string& name) : name_{name}{}

template<typename T>
DataSource<T>::~DataSource() {}

template<typename T>
class SimpleDataSource : public DataSource<T> {

private:
	T* source_;

public:
	SimpleDataSource(const std::string& name, T* source);
	virtual ~SimpleDataSource();

	T getData() override;
};

template<typename T>
SimpleDataSource<T>::SimpleDataSource(const std::string& name, T* source) : DataSource<T>{name}, source_{source} {
}

template<typename T>
SimpleDataSource<T>::~SimpleDataSource() {}

template<typename T>
T SimpleDataSource<T>::SimpleDataSource::getData() {
	return *source_;
}

} /* namespace seir */

#endif /* SRC_DATASOURCE_H_ */
