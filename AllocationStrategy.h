#ifndef ALLOCATIONSTRATEGY_H
#define ALLOCATIONSTRATEGY_H

#include <vector>
#include <string>
#include "MemoryBlock.h"

class AllocationStrategy
{
public:
    virtual ~AllocationStrategy() {}

    virtual int findBlock(
        const std::vector<MemoryBlock>& blocks,
        int memoryRequired
    ) = 0;

    virtual std::string getName() const = 0;
};

#endif
