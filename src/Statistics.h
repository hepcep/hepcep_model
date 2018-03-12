/*
 * Statistics.h
 */

#ifndef SRC_STATISTICS_H_
#define SRC_STATISTICS_H_

#include <vector>

#include "AggregateStats.h"

namespace hepcep {

enum class LogType {ACTIVATED, EXPOSED, INFECTED, INFECTIOUS, CHRONIC, RECOVERED, DEACTIVATED, INFO, STATUS,
    STARTED_TREATMENT, CURED, REGULAR_STATUS, FAILED_TREATMENT
};

class HCPerson;

class Statistics {

private:
    static Statistics* instance_;
    std::vector<AggregateStats> stats;

    Statistics();
public:
    static Statistics* instance();
    virtual ~Statistics();

    void logStatusChange(LogType logType, HCPerson* person, const std::string& msg);

    void collectStats(std::map<unsigned int, std::shared_ptr<HCPerson>>& persons);

};

} /* namespace seir */


#endif /* SRC_STATISTICS_H_ */

