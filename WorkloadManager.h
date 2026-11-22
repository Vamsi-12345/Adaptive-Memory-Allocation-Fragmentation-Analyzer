
#ifndef WORKLOADMANAGER_H
#define WORKLOADMANAGER_H

#include <vector>
#include <string>
#include "Process.h"

class WorkloadManager
{
private:
    std::vector<Process> processes;
    std::vector<std::string> eventHistory;
    std::string workloadName;
    const std::string fileName = "memoryx_workload.txt";

    bool readInteger(const std::string& prompt, int& value);
    bool saveToFile() const;
    bool loadFromFile();
    bool loadEventHistory();
    void createBasicWorkload();
    void createFragmentationWorkload();
    void createMixedWorkload();
    bool createCustomWorkload();

public:
    WorkloadManager();

    bool loadBasicWorkload();
    bool loadFragmentationWorkload();
    bool loadMixedWorkload();

    bool selectWorkload();

    const std::vector<Process>& getProcesses() const;
    std::vector<Process> getWorkloadCopy() const;
    const std::vector<std::string>& getEventHistory() const;

    const std::string& getWorkloadName() const;
    bool hasWorkload() const;

    bool saveWorkload() const;
    bool loadSavedWorkload();
    void displayWorkload() const;
};

#endif
