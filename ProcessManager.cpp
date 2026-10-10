#include "ProcessManager.h"
#include <iostream>

using namespace std;

ProcessManager::ProcessManager()
{
}

void ProcessManager::addProcess(const Process& process)
{
    processes.push_back(process);
}

bool ProcessManager::removeProcess(int processId)
{
    for (auto it = processes.begin();
         it != processes.end();
         ++it)
    {
        if (it->getProcessId() == processId)
        {
            processes.erase(it);
            return true;
        }
    }

    return false;
}

Process* ProcessManager::getProcess(int processId)
{
    for (int i = 0;
         i < static_cast<int>(processes.size());
         i++)
    {
        if (processes[i].getProcessId() == processId)
        {
            return &processes[i];
        }
    }

    return nullptr;
}

const vector<Process>& ProcessManager::getProcesses() const
{
    return processes;
}

void ProcessManager::displayProcesses() const
{
    cout << endl;
    cout << "==============================================" << endl;
    cout << "              PROCESS LIST" << endl;
    cout << "==============================================" << endl;

    if (processes.empty())
    {
        cout << "No processes available." << endl;
        return;
    }

    cout << "---------------------------------------------------------------" << endl;
    cout << "PID\tMemory\tArrival\tExecution\tPriority\tStatus" << endl;
    cout << "---------------------------------------------------------------" << endl;

    for (const Process& process : processes)
    {
        cout << process.getProcessId() << "\t"
             << process.getMemoryRequired() << "\t"
             << process.getArrivalTime() << "\t"
             << process.getExecutionTime() << "\t\t"
             << process.getPriority() << "\t\t"
             << process.getStatus()
             << endl;
    }

    cout << "---------------------------------------------------------------" << endl;
}

void ProcessManager::clearProcesses()
{
    processes.clear();
}

int ProcessManager::getProcessCount() const
{
    return static_cast<int>(processes.size());
}
