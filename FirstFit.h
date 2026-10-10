#ifndef FIRSTFIT_H
#define FIRSTFIT_H

#include "AllocationStrategy.h"

class FirstFit : public AllocationStrategy
{
public:
    int findBlock(
        const std::vector<MemoryBlock>& blocks,
        int memoryRequired
    ) override;

    std::string getName() const override
    {
        return "First Fit";
    }
};

#endif
