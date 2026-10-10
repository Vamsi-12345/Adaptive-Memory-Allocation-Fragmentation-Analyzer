#include "FCFSScheduler.h"

int FCFSScheduler::selectProcess(
    const std::vector<Process>& processes,
    int currentTime)
{
    int selectedIndex = -1;

    for (int i = 0;
         i < static_cast<int>(processes.size());
         i++)
    {
        const Process& process = processes[i];

        // Process has not arrived
        if (process.getArrivalTime() > currentTime)
            continue;

        // Only processes with allocated memory can run
        if (process.getStatus() != "Ready")
            continue;

        // Already completed
        if (process.getRemainingTime() <= 0)
            continue;

        if (selectedIndex == -1)
        {
            selectedIndex = i;
            continue;
        }

        const Process& selected =
            processes[selectedIndex];

        // First Come First Serve
        if (process.getArrivalTime() <
            selected.getArrivalTime())
        {
            selectedIndex = i;
        }
        else if (
            process.getArrivalTime() ==
                selected.getArrivalTime() &&
            process.getProcessId() <
                selected.getProcessId())
        {
            selectedIndex = i;
        }
    }

    return selectedIndex;
}
