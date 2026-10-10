#ifndef FRAGMENTATIONANALYZER_H
#define FRAGMENTATIONANALYZER_H

#include "MemoryManager.h"

struct FragmentationReport
{
    int totalMemory;
    int usedMemory;
    int freeMemory;
    int largestFreeBlock;
    int freeBlockCount;
    int allocationFailures;

    double utilization;
    double externalFragmentation;
};

class FragmentationAnalyzer
{
public:
    FragmentationReport analyze(
        const MemoryManager& memory
    ) const;

    void displayReport(
        const FragmentationReport& report
    ) const;
};

#endif
