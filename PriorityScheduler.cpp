#include "PriorityScheduler.h"

int PriorityScheduler::selectProcess(
    const std::vector<Process>& processes,
    int currentTime)
{
    int selectedIndex = -1;

    for (int i = 0;
         i < static_cast<int>(processes.size());
         i++)
    {
        const Process& process = processes[i];

        // Process has not arrived yet
        if (process.getArrivalTime() > currentTime)
            continue;

        // Only processes with allocated memory can run
        if (process.getStatus() != "Ready")
            continue;

        // Process already completed
        if (process.getRemainingTime() <= 0)
            continue;

        if (selectedIndex == -1)
        {
            selectedIndex = i;
            continue;
        }

        const Process& selected =
            processes[selectedIndex];

        // Lower number = higher priority
        if (process.getPriority() < selected.getPriority())
        {
            selectedIndex = i;
        }
        else if (
            process.getPriority() == selected.getPriority() &&
            process.getArrivalTime() < selected.getArrivalTime())
        {
            selectedIndex = i;
        }
        else if (
            process.getPriority() == selected.getPriority() &&
            process.getArrivalTime() == selected.getArrivalTime() &&
            process.getProcessId() < selected.getProcessId())
        {
            selectedIndex = i;
        }
    }

    return selectedIndex;
}
