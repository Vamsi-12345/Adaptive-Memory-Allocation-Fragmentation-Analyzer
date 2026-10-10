#include "FirstFit.h"

int FirstFit::findBlock(
    const std::vector<MemoryBlock>& blocks,
    int memoryRequired)
{
    for (int i = 0;
         i < static_cast<int>(blocks.size());
         i++)
    {
        if (blocks[i].isFree() &&
            blocks[i].getSize() >= memoryRequired)
        {
            return i;
        }
    }

    return -1;
}
