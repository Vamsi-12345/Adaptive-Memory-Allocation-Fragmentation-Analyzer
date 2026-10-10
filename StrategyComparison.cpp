
#include "StrategyComparison.h"

#include <iostream>
#include <iomanip>
#include <algorithm>

#include "FirstFit.h"
#include "BestFit.h"
#include "WorstFit.h"

using namespace std;

StrategyResult::StrategyResult()
{
    strategyName = "";
    totalMemory = 0;
    totalProcesses = 0;
    completedProcesses = 0;
    totalSimulationTime = 0;
    peakUsedMemory = 0;
    allocationFailures = 0;

    averageUtilization = 0.0;
    averageExternalFragmentation = 0.0;

    utilizationScore = 0.0;
    fragmentationScore = 0.0;
    failureScore = 0.0;
    completionScore = 0.0;
    finalScore = 0.0;
}

StrategyResult StrategyComparison::runStrategy(
    int totalMemory,
    const vector<Process>& processes,
    AllocationStrategy* strategy,
    const string& strategyName)
{
    StrategyResult result;

    result.strategyName = strategyName;
    result.totalMemory = totalMemory;
    result.totalProcesses = static_cast<int>(processes.size());

    if (totalMemory <= 0 || processes.empty() || strategy == nullptr)
    {
        return result;
    }

    // Every strategy receives its own fresh memory and process copies.
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
        bool runningProcessExists = false;
        bool futureProcessExists = false;
        bool waitingProcessExists = false;

        // Step 1: Admit and allocate processes whose arrival time has passed.
        for (Process& process : simulationProcesses)
        {
            if (process.getStatus() == "Completed")
            {
                continue;
            }

            if (process.getArrivalTime() > currentTime)
            {
                futureProcessExists = true;
                continue;
            }

            if (process.getStatus() == "Waiting" ||
                process.getStatus() == "WaitingForMemory")
            {
                int blockIndex = strategy->findBlock(
                    memory.getBlocks(),
                    process.getMemoryRequired()
                );

                if (blockIndex == -1)
                {
                    // Count only the first failed attempt for this waiting period.
                    if (process.getStatus() != "WaitingForMemory")
                    {
                        allocationFailures++;
                    }

                    process.setStatus("WaitingForMemory");
                    waitingProcessExists = true;
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
                    waitingProcessExists = true;
                    continue;
                }
            }
        }

        // Step 2: Execute running processes for one time unit.
        for (Process& process : simulationProcesses)
        {
            if (process.getStatus() != "Running")
            {
                continue;
            }

            runningProcessExists = true;
            process.decreaseRemainingTime();

            if (process.getRemainingTime() <= 0)
            {
                memory.deallocate(process.getProcessId());
                process.setStatus("Completed");
                completed++;
            }
        }

        // Step 3: Collect memory statistics after this time step.
        int usedMemory = 0;
        int freeMemory = 0;
        int largestFreeBlock = 0;

        for (const MemoryBlock& block : memory.getBlocks())
        {
            if (block.isFree())
            {
                freeMemory += block.getSize();

                if (block.getSize() > largestFreeBlock)
                {
                    largestFreeBlock = block.getSize();
                }
            }
            else
            {
                usedMemory += block.getSize();
            }
        }

        if (usedMemory > peakUsedMemory)
        {
            peakUsedMemory = usedMemory;
        }

        utilizationTotal +=
            (static_cast<double>(usedMemory) / totalMemory) * 100.0;

        double externalFragmentation = 0.0;

        if (freeMemory > 0)
        {
            externalFragmentation =
                (static_cast<double>(freeMemory - largestFreeBlock) /
                 freeMemory) * 100.0;
        }

        fragmentationTotal += externalFragmentation;
        samples++;

        currentTime++;

        // Stop if no process can make progress.
        if (completed < static_cast<int>(simulationProcesses.size()) &&
            !runningProcessExists &&
            !futureProcessExists &&
            waitingProcessExists)
        {
            break;
        }

        // Also prevent an infinite loop if all remaining processes are blocked.
        if (completed < static_cast<int>(simulationProcesses.size()) &&
            !runningProcessExists &&
            !futureProcessExists)
        {
            break;
        }
    }

    result.completedProcesses = completed;
    result.totalSimulationTime = currentTime;
    result.peakUsedMemory = peakUsedMemory;
    result.allocationFailures = allocationFailures;

    if (samples > 0)
    {
        result.averageUtilization = utilizationTotal / samples;
        result.averageExternalFragmentation = fragmentationTotal / samples;
    }

    // Score components are normalized to a 0-100 scale.
    result.utilizationScore = result.averageUtilization;

    result.fragmentationScore =
        100.0 - result.averageExternalFragmentation;

    if (result.fragmentationScore < 0.0)
    {
        result.fragmentationScore = 0.0;
    }

    result.failureScore =
        100.0 - (allocationFailures * 10.0);

    if (result.failureScore < 0.0)
    {
        result.failureScore = 0.0;
    }

    if (result.totalProcesses > 0)
    {
        result.completionScore =
            (static_cast<double>(completed) /
             result.totalProcesses) * 100.0;
    }

    // Prioritize utilization and fragmentation while considering failures
    // and the percentage of processes completed.
    result.finalScore =
        (0.35 * result.utilizationScore) +
        (0.30 * result.fragmentationScore) +
        (0.20 * result.failureScore) +
        (0.15 * result.completionScore);

    return result;
}

