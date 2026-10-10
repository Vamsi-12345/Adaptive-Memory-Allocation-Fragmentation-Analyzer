
#include "ProcessSimulation.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <limits>
#include <algorithm>

#include "FirstFit.h"
#include "BestFit.h"
#include "WorstFit.h"
#include "PriorityScheduler.h"
#include "FCFSScheduler.h"
#include "AdaptiveEngine.h"
#include "StrategyComparison.h"

using namespace std;

// ============================================================
// CPU SCHEDULING RESULT
// ============================================================

CPUSchedulingResult::CPUSchedulingResult()
{
    schedulerName = "";
    totalSimulationTime = 0;
    completedProcesses = 0;
    totalProcesses = 0;
    contextSwitches = 0;
    averageWaitingTime = 0.0;
    averageTurnaroundTime = 0.0;
    averageResponseTime = 0.0;
    cpuUtilization = 0.0;
}

// ============================================================
// CONSTRUCTOR
// ============================================================

ProcessSimulation::ProcessSimulation(
    const vector<Process>& selectedProcesses,
    const string& selectedWorkloadName)
{
    processes = selectedProcesses;
    workloadName = selectedWorkloadName;

    for (Process& process : processes)
        process.resetExecution();
}

// ============================================================
// DISPLAY WORKLOAD
// ============================================================

void ProcessSimulation::displayWorkload() const
{
    cout << "\n========================================\n";
    cout << "           SELECTED WORKLOAD\n";
    cout << "========================================\n";

    cout << "Workload Name  : " << workloadName << '\n';
    cout << "Total Processes: " << processes.size() << '\n';

    cout << '\n'
         << left
         << setw(12) << "Process ID"
         << setw(15) << "Memory (MB)"
         << setw(12) << "Arrival"
         << setw(12) << "Execution"
         << setw(10) << "Priority"
         << '\n';

    cout << string(61, '-') << '\n';

    for (const Process& process : processes)
    {
        cout << left
             << setw(12) << process.getProcessId()
             << setw(15) << process.getMemoryRequired()
             << setw(12) << process.getArrivalTime()
             << setw(12) << process.getExecutionTime()
             << setw(10) << process.getPriority()
             << '\n';
    }
}

// ============================================================
// DISPLAY ALLOCATED BLOCK
// ============================================================

void ProcessSimulation::displayAllocatedBlock(
    const MemoryManager& memory,
    int processId) const
{
    for (const MemoryBlock& block : memory.getBlocks())
    {
        if (!block.isFree() &&
            block.getProcessId() == processId)
        {
            cout << "Process P" << processId
                 << " allocated " << block.getSize()
                 << " MB at address "
                 << block.getStartAddress() << ".\n";
            return;
        }
    }
}

// ============================================================
// DISPLAY MEMORY STATISTICS
// ============================================================

void ProcessSimulation::displayMemoryStatistics(
    const MemoryManager& memory,
    int allocationFailures) const
{
    int usedMemory = 0;
    int freeMemory = 0;
    int largestFreeBlock = 0;
    int freeBlockCount = 0;

    for (const MemoryBlock& block : memory.getBlocks())
    {
        if (block.isFree())
        {
            freeMemory += block.getSize();
            freeBlockCount++;

            largestFreeBlock =
                max(largestFreeBlock, block.getSize());
        }
        else
        {
            usedMemory += block.getSize();
        }
    }

    double utilization = 0.0;
    double fragmentation = 0.0;

    if (memory.getTotalMemory() > 0)
    {
        utilization =
            static_cast<double>(usedMemory) /
            memory.getTotalMemory() * 100.0;
    }

    if (freeMemory > 0)
    {
        fragmentation =
            static_cast<double>(
                freeMemory - largestFreeBlock
            ) / freeMemory * 100.0;
    }

    if (allocationFailures < 0)
        allocationFailures = memory.getAllocationFailures();

    cout << "\n----------- MEMORY STATISTICS -----------\n";
    cout << "Total Memory          : "
         << memory.getTotalMemory() << " MB\n";
    cout << "Used Memory           : "
         << usedMemory << " MB\n";
    cout << "Free Memory           : "
         << freeMemory << " MB\n";
    cout << "Largest Free Block    : "
         << largestFreeBlock << " MB\n";
    cout << "Number of Free Blocks : "
         << freeBlockCount << '\n';

    cout << fixed << setprecision(2);

    cout << "Memory Utilization    : "
         << utilization << "%\n";
    cout << "External Fragmentation: "
         << fragmentation << "%\n";
    cout << "Allocation Failures   : "
         << allocationFailures << '\n';

    cout << "-----------------------------------------\n";
}

