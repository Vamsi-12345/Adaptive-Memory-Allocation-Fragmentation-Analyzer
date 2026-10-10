#ifndef FCFSSCHEDULER_H
#define FCFSSCHEDULER_H

#include "Scheduler.h"

class FCFSScheduler : public Scheduler
{
public:
    int selectProcess(
        const std::vector<Process>& processes,
        int currentTime
    ) override;
};

#endif
