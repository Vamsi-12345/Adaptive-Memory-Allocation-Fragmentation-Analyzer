
#ifndef PROCESSSIMULATION_H
#define PROCESSSIMULATION_H

#include <vector>
#include <string>

#include "Process.h"
#include "MemoryManager.h"
#include "AllocationStrategy.h"
#include "Scheduler.h"

struct CPUSchedulingResult
{
    std::string schedulerName;
    int totalSimulationTime;
    int completedProcesses;
    int totalProcesses;
    int contextSwitches;

    double averageWaitingTime;
    double averageTurnaroundTime;
    double averageResponseTime;
    double cpuUtilization;

    CPUSchedulingResult();
};

class ProcessSimulation
{
private:
    std::vector<Process> processes;
    std::string workloadName;

    void displayAllocatedBlock(
        const MemoryManager& memory,
        int processId
    ) const;

    void displayMemoryStatistics(
        const MemoryManager& memory,
        int allocationFailures = -1
    ) const;

    void runAutomaticSimulation(
        int totalMemory,
        AllocationStrategy* strategy
    );

    void cpuSchedulerMenu(int totalMemory);
    void runPriorityCPUSimulation(int totalMemory);
    void runFCFSCPUSimulation(int totalMemory);

    CPUSchedulingResult runCPUSimulation(
        int totalMemory,
        Scheduler& scheduler,
        bool preemptive,
        const std::string& schedulerName,
        bool displayDetails
    );

    void runSchedulingComparison(int totalMemory);

public:
    ProcessSimulation(
        const std::vector<Process>& selectedProcesses,
        const std::string& selectedWorkloadName
    );

    void displayWorkload() const;

    void executeWorkload(
        int totalMemory,
        AllocationStrategy* strategy
    );

    void compareAllStrategies(int totalMemory);
    void automaticSimulationMenu(int totalMemory);
    void showMenu(int totalMemory);
};

#endif