// ============================================================
// MEMORY ALLOCATION SIMULATION
// ============================================================

void ProcessSimulation::executeWorkload(
    int totalMemory,
    AllocationStrategy* strategy)
{
    if (totalMemory <= 0 || strategy == nullptr)
    {
        cout << "Invalid memory size or allocation strategy.\n";
        return;
    }

    if (processes.empty())
    {
        cout << "No workload is selected.\n";
        return;
    }

    vector<Process> simulationProcesses = processes;

    for (Process& process : simulationProcesses)
        process.resetExecution();

    MemoryManager memory(totalMemory, strategy);

    const int MAX_SIMULATION_TIME = 100000;

    int currentTime = 0;
    int completed = 0;
    int allocationFailures = 0;

    cout << "\n========================================\n";
    cout << "           PROCESS SIMULATION\n";
    cout << "========================================\n";

    cout << "Workload : " << workloadName << '\n';
    cout << "Strategy : " << strategy->getName() << '\n';
    cout << "Memory   : " << totalMemory << " MB\n";

    while (completed <
               static_cast<int>(simulationProcesses.size()) &&
           currentTime < MAX_SIMULATION_TIME)
    {
        bool runningProcessExists = false;
        bool futureProcessExists = false;
        bool waitingProcessExists = false;

        cout << "\nTime: " << currentTime << '\n';

        // Admit processes that have arrived.
        for (Process& process : simulationProcesses)
        {
            if (process.getStatus() == "Completed")
                continue;

            if (process.getArrivalTime() > currentTime)
            {
                futureProcessExists = true;
                continue;
            }

            if (process.getStatus() != "Waiting" &&
                process.getStatus() != "WaitingForMemory")
            {
                continue;
            }

            int blockIndex = strategy->findBlock(
                memory.getBlocks(),
                process.getMemoryRequired()
            );

            if (blockIndex == -1)
            {
                if (process.getStatus() != "WaitingForMemory")
                    allocationFailures++;

                process.setStatus("WaitingForMemory");
                waitingProcessExists = true;

                cout << "P" << process.getProcessId()
                     << " is waiting for sufficient contiguous memory.\n";

                continue;
            }

            if (memory.allocate(
                    process.getProcessId(),
                    process.getMemoryRequired()))
            {
                process.setStatus("Running");

                cout << "P" << process.getProcessId()
                     << " allocated "
                     << process.getMemoryRequired()
                     << " MB.\n";

                displayAllocatedBlock(
                    memory,
                    process.getProcessId()
                );
            }
            else
            {
                if (process.getStatus() != "WaitingForMemory")
                    allocationFailures++;

                process.setStatus("WaitingForMemory");
                waitingProcessExists = true;
            }
        }

        // Execute running processes for one time unit.
        for (Process& process : simulationProcesses)
        {
            if (process.getStatus() != "Running")
                continue;

            runningProcessExists = true;
            process.decreaseRemainingTime();

            cout << "P" << process.getProcessId()
                 << " executing. Remaining time: "
                 << process.getRemainingTime() << '\n';

            if (process.getRemainingTime() <= 0)
            {
                if (memory.deallocate(process.getProcessId()))
                {
                    cout << "P" << process.getProcessId()
                         << " completed and released memory.\n";
                }

                process.setStatus("Completed");
                completed++;
            }
        }

        displayMemoryStatistics(memory, allocationFailures);

        if (completed >=
            static_cast<int>(simulationProcesses.size()))
        {
            break;
        }

        if (!runningProcessExists &&
            !futureProcessExists &&
            waitingProcessExists)
        {
            cout << "\nSimulation stopped: remaining processes "
                    "cannot obtain sufficient contiguous memory.\n";
            break;
        }

        if (!runningProcessExists && !futureProcessExists)
        {
            cout << "\nSimulation stopped: no process can make progress.\n";
            break;
        }

        currentTime++;
    }

    cout << "\n========================================\n";
    cout << "         SIMULATION SUMMARY\n";
    cout << "========================================\n";

    cout << "Workload             : " << workloadName << '\n';
    cout << "Allocation Strategy  : " << strategy->getName() << '\n';
    cout << "Total Processes      : "
         << simulationProcesses.size() << '\n';
    cout << "Completed Processes  : " << completed << '\n';
    cout << "Uncompleted Processes: "
         << simulationProcesses.size() - completed << '\n';
    cout << "Simulation Time      : " << currentTime << '\n';
    cout << "Allocation Failures  : " << allocationFailures << '\n';

    displayMemoryStatistics(memory, allocationFailures);

    cout << "\nProcess Simulation finished.\n";
}

