#ifndef MEMORYBLOCK_H
#define MEMORYBLOCK_H

class MemoryBlock
{
private:
    int startAddress;
    int size;
    bool free;
    int processId;

public:
    MemoryBlock();

    MemoryBlock(
        int start,
        int blockSize,
        bool isFree,
        int id
    );

    int getStartAddress() const;
    int getSize() const;
    bool isFree() const;
    int getProcessId() const;

    void setSize(int newSize);
    void setFree(bool status);
    void setProcessId(int id);

    void display() const;
};

#endif
