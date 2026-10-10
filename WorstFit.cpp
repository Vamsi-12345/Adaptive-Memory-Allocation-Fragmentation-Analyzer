#include "WorstFit.h"

int WorstFit::findBlock(
    const std::vector<MemoryBlock>& blocks,
    int memoryRequired)
{
    int worstIndex = -1;
    int worstSize = -1;

    for (int i = 0;
         i < static_cast<int>(blocks.size());
         i++)
    {
        if (blocks[i].isFree() &&
            blocks[i].getSize() >= memoryRequired)
        {
            if (blocks[i].getSize() > worstSize)
            {
                worstSize = blocks[i].getSize();
                worstIndex = i;
            }
        }
    }

    return worstIndex;
}