// ============================================================
// AUTOMATIC SIMULATION
// ============================================================

void ProcessSimulation::runAutomaticSimulation(
    int totalMemory,
    AllocationStrategy* strategy)
{
    executeWorkload(totalMemory, strategy);
}

// ============================================================
// STRATEGY COMPARISON
// ============================================================

void ProcessSimulation::compareAllStrategies(int totalMemory)
{
    StrategyComparison comparison;
    comparison.compareAll(totalMemory, processes);
}

// ============================================================
// ALLOCATION SIMULATION MENU
// ============================================================

void ProcessSimulation::automaticSimulationMenu(int totalMemory)
{
    if (processes.empty())
    {
        cout << "No workload is available.\n";
        return;
    }

    int choice = 0;

    do
    {
        cout << "\n========================================\n";
        cout << "        ALLOCATION SIMULATION\n";
        cout << "========================================\n";

        cout << "Current Workload: " << workloadName << '\n';
        cout << "1. First Fit\n";
        cout << "2. Best Fit\n";
        cout << "3. Worst Fit\n";
        cout << "4. Compare All Strategies\n";
        cout << "5. Back\n";
        cout << "Enter your choice: ";

        if (!(cin >> choice))
        {
            cin.clear();
            cin.ignore(
                numeric_limits<streamsize>::max(), '\n'
            );
            cout << "Invalid input.\n";
            choice = 0;
            continue;
        }

        FirstFit firstFit;
        BestFit bestFit;
        WorstFit worstFit;

        switch (choice)
        {
            case 1:
                runAutomaticSimulation(totalMemory, &firstFit);
                break;

            case 2:
                runAutomaticSimulation(totalMemory, &bestFit);
                break;

            case 3:
                runAutomaticSimulation(totalMemory, &worstFit);
                break;

            case 4:
                compareAllStrategies(totalMemory);
                break;

            case 5:
                break;

            default:
                cout << "Invalid choice. Select 1 to 5.\n";
        }
    }
    while (choice != 5);
}

// ============================================================
// CPU SCHEDULING SIMULATION
// ============================================================

