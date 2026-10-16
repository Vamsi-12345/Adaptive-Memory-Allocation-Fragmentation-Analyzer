
#include "PriorityScheduler.h"

#include <queue>
#include <vector>

int PriorityScheduler::selectProcess(
    const std::vector<Process>& processes,
    int currentTime)
{
    // Stores the information needed to rank each ready process.
    struct ProcessEntry
    {
        int index;
        int priority;
        int arrivalTime;
        int processId;
    };

    // priority_queue normally places the largest element at the top.
    // This comparator makes the smallest priority number come first.
    struct ComparePriority
    {
        bool operator()(
            const ProcessEntry& a,
            const ProcessEntry& b) const
        {
            // Lower priority number means higher scheduling priority.
            if (a.priority != b.priority)
                return a.priority > b.priority;

            // Earlier arrival wins when priorities are equal.
            if (a.arrivalTime != b.arrivalTime)
                return a.arrivalTime > b.arrivalTime;

            // Smaller process ID wins when both are equal.
            return a.processId > b.processId;
        }
    };

    std::priority_queue<
        ProcessEntry,
        std::vector<ProcessEntry>,
        ComparePriority
    > readyQueue;

    // Add only processes that have arrived and are ready to execute.
    for (int i = 0; i < static_cast<int>(processes.size()); ++i)
    {
        const Process& process = processes[i];

        if (process.getArrivalTime() > currentTime)
            continue;

        if (process.getStatus() != "Ready")
            continue;

        if (process.getRemainingTime() <= 0)
            continue;

        ProcessEntry entry;
        entry.index = i;
        entry.priority = process.getPriority();
        entry.arrivalTime = process.getArrivalTime();
        entry.processId = process.getProcessId();

        readyQueue.push(entry);
    }

    // Return the index of the highest-priority eligible process.
    if (readyQueue.empty())
        return -1;

    return readyQueue.top().index;
}
