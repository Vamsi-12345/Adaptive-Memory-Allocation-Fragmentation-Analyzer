#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <vector>
#include "MemoryBlock.h"
#include "AllocationStrategy.h"

class MemoryManager
{
private:
    int totalMemory;
    std::vector<MemoryBlock> blocks;
    AllocationStrategy* strategy;

    int allocationFailures;

public:
    MemoryManager(
        int memorySize,
        AllocationStrategy* allocationStrategy
    );

    void displayMemory() const;

    bool canAllocate(int memoryRequired) const;

    bool allocate(
        int processId,
        int memoryRequired
    );

    bool deallocate(int processId);

    void mergeFreeBlocks();

    // Functions for strategy comparison
    int getTotalMemory() const;

    const std::vector<MemoryBlock>& getBlocks() const;

    int getAllocationFailures() const;
};

#endif