CPUSchedulingResult ProcessSimulation::runCPUSimulation(
    int totalMemory,
    Scheduler& scheduler,
    bool preemptive,
    const string& schedulerName,
    bool displayDetails)
{
    CPUSchedulingResult result;

    result.schedulerName = schedulerName;
    result.totalProcesses = static_cast<int>(processes.size());

    if (totalMemory <= 0 || processes.empty())
    {
        cout << "Invalid memory or empty workload.\n";
        return result;
    }

    vector<Process> simulationProcesses = processes;

    for (Process& process : simulationProcesses)
        process.resetExecution();

    FirstFit firstFit;
    MemoryManager memory(totalMemory, &firstFit);

    vector<int> firstStart(simulationProcesses.size(), -1);
    vector<int> completionTimes(simulationProcesses.size(), -1);
    vector<int> responseTimes(simulationProcesses.size(), -1);

    int currentTime = 0;
    int completed = 0;
    int contextSwitches = 0;
    int lastProcessId = -1;
    int busyTime = 0;

    // Retain the active process for non-preemptive scheduling.
    int activeProcessIndex = -1;

    const int MAX_SIMULATION_TIME = 100000;

    if (displayDetails)
    {
        cout << "\n========================================\n";
        cout << "          CPU SCHEDULING\n";
        cout << "========================================\n";
        cout << "Workload : " << workloadName << '\n';
        cout << "Scheduler: " << schedulerName << '\n';
        cout << "Mode     : "
             << (preemptive ? "Preemptive" : "Non-preemptive")
             << '\n';
    }

    while (completed <
               static_cast<int>(simulationProcesses.size()) &&
           currentTime < MAX_SIMULATION_TIME)
    {
        // Admit arrived processes when memory is available.
        for (Process& process : simulationProcesses)
        {
            if (process.getStatus() != "Waiting" &&
                process.getStatus() != "WaitingForMemory")
            {
                continue;
            }

            if (process.getArrivalTime() > currentTime)
                continue;

            int blockIndex = firstFit.findBlock(
                memory.getBlocks(),
                process.getMemoryRequired()
            );

            if (blockIndex == -1)
            {
                process.setStatus("WaitingForMemory");
                continue;
            }

            if (memory.allocate(
                    process.getProcessId(),
                    process.getMemoryRequired()))
            {
                process.setStatus("Ready");

                if (displayDetails)
                {
                    cout << "Time " << currentTime
                         << ": P" << process.getProcessId()
                         << " admitted to ready queue.\n";
                }
            }
        }

        int selectedIndex = -1;

        // FCFS keeps the active process until it finishes.
        if (!preemptive && activeProcessIndex >= 0)
        {
            selectedIndex = activeProcessIndex;
        }
        else
        {
            selectedIndex = scheduler.selectProcess(
                simulationProcesses,
                currentTime
            );
        }

        if (selectedIndex < 0 ||
            selectedIndex >=
                static_cast<int>(simulationProcesses.size()))
        {
            bool futureProcessExists = false;
            bool readyOrRunningExists = false;

            for (const Process& process : simulationProcesses)
            {
                if (process.getStatus() == "Ready" ||
                    process.getStatus() == "Running")
                {
                    readyOrRunningExists = true;
                }

                if (process.getStatus() != "Completed" &&
                    process.getArrivalTime() > currentTime)
                {
                    futureProcessExists = true;
                }
            }

            if (!readyOrRunningExists && !futureProcessExists)
            {
                if (displayDetails && completed <
                    static_cast<int>(simulationProcesses.size()))
                {
                    cout << "Simulation stopped: no remaining process "
                            "can proceed with the available memory.\n";
                }
                break;
            }

            currentTime++;
            continue;
        }

        Process& selectedProcess = simulationProcesses[selectedIndex];

        if (selectedProcess.getStatus() != "Ready" &&
            selectedProcess.getStatus() != "Running")
        {
            currentTime++;
            continue;
        }

        if (lastProcessId != selectedProcess.getProcessId())
        {
            if (lastProcessId != -1)
                contextSwitches++;

            lastProcessId = selectedProcess.getProcessId();
        }

        if (firstStart[selectedIndex] == -1)
        {
            firstStart[selectedIndex] = currentTime;
            responseTimes[selectedIndex] =
                currentTime - selectedProcess.getArrivalTime();
        }

        selectedProcess.setStatus("Running");

        if (!preemptive)
            activeProcessIndex = selectedIndex;

        if (displayDetails)
        {
            cout << "Time " << currentTime
                 << ": P" << selectedProcess.getProcessId()
                 << " is executing.\n";
        }

        selectedProcess.decreaseRemainingTime();
        busyTime++;

        if (selectedProcess.getRemainingTime() <= 0)
        {
            memory.deallocate(selectedProcess.getProcessId());

            selectedProcess.setStatus("Completed");
            completionTimes[selectedIndex] = currentTime + 1;
            completed++;

            if (!preemptive)
                activeProcessIndex = -1;

            if (displayDetails)
            {
                cout << "Time " << currentTime + 1
                     << ": P" << selectedProcess.getProcessId()
                     << " completed and released memory.\n";
            }
        }
        else if (preemptive)
        {
            // Allow the priority scheduler to reconsider next time unit.
            selectedProcess.setStatus("Ready");
        }

        currentTime++;
    }

    result.totalSimulationTime = currentTime;
    result.completedProcesses = completed;
    result.contextSwitches = contextSwitches;

    double waitingTotal = 0.0;
    double turnaroundTotal = 0.0;
    double responseTotal = 0.0;
    int metricCount = 0;

    for (size_t i = 0; i < simulationProcesses.size(); i++)
    {
        if (completionTimes[i] == -1)
            continue;

        int turnaround =
            completionTimes[i] -
            simulationProcesses[i].getArrivalTime();

        int waiting =
            turnaround -
            simulationProcesses[i].getExecutionTime();

        waiting = max(0, waiting);

        waitingTotal += waiting;
        turnaroundTotal += turnaround;
        responseTotal += responseTimes[i];
        metricCount++;
    }

    if (metricCount > 0)
    {
        result.averageWaitingTime =
            waitingTotal / metricCount;

        result.averageTurnaroundTime =
            turnaroundTotal / metricCount;

        result.averageResponseTime =
            responseTotal / metricCount;
    }

    if (currentTime > 0)
    {
        result.cpuUtilization =
            static_cast<double>(busyTime) /
            currentTime * 100.0;
    }

    if (displayDetails)
    {
        cout << "\n----------- CPU SCHEDULING RESULTS -----------\n";
        cout << "Scheduler              : " << schedulerName << '\n';
        cout << "Processes completed    : "
             << completed << " / "
             << simulationProcesses.size() << '\n';
        cout << "Total simulation time  : " << currentTime << '\n';
        cout << "Context switches       : " << contextSwitches << '\n';

        cout << fixed << setprecision(2);

        cout << "Average waiting time   : "
             << result.averageWaitingTime << '\n';
        cout << "Average turnaround time: "
             << result.averageTurnaroundTime << '\n';
        cout << "Average response time  : "
             << result.averageResponseTime << '\n';
        cout << "CPU utilization        : "
             << result.cpuUtilization << "%\n";
        cout << "----------------------------------------------\n";
    }

    return result;
}

