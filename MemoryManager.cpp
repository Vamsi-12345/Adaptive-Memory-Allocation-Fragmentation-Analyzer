#include "MemoryManager.h"
#include <iostream>

using namespace std;

MemoryManager::MemoryManager(
    int memorySize,
    AllocationStrategy* allocationStrategy)
{
    totalMemory = memorySize;
    strategy = allocationStrategy;
    allocationFailures = 0;

    blocks.push_back(
        MemoryBlock(
            0,
            totalMemory,
            true,
            -1
        )
    );
}

void MemoryManager::displayMemory() const
{
    cout << "Total Simulated Memory: "
         << totalMemory
         << " MB"
         << endl;

    cout << "Memory Blocks:" << endl;

    for (const MemoryBlock& block : blocks)
    {
        block.display();
        cout << endl;
    }
}

bool MemoryManager::canAllocate(
    int memoryRequired) const
{
    if (memoryRequired <= 0)
    {
        return false;
    }

    if (memoryRequired > totalMemory)
    {
        return false;
    }

    if (strategy == nullptr)
    {
        return false;
    }

    // IMPORTANT:
    // Check whether a suitable CONTIGUOUS free block exists.
    int index = strategy->findBlock(
        blocks,
        memoryRequired
    );

    return index != -1;
}

bool MemoryManager::allocate(
    int processId,
    int memoryRequired)
{
    if (memoryRequired <= 0)
    {
        allocationFailures++;

        cout << "Allocation Failed: "
             << "Memory requirement must be greater than 0."
             << endl;

        return false;
    }

    if (memoryRequired > totalMemory)
    {
        allocationFailures++;

        cout << "Allocation Failed: "
             << "Requested memory exceeds total memory."
             << endl;

        return false;
    }

    if (strategy == nullptr)
    {
        allocationFailures++;

        cout << "Allocation Failed: "
             << "Allocation strategy is not available."
             << endl;

        return false;
    }

    int index = strategy->findBlock(
        blocks,
        memoryRequired
    );

    if (index == -1)
    {
        allocationFailures++;

        return false;
    }

    int originalSize =
        blocks[index].getSize();

    int startAddress =
        blocks[index].getStartAddress();

    blocks[index].setSize(
        memoryRequired
    );

    blocks[index].setFree(false);

    blocks[index].setProcessId(
        processId
    );

    if (originalSize > memoryRequired)
    {
        MemoryBlock remainingBlock(
            startAddress + memoryRequired,
            originalSize - memoryRequired,
            true,
            -1
        );

        blocks.insert(
            blocks.begin() + index + 1,
            remainingBlock
        );
    }

    return true;
}

bool MemoryManager::deallocate(
    int processId)
{
    int blockIndex = -1;

    for (int i = 0;
         i < static_cast<int>(blocks.size());
         i++)
    {
        if (!blocks[i].isFree() &&
            blocks[i].getProcessId() == processId)
        {
            blockIndex = i;
            break;
        }
    }

    if (blockIndex == -1)
    {
        cout << "Deallocation Failed: Process "
             << processId
             << " not found."
             << endl;

        return false;
    }

    blocks[blockIndex].setFree(true);
    blocks[blockIndex].setProcessId(-1);

    mergeFreeBlocks();

    return true;
}

void MemoryManager::mergeFreeBlocks()
{
    if (blocks.size() < 2)
    {
        return;
    }

    for (int i = 0;
         i < static_cast<int>(blocks.size()) - 1;
         i++)
    {
        if (blocks[i].isFree() &&
            blocks[i + 1].isFree())
        {
            int mergedSize =
                blocks[i].getSize() +
                blocks[i + 1].getSize();

            blocks[i].setSize(
                mergedSize
            );

            blocks.erase(
                blocks.begin() + i + 1
            );

            i--;
        }
    }
}

int MemoryManager::getTotalMemory() const
{
    return totalMemory;
}

const vector<MemoryBlock>&
MemoryManager::getBlocks() const
{
    return blocks;
}

int MemoryManager::getAllocationFailures() const
{
    return allocationFailures;
}
