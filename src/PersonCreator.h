/*
 * PersonCreator.h
 *
 *  
 */

#ifndef SRC_PERSONCREATOR_H_
#define SRC_PERSONCREATOR_H_

#include <map>
#include <string>

#include "HCPerson.h"

namespace hepcep {

class PersonCreator {

protected:
	bool burnInMode = false;
	double burnInDays = 0;
	double probInfectedNewArriving;
	unsigned int id_counter;

public:

	PersonCreator(unsigned int starting_id);
	
  virtual ~PersonCreator();
	
	/**
	 * Create Persons from the specified file, placing them in the specified map.
	 */
	void create_persons(std::map<unsigned int, PersonPtr>& persons,
			std::vector<HCPersonData> & personData, std::map<unsigned int,ZonePtr>& zoneMap,
			NetworkPtr<HCPerson> network, unsigned int person_count, bool earlyCareerOnly);

	void create_persons_from_ergm_data(std::map<unsigned int, PersonPtr>& persons,
		std::vector<HCPersonData> & personData, std::map<unsigned int,ZonePtr>& zoneMap,
		NetworkPtr<HCPerson> network);

	void create_person_from_data(std::map<unsigned int, PersonPtr>& persons,
		HCPersonData & data, std::map<unsigned int,ZonePtr>& zoneMap,
		NetworkPtr<HCPerson> network, bool earlyCareerOnly);

	void setBurnInPeriod(bool burnInMode, double burnInPeriod);

};

} // namespace hepcep

#endif /* SRC_PERSONCREATOR_H_ */
