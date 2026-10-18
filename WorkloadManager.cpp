
#include "WorkloadManager.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <limits>
#include <set>

using namespace std;

WorkloadManager::WorkloadManager()
{
    workloadName = "";
    loadFromFile();
}

bool WorkloadManager::readInteger(
    const string& prompt,
    int& value)
{
    while (true)
    {
        cout << prompt;

        if (cin >> value)
        {
            return true;
        }

        if (cin.eof())
        {
            return false;
        }

        cout << "Invalid input. Enter a whole number.\n";

        cin.clear();
        cin.ignore(
            numeric_limits<streamsize>::max(),
            '\n'
        );
    }
}

void WorkloadManager::createBasicWorkload()
{
    processes.clear();

    processes.push_back(Process(1, 2500, 0, 5, 2));
    processes.push_back(Process(2, 2000, 1, 4, 1));
    processes.push_back(Process(3, 1500, 2, 3, 3));
    processes.push_back(Process(4, 3000, 3, 5, 2));
    processes.push_back(Process(5, 1800, 4, 4, 1));
    processes.push_back(Process(6, 2200, 5, 6, 3));
    processes.push_back(Process(7, 1200, 6, 3, 2));
    processes.push_back(Process(8, 2800, 7, 5, 1));
    processes.push_back(Process(9, 3500, 8, 4, 3));
    processes.push_back(Process(10, 1000, 9, 3, 2));

    workloadName = "Basic Allocation Workload";
}

void WorkloadManager::createFragmentationWorkload()
{
    processes.clear();

    processes.push_back(Process(1, 4000, 0, 5, 2));
    processes.push_back(Process(2, 1000, 1, 3, 1));
    processes.push_back(Process(3, 3000, 2, 4, 3));
    processes.push_back(Process(4, 1000, 3, 3, 2));
    processes.push_back(Process(5, 5000, 4, 6, 1));
    processes.push_back(Process(6, 2000, 5, 4, 3));
    processes.push_back(Process(7, 2500, 6, 5, 2));
    processes.push_back(Process(8, 2800, 7, 4, 1));
    processes.push_back(Process(9, 3500, 8, 5, 3));
    processes.push_back(Process(10, 1200, 9, 3, 2));
    processes.push_back(Process(11, 1800, 10, 4, 1));
    processes.push_back(Process(12, 2200, 11, 5, 3));

    workloadName = "Fragmentation Workload";
}

void WorkloadManager::createMixedWorkload()
{
    processes.clear();

    processes.push_back(Process(1, 3200, 0, 8, 2));
    processes.push_back(Process(2, 1800, 1, 4, 1));
    processes.push_back(Process(3, 4500, 2, 7, 3));
    processes.push_back(Process(4, 2200, 3, 5, 2));
    processes.push_back(Process(5, 3800, 4, 6, 1));
    processes.push_back(Process(6, 1200, 5, 3, 3));
    processes.push_back(Process(7, 4600, 6, 7, 2));
    processes.push_back(Process(8, 2000, 7, 4, 1));
    processes.push_back(Process(9, 3400, 8, 6, 3));
    processes.push_back(Process(10, 2800, 9, 5, 2));
    processes.push_back(Process(11, 1500, 10, 3, 1));
    processes.push_back(Process(12, 4100, 11, 6, 3));
    processes.push_back(Process(13, 2300, 12, 4, 2));
    processes.push_back(Process(14, 3600, 13, 5, 1));
    processes.push_back(Process(15, 1700, 14, 3, 3));

    workloadName = "Mixed Workload";
}

bool WorkloadManager::createCustomWorkload()
{
    int count;

    if (!readInteger(
            "How many processes do you want to enter? ",
            count))
    {
        return false;
    }

    if (count <= 0 || count > 1000)
    {
        cout << "Enter a process count between 1 and 1000.\n";
        return false;
    }

    vector<Process> customProcesses;
    set<int> usedIds;

    cout << "\nEnter details for each process.\n";
    cout << "Memory must be positive; arrival time must be "
         << "non-negative; execution time and priority must be positive.\n";

    for (int i = 0; i < count; ++i)
    {
        int id;
        int memory;
        int arrival;
        int execution;
        int priority;

        cout << "\nProcess " << i + 1 << '\n';

        if (!readInteger("Process ID: ", id) ||
            !readInteger("Memory required (MB): ", memory) ||
            !readInteger("Arrival time: ", arrival) ||
            !readInteger("Execution time: ", execution) ||
            !readInteger("Priority (1 = highest): ", priority))
        {
            cout << "\nInput ended. Custom workload was not changed.\n";
            return false;
        }

        if (id <= 0 ||
            memory <= 0 ||
            arrival < 0 ||
            execution <= 0 ||
            priority <= 0)
        {
            cout << "Invalid process details. Please try again.\n";
            --i;
            continue;
        }

        if (usedIds.count(id) != 0)
        {
            cout << "That process ID already exists. "
                 << "Enter a unique ID.\n";
            --i;
            continue;
        }

        usedIds.insert(id);

        customProcesses.push_back(
            Process(id, memory, arrival, execution, priority)
        );
    }

    processes = customProcesses;
    workloadName = "Custom Workload";

    if (!saveToFile())
    {
        cout << "\nWarning: workload is available for this session, "
             << "but could not be saved to disk.\n";
    }
    else
    {
        cout << "\nCustom workload saved successfully.\n";
    }

    return true;
}

