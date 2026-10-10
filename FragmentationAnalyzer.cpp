#include "FragmentationAnalyzer.h"

#include <iostream>
#include <iomanip>

using namespace std;

FragmentationReport FragmentationAnalyzer::analyze(
    const MemoryManager& memory) const
{
    FragmentationReport report;

    report.totalMemory = memory.getTotalMemory();

    report.usedMemory = 0;
    report.freeMemory = 0;
    report.largestFreeBlock = 0;
    report.freeBlockCount = 0;

    report.allocationFailures =
        memory.getAllocationFailures();

    const vector<MemoryBlock>& blocks =
        memory.getBlocks();

    // Analyze every memory block
    for (const MemoryBlock& block : blocks)
    {
        if (block.isFree())
        {
            report.freeMemory += block.getSize();

            report.freeBlockCount++;

            if (block.getSize() >
                report.largestFreeBlock)
            {
                report.largestFreeBlock =
                    block.getSize();
            }
        }
        else
        {
            report.usedMemory += block.getSize();
        }
    }

    // Calculate memory utilization
    if (report.totalMemory > 0)
    {
        report.utilization =
            (static_cast<double>(report.usedMemory) /
             report.totalMemory) * 100.0;
    }
    else
    {
        report.utilization = 0.0;
    }

    // Calculate external fragmentation
    //
    // External Fragmentation =
    // (Total Free Memory - Largest Free Block)
    // ----------------------------------------- x 100
    //          Total Free Memory
    //
    if (report.freeMemory > 0)
    {
        report.externalFragmentation =
            (static_cast<double>(
                report.freeMemory -
                report.largestFreeBlock) /
             report.freeMemory) * 100.0;
    }
    else
    {
        report.externalFragmentation = 0.0;
    }

    return report;
}

void FragmentationAnalyzer::displayReport(
    const FragmentationReport& report) const
{
    cout << endl;
    cout << "==============================================" << endl;
    cout << "        FRAGMENTATION ANALYSIS REPORT" << endl;
    cout << "==============================================" << endl;

    cout << fixed << setprecision(2);

    cout << "Total Memory              : "
         << report.totalMemory
         << " MB" << endl;

    cout << "Used Memory               : "
         << report.usedMemory
         << " MB" << endl;

    cout << "Free Memory               : "
         << report.freeMemory
         << " MB" << endl;

    cout << "Largest Free Block        : "
         << report.largestFreeBlock
         << " MB" << endl;

    cout << "Free Block Count          : "
         << report.freeBlockCount
         << endl;

    cout << "Allocation Failures       : "
         << report.allocationFailures
         << endl;

    cout << "Memory Utilization        : "
         << report.utilization
         << "%" << endl;

    cout << "External Fragmentation    : "
         << report.externalFragmentation
         << "%" << endl;

    cout << "==============================================" << endl;
}
