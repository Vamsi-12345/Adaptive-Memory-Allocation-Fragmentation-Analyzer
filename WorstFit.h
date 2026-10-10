#ifndef WORSTFIT_H
#define WORSTFIT_H

#include "AllocationStrategy.h"

class WorstFit : public AllocationStrategy
{
public:
    int findBlock(
        const std::vector<MemoryBlock>& blocks,
        int memoryRequired
    ) override;

    std::string getName() const override
    {
        return "Worst Fit";
    }
};

#endif
