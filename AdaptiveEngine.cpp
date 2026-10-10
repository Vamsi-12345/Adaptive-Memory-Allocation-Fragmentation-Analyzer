
#include "AdaptiveEngine.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>

#include "MemoryManager.h"
#include "MemoryBlock.h"
#include "FirstFit.h"
#include "BestFit.h"
#include "WorstFit.h"

using namespace std;

AdaptiveResult AdaptiveEngine::evaluateStrategy(
    int totalMemory,
    const vector<Process>& processes,
    AllocationStrategy* strategy,
    const string& strategyName)
{
    AdaptiveResult result = {};

    result.strategyName = strategyName;
    result.totalMemory = totalMemory;
    result.totalProcesses = static_cast<int>(processes.size());

    if (totalMemory <= 0 || processes.empty() || strategy == nullptr)
    {
        return result;
    }

    MemoryManager memory(totalMemory, strategy);

    vector<Process> simulationProcesses = processes;

    for (Process& process : simulationProcesses)
    {
        process.resetExecution();
    }

    const int MAX_SIMULATION_TIME = 100000;

    int currentTime = 0;
    int completed = 0;
    int allocationFailures = 0;
    int peakUsedMemory = 0;

    double utilizationTotal = 0.0;
    double fragmentationTotal = 0.0;
    int samples = 0;

    while (completed < static_cast<int>(simulationProcesses.size()) &&
           currentTime < MAX_SIMULATION_TIME)
    {
        bool runningExists = false;
        bool futureExists = false;
        bool waitingExists = false;

        // Admit processes that have arrived.
        for (Process& process : simulationProcesses)
        {
            if (process.getStatus() == "Completed")
            {
                continue;
            }

            if (process.getArrivalTime() > currentTime)
            {
                futureExists = true;
                continue;
            }

            if (process.getStatus() == "Waiting" ||
                process.getStatus() == "WaitingForMemory")
            {
                int blockIndex = strategy->findBlock(
                    memory.getBlocks(),
                    process.getMemoryRequired());

                if (blockIndex == -1)
                {
                    if (process.getStatus() != "WaitingForMemory")
                    {
                        allocationFailures++;
                    }

                    process.setStatus("WaitingForMemory");
                    waitingExists = true;
                    continue;
                }

                if (memory.allocate(
                        process.getProcessId(),
                        process.getMemoryRequired()))
                {
                    process.setStatus("Running");
                }
                else
                {
                    if (process.getStatus() != "WaitingForMemory")
                    {
                        allocationFailures++;
                    }

                    process.setStatus("WaitingForMemory");
                    waitingExists = true;
                }
            }
        }

        // Execute running processes.
        for (Process& process : simulationProcesses)
        {
            if (process.getStatus() != "Running")
            {
                continue;
            }

            runningExists = true;
            process.decreaseRemainingTime();

            if (process.getRemainingTime() <= 0)
            {
                memory.deallocate(process.getProcessId());
                process.setStatus("Completed");
                completed++;
            }
        }

        // Sample memory usage and external fragmentation.
        int usedMemory = 0;
        int freeMemory = 0;
        int largestFreeBlock = 0;

        for (const MemoryBlock& block : memory.getBlocks())
        {
            if (block.isFree())
            {
                freeMemory += block.getSize();

                largestFreeBlock = max(
                    largestFreeBlock,
                    block.getSize());
            }
            else
            {
                usedMemory += block.getSize();
            }
        }

        peakUsedMemory = max(peakUsedMemory, usedMemory);

        utilizationTotal +=
            static_cast<double>(usedMemory) / totalMemory * 100.0;

        double fragmentation = 0.0;

        if (freeMemory > 0)
        {
            fragmentation =
                static_cast<double>(freeMemory - largestFreeBlock) /
                freeMemory * 100.0;
        }

        fragmentationTotal += fragmentation;
        samples++;

        if (completed >= static_cast<int>(simulationProcesses.size()))
        {
            currentTime++;
            break;
        }

        if (!runningExists && !futureExists && waitingExists)
        {
            break;
        }

        if (!runningExists && !futureExists)
        {
            break;
        }

        currentTime++;
    }

    result.completedProcesses = completed;
    result.totalSimulationTime = currentTime;
    result.allocationFailures = allocationFailures;
    result.peakUsedMemory = peakUsedMemory;

    if (samples > 0)
    {
        result.averageUtilization = utilizationTotal / samples;
        result.averageExternalFragmentation =
            fragmentationTotal / samples;
    }

    result.utilizationScore = result.averageUtilization;

    result.fragmentationScore =
        max(0.0, 100.0 - result.averageExternalFragmentation);

    result.failureScore =
        max(0.0, 100.0 - allocationFailures * 10.0);

    result.completionScore =
        static_cast<double>(completed) /
        result.totalProcesses * 100.0;

    result.finalScore =
        0.35 * result.utilizationScore +
        0.35 * result.fragmentationScore +
        0.20 * result.failureScore +
        0.10 * result.completionScore;

    return result;
}

