#ifndef BESTFIT_H
#define BESTFIT_H

#include "AllocationStrategy.h"

class BestFit : public AllocationStrategy
{
public:
    int findBlock(
        const std::vector<MemoryBlock>& blocks,
        int memoryRequired
    ) override;

    std::string getName() const override
    {
        return "Best Fit";
    }
};

#endif
