#include "Process.h"
#include <iostream>

using namespace std;

Process::Process()
{
    processId = 0;
    memoryRequired = 0;
    arrivalTime = 0;
    executionTime = 0;
    priority = 0;
}

Process::Process(int id, int memory, int arrival,
                 int execution, int priority)
{
    processId = id;
    memoryRequired = memory;
    arrivalTime = arrival;
    executionTime = execution;
    this->priority = priority;
}

int Process::getProcessId() const
{
    return processId;
}

int Process::getMemoryRequired() const
{
    return memoryRequired;
}

int Process::getArrivalTime() const
{
    return arrivalTime;
}

int Process::getExecutionTime() const
{
    return executionTime;
}

int Process::getPriority() const
{
    return priority;
}

void Process::display() const
{
    cout << "Process ID: " << processId << endl;
    cout << "Memory Required: " << memoryRequired << " MB" << endl;
    cout << "Arrival Time: " << arrivalTime << endl;
    cout << "Execution Time: " << executionTime << endl;
    cout << "Priority: " << priority << endl;
}