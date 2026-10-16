#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include "Process.h"

class Scheduler
{
public:
    virtual ~Scheduler() {}

    // Returns index of the process that should get the CPU.
    virtual int selectProcess(
        const std::vector<Process>& processes,
        int currentTime
    ) = 0;
};

#endif
