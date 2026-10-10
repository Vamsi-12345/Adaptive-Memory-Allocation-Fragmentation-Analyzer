
#ifndef ADAPTIVEENGINE_H
#define ADAPTIVEENGINE_H

#include <string>
#include <vector>

#include "Process.h"
#include "AllocationStrategy.h"

struct AdaptiveResult
{
    std::string strategyName;

    int totalMemory;
    int totalProcesses;
    int completedProcesses;
    int totalSimulationTime;
    int allocationFailures;
    int peakUsedMemory;

    double averageUtilization;
    double averageExternalFragmentation;

    double utilizationScore;
    double fragmentationScore;
    double failureScore;
    double completionScore;
    double finalScore;
};

class AdaptiveEngine
{
private:
    AdaptiveResult evaluateStrategy(
        int totalMemory,
        const std::vector<Process>& processes,
        AllocationStrategy* strategy,
        const std::string& strategyName
    );

    void displayResult(
        const AdaptiveResult& result
    ) const;

public:
    void run(
        int totalMemory,
        const std::vector<Process>& processes
    );
};

#endif
