
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <stdexcept>

#include "WorkloadManager.h"
#include "Process.h"
#include "MemoryManager.h"
#include "MemoryBlock.h"
#include "AllocationStrategy.h"
#include "FirstFit.h"
#include "BestFit.h"
#include "WorstFit.h"
#include "StrategyComparison.h"
#include "AdaptiveEngine.h"
#include "ProcessSimulation.h"

using namespace std;

// Escape strings before placing them in JSON.
string escapeJSON(const string& value)
{
    string result;

    for (char ch : value)
    {
        if (ch == '"')
            result += "\\\"";
        else if (ch == '\\')
            result += "\\\\";
        else if (ch == '\n')
            result += "\\n";
        else if (ch == '\r')
            result += "\\r";
        else if (ch == '\t')
            result += "\\t";
        else
            result += ch;
    }

    return result;
}

// Load the requested workload.
bool loadWorkload(WorkloadManager& manager, const string& type)
{
    if (type == "Basic")
        return manager.loadBasicWorkload();

    if (type == "Fragmentation")
        return manager.loadFragmentationWorkload();

    if (type == "Mixed")
        return manager.loadMixedWorkload();

    if (type == "Saved")
        return manager.loadSavedWorkload();

    return false;
}

// Return workload information as JSON.
void printWorkload(const WorkloadManager& manager)
{
    const vector<Process>& processes = manager.getProcesses();

    cout << "{";
    cout << "\"success\":true,";
    cout << "\"workloadName\":\""
         << escapeJSON(manager.getWorkloadName()) << "\",";
    cout << "\"totalProcesses\":" << processes.size() << ",";
    cout << "\"processes\":[";

    for (size_t i = 0; i < processes.size(); ++i)
    {
        const Process& p = processes[i];

        if (i > 0)
            cout << ",";

        cout << "{";
        cout << "\"pid\":" << p.getProcessId() << ",";
        cout << "\"memory\":" << p.getMemoryRequired() << ",";
        cout << "\"arrival\":" << p.getArrivalTime() << ",";
        cout << "\"execution\":" << p.getExecutionTime() << ",";
        cout << "\"priority\":" << p.getPriority();
        cout << "}";
    }

    cout << "]}";
}

// Run the actual C++ memory allocation simulation.
void runSimulation(
    const WorkloadManager& workload,
    int totalMemory,
    const string& strategyName,
    AllocationStrategy* strategy)
{
    MemoryManager memoryManager(totalMemory, strategy);

    const vector<Process>& processes = workload.getProcesses();

    cout << "{";
    cout << "\"success\":true,";
    cout << "\"workloadName\":\""
         << escapeJSON(workload.getWorkloadName()) << "\",";
    cout << "\"strategy\":\"" << escapeJSON(strategyName) << "\",";
    cout << "\"totalMemory\":" << totalMemory << ",";
    cout << "\"totalProcesses\":" << processes.size() << ",";

    int successfulAllocations = 0;
    int failedAllocations = 0;

    cout << "\"allocationResults\":[";

    for (size_t i = 0; i < processes.size(); ++i)
    {
        const Process& process = processes[i];

        // Keep C++ allocation messages out of the JSON response.
        ostringstream allocationMessages;
        streambuf* originalBuffer = cout.rdbuf();

        cout.rdbuf(allocationMessages.rdbuf());

        bool allocated = memoryManager.allocate(
            process.getProcessId(),
            process.getMemoryRequired()
        );

        cout.rdbuf(originalBuffer);

        if (allocated)
            successfulAllocations++;
        else
            failedAllocations++;

        if (i > 0)
            cout << ",";

        cout << "{";
        cout << "\"pid\":" << process.getProcessId() << ",";
        cout << "\"memory\":" << process.getMemoryRequired() << ",";
        cout << "\"allocated\":" << (allocated ? "true" : "false");
        cout << "}";
    }

    const vector<MemoryBlock>& blocks = memoryManager.getBlocks();

    int allocatedMemory = 0;
    int freeMemory = 0;

    for (const MemoryBlock& block : blocks)
    {
        if (block.isFree())
            freeMemory += block.getSize();
        else
            allocatedMemory += block.getSize();
    }

    cout << "],";

    cout << "\"successfulAllocations\":"
         << successfulAllocations << ",";

    cout << "\"failedAllocations\":"
         << failedAllocations << ",";

    cout << "\"allocationFailures\":"
         << memoryManager.getAllocationFailures() << ",";

    cout << "\"allocatedMemory\":" << allocatedMemory << ",";
    cout << "\"freeMemory\":" << freeMemory << ",";

    cout << "\"utilizationPercentage\":"
         << (totalMemory > 0
             ? (allocatedMemory * 100.0 / totalMemory)
             : 0.0) << ",";

    cout << "\"blocks\":[";

    for (size_t i = 0; i < blocks.size(); ++i)
    {
        const MemoryBlock& block = blocks[i];

        if (i > 0)
            cout << ",";

        cout << "{";
        cout << "\"startAddress\":" << block.getStartAddress() << ",";
        cout << "\"size\":" << block.getSize() << ",";
        cout << "\"free\":" << (block.isFree() ? "true" : "false") << ",";
        cout << "\"processId\":" << block.getProcessId();
        cout << "}";
    }

    cout << "]}";
}

