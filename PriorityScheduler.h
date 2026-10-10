#ifndef PRIORITYSCHEDULER_H
#define PRIORITYSCHEDULER_H

#include "Scheduler.h"

class PriorityScheduler : public Scheduler
{
public:

    // Priority 1 = highest
    // Priority 2 = medium
    // Priority 3 = lowest
    int selectProcess(
        const std::vector<Process>& processes,
        int currentTime
    ) override;
};

#endif