// ============================================================
// PRIORITY SCHEDULING
// ============================================================

void ProcessSimulation::runPriorityCPUSimulation(int totalMemory)
{
    PriorityScheduler scheduler;

    runCPUSimulation(
        totalMemory,
        scheduler,
        true,
        "Priority Scheduling",
        true
    );
}

// ============================================================
// FCFS SCHEDULING
// ============================================================

void ProcessSimulation::runFCFSCPUSimulation(int totalMemory)
{
    FCFSScheduler scheduler;

    runCPUSimulation(
        totalMemory,
        scheduler,
        false,
        "First Come First Served (FCFS)",
        true
    );
}

// ============================================================
// CPU SCHEDULING COMPARISON
// ============================================================

void ProcessSimulation::runSchedulingComparison(int totalMemory)
{
    PriorityScheduler priorityScheduler;
    FCFSScheduler fcfsScheduler;

    CPUSchedulingResult priorityResult =
        runCPUSimulation(
            totalMemory,
            priorityScheduler,
            true,
            "Priority Scheduling",
            false
        );

    CPUSchedulingResult fcfsResult =
        runCPUSimulation(
            totalMemory,
            fcfsScheduler,
            false,
            "FCFS",
            false
        );

    cout << "\n========================================\n";
    cout << "       CPU SCHEDULING COMPARISON\n";
    cout << "========================================\n";

    cout << fixed << setprecision(2);

    cout << "\nPriority Scheduling\n";
    cout << "Completed: "
         << priorityResult.completedProcesses << " / "
         << priorityResult.totalProcesses << '\n';
    cout << "Simulation Time: "
         << priorityResult.totalSimulationTime << '\n';
    cout << "Context Switches: "
         << priorityResult.contextSwitches << '\n';
    cout << "Average Waiting Time: "
         << priorityResult.averageWaitingTime << '\n';
    cout << "Average Turnaround Time: "
         << priorityResult.averageTurnaroundTime << '\n';
    cout << "Average Response Time: "
         << priorityResult.averageResponseTime << '\n';
    cout << "CPU Utilization: "
         << priorityResult.cpuUtilization << "%\n";

    cout << "\nFCFS Scheduling\n";
    cout << "Completed: "
         << fcfsResult.completedProcesses << " / "
         << fcfsResult.totalProcesses << '\n';
    cout << "Simulation Time: "
         << fcfsResult.totalSimulationTime << '\n';
    cout << "Context Switches: "
         << fcfsResult.contextSwitches << '\n';
    cout << "Average Waiting Time: "
         << fcfsResult.averageWaitingTime << '\n';
    cout << "Average Turnaround Time: "
         << fcfsResult.averageTurnaroundTime << '\n';
    cout << "Average Response Time: "
         << fcfsResult.averageResponseTime << '\n';
    cout << "CPU Utilization: "
         << fcfsResult.cpuUtilization << "%\n";

    cout << "\nNote: averages are calculated over completed processes.\n";
}