// Run all three strategies using the actual StrategyComparison class.
void runComparison(
    const WorkloadManager& workload,
    int totalMemory)
{
    StrategyComparison comparison;

    vector<StrategyResult> results =
        comparison.getComparisonResults(
            totalMemory,
            workload.getProcesses()
        );

    if (results.empty())
    {
        cout << "{\"success\":false,"
             << "\"message\":\"Unable to generate comparison results\"}";
        return;
    }

    // Select the recommended strategy using the existing tie-breaking rules.
    size_t bestIndex = 0;

    for (size_t i = 1; i < results.size(); ++i)
    {
        if (results[i].finalScore > results[bestIndex].finalScore)
        {
            bestIndex = i;
        }
        else if (results[i].finalScore == results[bestIndex].finalScore)
        {
            if (results[i].completedProcesses >
                results[bestIndex].completedProcesses)
            {
                bestIndex = i;
            }
            else if (
                results[i].completedProcesses ==
                results[bestIndex].completedProcesses &&
                results[i].allocationFailures <
                results[bestIndex].allocationFailures)
            {
                bestIndex = i;
            }
        }
    }

    cout << fixed << setprecision(2);

    cout << "{";
    cout << "\"success\":true,";
    cout << "\"workloadName\":\""
         << escapeJSON(workload.getWorkloadName()) << "\",";
    cout << "\"totalMemory\":" << totalMemory << ",";
    cout << "\"totalProcesses\":" << workload.getProcesses().size() << ",";
    cout << "\"results\":[";

    for (size_t i = 0; i < results.size(); ++i)
    {
        const StrategyResult& result = results[i];

        if (i > 0)
            cout << ",";

        cout << "{";

        cout << "\"strategyName\":\""
             << escapeJSON(result.strategyName) << "\",";

        cout << "\"completedProcesses\":"
             << result.completedProcesses << ",";

        cout << "\"totalSimulationTime\":"
             << result.totalSimulationTime << ",";

        cout << "\"peakUsedMemory\":"
             << result.peakUsedMemory << ",";

        cout << "\"allocationFailures\":"
             << result.allocationFailures << ",";

        cout << "\"averageUtilization\":"
             << result.averageUtilization << ",";

        cout << "\"averageExternalFragmentation\":"
             << result.averageExternalFragmentation << ",";

        cout << "\"utilizationScore\":"
             << result.utilizationScore << ",";

        cout << "\"fragmentationScore\":"
             << result.fragmentationScore << ",";

        cout << "\"failureScore\":"
             << result.failureScore << ",";

        cout << "\"completionScore\":"
             << result.completionScore << ",";

        cout << "\"finalScore\":"
             << result.finalScore;

        cout << "}";
    }

    cout << "],";

    cout << "\"recommendedStrategy\":\""
         << escapeJSON(results[bestIndex].strategyName) << "\",";

    cout << "\"recommendedScore\":"
         << results[bestIndex].finalScore;

    cout << "}";
}


void runAdaptive(
    const WorkloadManager& workload,
    int totalMemory)
{
    AdaptiveEngine engine;

    vector<AdaptiveResult> results =
        engine.getAdaptiveResults(
            totalMemory,
            workload.getProcesses()
        );

    if (results.empty())
    {
        cout << "{\"success\":false,"
             << "\"message\":\"Unable to generate adaptive results\"}";
        return;
    }

    size_t bestIndex = 0;

    for (size_t i = 1; i < results.size(); ++i)
    {
        if (results[i].finalScore > results[bestIndex].finalScore)
        {
            bestIndex = i;
        }
        else if (results[i].finalScore == results[bestIndex].finalScore)
        {
            if (results[i].completedProcesses >
                results[bestIndex].completedProcesses)
            {
                bestIndex = i;
            }
            else if (
                results[i].completedProcesses ==
                results[bestIndex].completedProcesses &&
                results[i].allocationFailures <
                results[bestIndex].allocationFailures)
            {
                bestIndex = i;
            }
        }
    }

    cout << fixed << setprecision(2);

    cout << "{";
    cout << "\"success\":true,";
    cout << "\"workloadName\":\""
         << escapeJSON(workload.getWorkloadName()) << "\",";
    cout << "\"totalMemory\":" << totalMemory << ",";
    cout << "\"totalProcesses\":" << workload.getProcesses().size() << ",";
    cout << "\"results\":[";

    for (size_t i = 0; i < results.size(); ++i)
    {
        const AdaptiveResult& result = results[i];

        if (i > 0)
            cout << ",";

        cout << "{";

        cout << "\"strategyName\":\""
             << escapeJSON(result.strategyName) << "\",";

        cout << "\"totalProcesses\":"
             << result.totalProcesses << ",";

        cout << "\"completedProcesses\":"
             << result.completedProcesses << ",";

        cout << "\"totalSimulationTime\":"
             << result.totalSimulationTime << ",";

        cout << "\"peakUsedMemory\":"
             << result.peakUsedMemory << ",";

        cout << "\"allocationFailures\":"
             << result.allocationFailures << ",";

        cout << "\"averageUtilization\":"
             << result.averageUtilization << ",";

        cout << "\"averageExternalFragmentation\":"
             << result.averageExternalFragmentation << ",";

        cout << "\"utilizationScore\":"
             << result.utilizationScore << ",";

        cout << "\"fragmentationScore\":"
             << result.fragmentationScore << ",";

        cout << "\"failureScore\":"
             << result.failureScore << ",";

        cout << "\"completionScore\":"
             << result.completionScore << ",";

        cout << "\"finalScore\":"
             << result.finalScore;

        cout << "}";
    }

    cout << "],";

    cout << "\"recommendedStrategy\":\""
         << escapeJSON(results[bestIndex].strategyName) << "\",";

    cout << "\"recommendedScore\":"
         << results[bestIndex].finalScore;

    cout << "}";
}