void AdaptiveEngine::displayResult(
    const AdaptiveResult& result) const
{
    cout << "\nStrategy: " << result.strategyName << '\n';
    cout << "----------------------------------------\n";

    cout << "Processes completed: "
         << result.completedProcesses << " / "
         << result.totalProcesses << '\n';

    cout << "Simulation time: "
         << result.totalSimulationTime << '\n';

    cout << "Peak memory used: "
         << result.peakUsedMemory << " MB\n";

    cout << "Allocation failures: "
         << result.allocationFailures << '\n';

    cout << fixed << setprecision(2);

    cout << "Average utilization: "
         << result.averageUtilization << "%\n";

    cout << "Average external fragmentation: "
         << result.averageExternalFragmentation << "%\n";

    cout << "Final score: "
         << result.finalScore << " / 100\n";
}

void AdaptiveEngine::run(
    int totalMemory,
    const vector<Process>& processes)
{
    if (totalMemory <= 0)
    {
        cout << "Error: total memory must be greater than zero.\n";
        return;
    }

    if (processes.empty())
    {
        cout << "Error: no workload is selected.\n";
        return;
    }

    FirstFit firstFit;
    BestFit bestFit;
    WorstFit worstFit;

    vector<AdaptiveResult> results;

    results.push_back(
        evaluateStrategy(
            totalMemory, processes, &firstFit, "First Fit"));

    results.push_back(
        evaluateStrategy(
            totalMemory, processes, &bestFit, "Best Fit"));

    results.push_back(
        evaluateStrategy(
            totalMemory, processes, &worstFit, "Worst Fit"));

    cout << "\n========================================\n";
    cout << "       ADAPTIVE STRATEGY ENGINE\n";
    cout << "========================================\n";

    cout << "Processes: " << processes.size() << '\n';
    cout << "Memory: " << totalMemory << " MB\n";

    for (const AdaptiveResult& result : results)
    {
        displayResult(result);
    }

    const AdaptiveResult* bestResult = &results[0];

    for (const AdaptiveResult& result : results)
    {
        if (result.finalScore > bestResult->finalScore ||
            (result.finalScore == bestResult->finalScore &&
             result.completedProcesses > bestResult->completedProcesses) ||
            (result.finalScore == bestResult->finalScore &&
             result.completedProcesses == bestResult->completedProcesses &&
             result.allocationFailures < bestResult->allocationFailures))
        {
            bestResult = &result;
        }
    }

    cout << "\n========================================\n";
    cout << "         ADAPTIVE RECOMMENDATION\n";
    cout << "========================================\n";

    cout << "Recommended strategy: "
         << bestResult->strategyName << '\n';

    cout << "Final score: "
         << fixed << setprecision(2)
         << bestResult->finalScore << " / 100\n";
}
