#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <vector>
#include "Process.h"

class ProcessManager
{
private:
    std::vector<Process> processes;

public:
    ProcessManager();

    void addProcess(const Process& process);

    bool removeProcess(int processId);

    Process* getProcess(int processId);

    const std::vector<Process>& getProcesses() const;

    void displayProcesses() const;

    void clearProcesses();

    int getProcessCount() const;
};

#endif
