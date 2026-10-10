#ifndef PROCESS_H
#define PROCESS_H

#include <string>

class Process
{
private:
    int processId;
    int memoryRequired;
    int arrivalTime;
    int executionTime;
    int remainingTime;
    int priority;
    std::string status;

public:
    Process();

    Process(
        int id,
        int memory,
        int arrival,
        int execution,
        int p
    );

    int getProcessId() const;
    int getMemoryRequired() const;
    int getArrivalTime() const;
    int getExecutionTime() const;
    int getRemainingTime() const;
    int getPriority() const;
    std::string getStatus() const;

    void setStatus(
        const std::string& newStatus
    );

    void decreaseRemainingTime();

    void resetExecution();

    void display() const;
};

#endif
