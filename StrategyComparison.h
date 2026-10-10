
#ifndef STRATEGYCOMPARISON_H
#define STRATEGYCOMPARISON_H

#include <string>
#include <vector>
#include "Process.h"
#include "MemoryManager.h"
#include "AllocationStrategy.h"

struct StrategyResult
{
    std::string strategyName;

    int totalMemory;
    int totalProcesses;
    int completedProcesses;
    int totalSimulationTime;
    int peakUsedMemory;
    int allocationFailures;

    double averageUtilization;
    double averageExternalFragmentation;

    double utilizationScore;
    double fragmentationScore;
    double failureScore;
    double completionScore;
    double finalScore;

    StrategyResult();
};

class StrategyComparison
{
private:
    StrategyResult runStrategy(
        int totalMemory,
        const std::vector<Process>& processes,
        AllocationStrategy* strategy,
        const std::string& strategyName
    );

    void displayReport(
        const std::vector<StrategyResult>& results
    ) const;

    void giveSuggestion(
        const std::vector<StrategyResult>& results
    ) const;

public:
    void runIndividualStrategy(
        int totalMemory,
        const std::vector<Process>& processes,
        int strategyChoice
    );

    void compareAll(
        int totalMemory,
        const std::vector<Process>& processes
    );
};

#endif
