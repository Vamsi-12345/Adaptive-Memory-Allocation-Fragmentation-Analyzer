
#ifndef WORKLOADMANAGER_H
#define WORKLOADMANAGER_H

#include <vector>
#include <string>
#include "Process.h"

class WorkloadManager
{
private:
    std::vector<Process> processes;
    std::string workloadName;
    const std::string fileName = "memoryx_workload.txt";

    bool readInteger(const std::string& prompt, int& value);
    bool saveToFile() const;
    bool loadFromFile();

    void createBasicWorkload();
    void createFragmentationWorkload();
    void createMixedWorkload();
    bool createCustomWorkload();

public:
    WorkloadManager();

    bool selectWorkload();

    const std::vector<Process>& getProcesses() const;
    std::vector<Process> getWorkloadCopy() const;

    const std::string& getWorkloadName() const;
    bool hasWorkload() const;

    bool saveWorkload() const;
    bool loadSavedWorkload();
    void displayWorkload() const;
};

#endif
