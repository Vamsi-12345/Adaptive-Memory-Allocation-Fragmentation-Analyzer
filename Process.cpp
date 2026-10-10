#include "Process.h"
#include <iostream>

using namespace std;

// ============================================================
// DEFAULT CONSTRUCTOR
// ============================================================

Process::Process()
{
    processId = 0;
    memoryRequired = 0;
    arrivalTime = 0;
    executionTime = 0;
    remainingTime = 0;
    priority = 0;
    status = "Waiting";
}

// ============================================================
// PARAMETERIZED CONSTRUCTOR
// ============================================================

Process::Process(
    int id,
    int memory,
    int arrival,
    int execution,
    int processPriority)
{
    processId = id;
    memoryRequired = memory;
    arrivalTime = arrival;
    executionTime = execution;
    remainingTime = execution;
    priority = processPriority;
    status = "Waiting";
}

// ============================================================
// GET PROCESS ID
// ============================================================

int Process::getProcessId() const
{
    return processId;
}

// ============================================================
// GET MEMORY
// ============================================================

int Process::getMemoryRequired() const
{
    return memoryRequired;
}

// ============================================================
// GET ARRIVAL TIME
// ============================================================

int Process::getArrivalTime() const
{
    return arrivalTime;
}

// ============================================================
// GET EXECUTION TIME
// ============================================================

int Process::getExecutionTime() const
{
    return executionTime;
}

// ============================================================
// GET REMAINING TIME
// ============================================================

int Process::getRemainingTime() const
{
    return remainingTime;
}

// ============================================================
// GET PRIORITY
// ============================================================

int Process::getPriority() const
{
    return priority;
}

// ============================================================
// GET STATUS
// ============================================================

string Process::getStatus() const
{
    return status;
}

// ============================================================
// SET STATUS
// ============================================================

void Process::setStatus(
    const string& newStatus)
{
    status = newStatus;
}

// ============================================================
// DECREASE REMAINING TIME
// ============================================================

void Process::decreaseRemainingTime()
{
    if (remainingTime > 0)
    {
        remainingTime--;
    }
}

// ============================================================
// RESET EXECUTION
// ============================================================

void Process::resetExecution()
{
    remainingTime = executionTime;
    status = "Waiting";
}

// ============================================================
// DISPLAY
// ============================================================

void Process::display() const
{
    cout << "Process ID       : "
         << processId
         << endl;

    cout << "Memory Required  : "
         << memoryRequired
         << " MB"
         << endl;

    cout << "Arrival Time     : "
         << arrivalTime
         << endl;

    cout << "Execution Time   : "
         << executionTime
         << endl;

    cout << "Remaining Time   : "
         << remainingTime
         << endl;

    cout << "Priority         : "
         << priority
         << endl;

    cout << "Status           : "
         << status
         << endl;
}
