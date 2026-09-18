class Process
{
private:
    int processId;
    int memoryRequired;
    int arrivalTime;
    int executionTime;
    int priority;

public:
    Process();

    Process(int id, int memory, int arrival,
            int execution, int priority);

    int getProcessId() const;
    int getMemoryRequired() const;
    int getArrivalTime() const;
    int getExecutionTime() const;
    int getPriority() const;

    void display() const;
};