void StrategyComparison::runIndividualStrategy(
    int totalMemory,
    const vector<Process>& processes,
    int strategyChoice)
{
    FirstFit firstFit;
    BestFit bestFit;
    WorstFit worstFit;

    StrategyResult result;

    switch (strategyChoice)
    {
        case 1:
            result = runStrategy(
                totalMemory, processes, &firstFit, "First Fit");
            break;

        case 2:
            result = runStrategy(
                totalMemory, processes, &bestFit, "Best Fit");
            break;

        case 3:
            result = runStrategy(
                totalMemory, processes, &worstFit, "Worst Fit");
            break;

        default:
            cout << "Invalid strategy choice.\n";
            return;
    }

    displayReport(vector<StrategyResult>(1, result));
}

void StrategyComparison::compareAll(
    int totalMemory,
    const vector<Process>& processes)
{
    if (totalMemory <= 0)
    {
        cout << "Total memory must be greater than zero.\n";
        return;
    }

    if (processes.empty())
    {
        cout << "No workload is selected. Select a workload first.\n";
        return;
    }

    FirstFit firstFit;
    BestFit bestFit;
    WorstFit worstFit;

    vector<StrategyResult> results;

    cout << "\n========================================\n";
    cout << "        STRATEGY COMPARISON\n";
    cout << "========================================\n";

    cout << "Processes in workload: " << processes.size() << '\n';
    cout << "Total memory: " << totalMemory << " MB\n";
    cout << "Running all strategies on the same workload...\n";

    results.push_back(
        runStrategy(totalMemory, processes, &firstFit, "First Fit"));

    results.push_back(
        runStrategy(totalMemory, processes, &bestFit, "Best Fit"));

    results.push_back(
        runStrategy(totalMemory, processes, &worstFit, "Worst Fit"));

    displayReport(results);
    giveSuggestion(results);
}

void StrategyComparison::displayReport(
    const vector<StrategyResult>& results) const
{
    cout << fixed << setprecision(2);

    cout << "\n========================================\n";
    cout << "           COMPARISON REPORT\n";
    cout << "========================================\n";

    for (const StrategyResult& result : results)
    {
        cout << "\nStrategy: " << result.strategyName << '\n';
        cout << "----------------------------------------\n";

        cout << "Processes completed     : "
             << result.completedProcesses << " / "
             << result.totalProcesses << '\n';

        cout << "Simulation time         : "
             << result.totalSimulationTime << '\n';

        cout << "Peak memory used        : "
             << result.peakUsedMemory << " MB\n";

        cout << "Average utilization     : "
             << result.averageUtilization << "%\n";

        cout << "Average external frag.  : "
             << result.averageExternalFragmentation << "%\n";

        cout << "Allocation failures     : "
             << result.allocationFailures << '\n';

        cout << "Utilization score       : "
             << result.utilizationScore << '\n';

        cout << "Fragmentation score     : "
             << result.fragmentationScore << '\n';

        cout << "Failure score           : "
             << result.failureScore << '\n';

        cout << "Completion score        : "
             << result.completionScore << '\n';

        cout << "Final score             : "
             << result.finalScore << " / 100\n";
    }
}

void StrategyComparison::giveSuggestion(
    const vector<StrategyResult>& results) const
{
    if (results.empty())
    {
        cout << "\nNo results are available to compare.\n";
        return;
    }

    const StrategyResult* bestResult = &results[0];

    for (const StrategyResult& result : results)
    {
        if (result.finalScore > bestResult->finalScore)
        {
            bestResult = &result;
        }
        else if (result.finalScore == bestResult->finalScore &&
                 result.completedProcesses > bestResult->completedProcesses)
        {
            bestResult = &result;
        }
        else if (result.finalScore == bestResult->finalScore &&
                 result.completedProcesses == bestResult->completedProcesses &&
                 result.allocationFailures < bestResult->allocationFailures)
        {
            bestResult = &result;
        }
    }

    cout << "\n========================================\n";
    cout << "          STRATEGY RECOMMENDATION\n";
    cout << "========================================\n";

    cout << "Recommended strategy: "
         << bestResult->strategyName << '\n';

    cout << "Final score: "
         << fixed << setprecision(2)
         << bestResult->finalScore << " / 100\n";

    cout << "Processes completed: "
         << bestResult->completedProcesses << " / "
         << bestResult->totalProcesses << '\n';

    cout << "Allocation failures: "
         << bestResult->allocationFailures << '\n';
}
