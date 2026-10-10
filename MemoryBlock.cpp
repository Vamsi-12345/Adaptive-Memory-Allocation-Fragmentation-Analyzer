#include "MemoryBlock.h"
#include <iostream>

using namespace std;

// Default constructor
MemoryBlock::MemoryBlock()
{
    startAddress = 0;
    size = 0;
    free = true;
    processId = -1;
}

// Parameterized constructor
MemoryBlock::MemoryBlock(
    int start,
    int blockSize,
    bool isFree,
    int id)
{
    startAddress = start;
    size = blockSize;
    free = isFree;
    processId = id;
}

// Get start address
int MemoryBlock::getStartAddress() const
{
    return startAddress;
}

// Get block size
int MemoryBlock::getSize() const
{
    return size;
}

// Check whether block is free
bool MemoryBlock::isFree() const
{
    return free;
}

// Get process ID
int MemoryBlock::getProcessId() const
{
    return processId;
}

// Set block size
void MemoryBlock::setSize(int newSize)
{
    size = newSize;
}

// Set free/allocated status
void MemoryBlock::setFree(bool status)
{
    free = status;
}

// Set process ID
void MemoryBlock::setProcessId(int id)
{
    processId = id;
}

// Display block information
void MemoryBlock::display() const
{
    cout << "Start Address: " << startAddress << endl;
    cout << "Size: " << size << " MB" << endl;

    cout << "Status: "
         << (free ? "Free" : "Allocated")
         << endl;

    if (!free)
    {
        cout << "Process ID: "
             << processId
             << endl;
    }
}