bool WorkloadManager::selectWorkload()
{
    while (true)
    {
        cout << "\n========================================\n";
        cout << "           WORKLOAD MANAGER\n";
        cout << "========================================\n";
        cout << "1. Basic Allocation Workload\n";
        cout << "2. Fragmentation Workload\n";
        cout << "3. Mixed Workload\n";
        cout << "4. Create Custom Workload\n";
        cout << "5. Load Previously Saved Workload\n";
        cout << "6. Back\n";

        if (!workloadName.empty())
        {
            cout << "\nCurrent workload: " << workloadName
                 << " (" << processes.size() << " processes)\n";
        }

        int choice;

        if (!readInteger("Enter choice: ", choice))
        {
            return false;
        }

        switch (choice)
        {
            case 1:
                createBasicWorkload();
                break;

            case 2:
                createFragmentationWorkload();
                break;

            case 3:
                createMixedWorkload();
                break;

            case 4:
                if (!createCustomWorkload())
                {
                    cout << "Custom workload was not selected.\n";
                    continue;
                }
                break;

            case 5:
                if (!loadFromFile())
                {
                    cout << "No valid saved workload was found.\n";
                    continue;
                }
                break;

            case 6:
                return false;

            default:
                cout << "Invalid choice.\n";
                continue;
        }

        if (choice >= 1 && choice <= 3)
        {
            saveToFile();
        }

        if (hasWorkload())
        {
            cout << "\nSelected: " << workloadName << '\n';
            displayWorkload();
            return true;
        }
    }
}

const vector<Process>& WorkloadManager::getProcesses() const
{
    return processes;
}

vector<Process> WorkloadManager::getWorkloadCopy() const
{
    return processes;
}

const string& WorkloadManager::getWorkloadName() const
{
    return workloadName;
}

bool WorkloadManager::hasWorkload() const
{
    return !processes.empty();
}

bool WorkloadManager::saveToFile() const
{
    ofstream file(fileName.c_str());

    if (!file)
    {
        return false;
    }

    file << "MEMORYX_WORKLOAD_V1\n";
    file << workloadName << '\n';
    file << processes.size() << '\n';

    for (const Process& process : processes)
    {
        file << process.getProcessId() << ' '
             << process.getMemoryRequired() << ' '
             << process.getArrivalTime() << ' '
             << process.getExecutionTime() << ' '
             << process.getPriority() << '\n';
    }

    return file.good();
}

bool WorkloadManager::saveWorkload() const
{
    return saveToFile();
}

bool WorkloadManager::loadFromFile()
{
    ifstream file(fileName.c_str());

    if (!file)
    {
        return false;
    }

    string signature;
    string savedName;
    size_t count;

    if (!getline(file, signature) ||
        signature != "MEMORYX_WORKLOAD_V1" ||
        !getline(file, savedName) ||
        !(file >> count) ||
        count == 0 ||
        count > 1000)
    {
        return false;
    }

    vector<Process> loadedProcesses;
    set<int> usedIds;

    for (size_t i = 0; i < count; ++i)
    {
        int id;
        int memory;
        int arrival;
        int execution;
        int priority;

        if (!(file >> id >> memory >> arrival >> execution >> priority))
        {
            return false;
        }

        if (id <= 0 ||
            memory <= 0 ||
            arrival < 0 ||
            execution <= 0 ||
            priority <= 0 ||
            usedIds.count(id) != 0)
        {
            return false;
        }

        usedIds.insert(id);

        loadedProcesses.push_back(
            Process(id, memory, arrival, execution, priority)
        );
    }

    processes = loadedProcesses;
    workloadName = savedName;

    return true;
}

bool WorkloadManager::loadSavedWorkload()
{
    return loadFromFile();
}

void WorkloadManager::displayWorkload() const
{
    if (processes.empty())
    {
        cout << "\nNo workload selected.\n";
        return;
    }

    cout << "\n============================================================\n";
    cout << "                  SELECTED WORKLOAD\n";
    cout << "============================================================\n";

    cout << "Workload: " << workloadName << '\n';
    cout << "Processes: " << processes.size() << "\n\n";

    cout << left
         << setw(8) << "PID"
         << setw(16) << "Memory(MB)"
         << setw(12) << "Arrival"
         << setw(12) << "Execution"
         << setw(10) << "Priority"
         << '\n';

    cout << "------------------------------------------------------------\n";

    for (const Process& process : processes)
    {
        cout << left
             << setw(8) << process.getProcessId()
             << setw(16) << process.getMemoryRequired()
             << setw(12) << process.getArrivalTime()
             << setw(12) << process.getExecutionTime()
             << setw(10) << process.getPriority()
             << '\n';
    }

    cout << "============================================================\n";
}

