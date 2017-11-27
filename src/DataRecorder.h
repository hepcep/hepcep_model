/*
 * DataRecorder.h
 *
 *  Created on: Mar 2, 2016
 *      Author: nick
 */

#ifndef SRC_DATARECORDER_H_
#define SRC_DATARECORDER_H_

namespace hepcep {

template<typename T>
class DataRecorder {

public:
	DataRecorder() {}
	virtual ~DataRecorder() {}
	virtual void record() = 0;
	virtual void write(std::vector<T>* var) = 0;
	virtual const std::string name() const = 0;

	virtual size_t size() const = 0;

};



}



#endif /* SRC_DATARECORDER_H_ */
