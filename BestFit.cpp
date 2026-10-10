#include "BestFit.h"

int BestFit::findBlock(
    const std::vector<MemoryBlock>& blocks,
    int memoryRequired)
{
    int bestIndex = -1;
    int bestSize = 999999;

    for (int i = 0;
         i < static_cast<int>(blocks.size());
         i++)
    {
        if (blocks[i].isFree() &&
            blocks[i].getSize() >= memoryRequired)
        {
            if (blocks[i].getSize() < bestSize)
            {
                bestSize = blocks[i].getSize();
                bestIndex = i;
            }
        }
    }

    return bestIndex;
}
