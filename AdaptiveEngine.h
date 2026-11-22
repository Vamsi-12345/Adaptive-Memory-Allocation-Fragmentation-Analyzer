
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
        const std::string& strategyName,
        const std::vector<std::string>& eventHistory
    );

    void displayResult(
        const AdaptiveResult& result
    ) const;

public:
    // New function for GUI integration
    std::vector<AdaptiveResult> getAdaptiveResults(
        int totalMemory,
        const std::vector<Process>& processes,
        const std::vector<std::string>& eventHistory =
            std::vector<std::string>()
    );

    void run(
        int totalMemory,
        const std::vector<Process>& processes,
        const std::vector<std::string>& eventHistory =
            std::vector<std::string>()
    );
};

#endif