// ============================================================
// CPU SCHEDULING - JSON OUTPUT FOR GUI
// ============================================================

void runCPUScheduling(
    const WorkloadManager& workload,
    int totalMemory)
{
    ProcessSimulation simulation(
        workload.getProcesses(),
        workload.getWorkloadName()
    );

    vector<CPUSchedulingResult> results =
        simulation.getCPUSchedulingResults(totalMemory);

    cout << fixed << setprecision(2);

    cout << "{";
    cout << "\"success\":true,";
    cout << "\"workloadName\":\""
         << escapeJSON(workload.getWorkloadName()) << "\",";
    cout << "\"totalMemory\":" << totalMemory << ",";
    cout << "\"totalProcesses\":" << workload.getProcesses().size() << ",";
    cout << "\"results\":[";

    for (size_t i = 0; i < results.size(); ++i)
    {
        const CPUSchedulingResult& result = results[i];

        if (i > 0)
            cout << ",";

        cout << "{";

        cout << "\"schedulerName\":\""
             << escapeJSON(result.schedulerName) << "\",";

        cout << "\"totalSimulationTime\":"
             << result.totalSimulationTime << ",";

        cout << "\"completedProcesses\":"
             << result.completedProcesses << ",";

        cout << "\"totalProcesses\":"
             << result.totalProcesses << ",";

        cout << "\"contextSwitches\":"
             << result.contextSwitches << ",";

        cout << "\"averageWaitingTime\":"
             << result.averageWaitingTime << ",";

        cout << "\"averageTurnaroundTime\":"
             << result.averageTurnaroundTime << ",";

        cout << "\"averageResponseTime\":"
             << result.averageResponseTime << ",";

        cout << "\"cpuUtilization\":"
             << result.cpuUtilization;

        cout << "}";
    }

    cout << "]}";
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        cout << "{\"success\":false,"
             << "\"message\":\"Missing workload type\"}";
        return 1;
    }

    string type = argv[1];

    WorkloadManager manager;

    if (!loadWorkload(manager, type) || !manager.hasWorkload())
    {
        cout << "{\"success\":false,"
             << "\"message\":\"Unable to load workload\"}";
        return 1;
    }

    // One argument = return workload data.
    if (argc == 2)
    {
        printWorkload(manager);
        return 0;
    }

    // Simulation/comparison requires memory size and mode.
    if (argc < 4)
    {
        cout << "{\"success\":false,"
             << "\"message\":\"Missing memory size or strategy\"}";
        return 1;
    }

    int totalMemory = 0;

    try
    {
        size_t position = 0;
        string memoryArgument = argv[2];

        totalMemory = stoi(memoryArgument, &position);

        if (position != memoryArgument.size() || totalMemory <= 0)
            throw invalid_argument("Invalid memory size");
    }
    catch (...)
    {
        cout << "{\"success\":false,"
             << "\"message\":\"Enter a valid positive memory size\"}";
        return 1;
    }

    string strategyName = argv[3];

    // Compare All mode.
    if (strategyName == "COMPARE")
    {
        runComparison(manager, totalMemory);
        return 0;
    }
    // CPU Scheduling mode.
if (strategyName == "CPU")
{
    runCPUScheduling(manager, totalMemory);
    return 0;
}

    if (strategyName == "ADAPTIVE")
{
    runAdaptive(manager, totalMemory);
    return 0;
}

    // Existing individual simulation modes.
    FirstFit firstFit;
    BestFit bestFit;
    WorstFit worstFit;

    AllocationStrategy* strategy = nullptr;

    if (strategyName == "First Fit")
        strategy = &firstFit;
    else if (strategyName == "Best Fit")
        strategy = &bestFit;
    else if (strategyName == "Worst Fit")
        strategy = &worstFit;
    else
    {
        cout << "{\"success\":false,"
             << "\"message\":\"Invalid allocation strategy\"}";
        return 1;
    }

    runSimulation(
        manager,
        totalMemory,
        strategyName,
        strategy
    );

    return 0;
}