// ============================================================
// CPU SCHEDULER MENU
// ============================================================

void ProcessSimulation::cpuSchedulerMenu(int totalMemory)
{
    int choice = 0;

    do
    {
        cout << "\n========================================\n";
        cout << "           CPU SCHEDULING MENU\n";
        cout << "========================================\n";

        cout << "Current Workload: " << workloadName << '\n';
        cout << "1. Priority Scheduling\n";
        cout << "2. FCFS Scheduling\n";
        cout << "3. Compare CPU Schedulers\n";
        cout << "4. Back\n";
        cout << "Enter your choice: ";

        if (!(cin >> choice))
        {
            cin.clear();
            cin.ignore(
                numeric_limits<streamsize>::max(), '\n'
            );
            cout << "Invalid input.\n";
            choice = 0;
            continue;
        }

        switch (choice)
        {
            case 1:
                runPriorityCPUSimulation(totalMemory);
                break;

            case 2:
                runFCFSCPUSimulation(totalMemory);
                break;

            case 3:
                runSchedulingComparison(totalMemory);
                break;

            case 4:
                break;

            default:
                cout << "Invalid choice. Select 1 to 4.\n";
        }
    }
    while (choice != 4);
}

// ============================================================
// PROCESS SIMULATION MENU
// ============================================================

void ProcessSimulation::showMenu(int totalMemory)
{
    if (processes.empty())
    {
        cout << "\nNo workload is available for simulation.\n";
        return;
    }

    int choice = 0;

    do
    {
        cout << "\n========================================\n";
        cout << "          PROCESS SIMULATION\n";
        cout << "========================================\n";

        cout << "Current Workload: " << workloadName << '\n';
        cout << "Total Processes : " << processes.size() << '\n';
        cout << "Total Memory    : " << totalMemory << " MB\n\n";

        cout << "1. Display Selected Workload\n";
        cout << "2. Memory Allocation Simulation\n";
        cout << "3. CPU Scheduling\n";
        cout << "4. Adaptive Strategy Selection\n";
        cout << "5. Change Workload\n";
        cout << "6. Back to Main Menu\n";
        cout << "Enter your choice: ";

        if (!(cin >> choice))
        {
            cin.clear();
            cin.ignore(
                numeric_limits<streamsize>::max(), '\n'
            );
            cout << "Invalid input.\n";
            choice = 0;
            continue;
        }

        switch (choice)
        {
            case 1:
                displayWorkload();
                break;

            case 2:
                automaticSimulationMenu(totalMemory);
                break;

            case 3:
                cpuSchedulerMenu(totalMemory);
                break;

            case 4:
            {
                AdaptiveEngine engine;
                engine.run(totalMemory, processes);
                break;
            }

            case 5:
                cout << "\nReturn to the main menu to select a "
                        "different workload.\n";
                break;

            case 6:
                break;

            default:
                cout << "Invalid choice. Select 1 to 6.\n";
        }
    }
    while (choice != 6);
}
