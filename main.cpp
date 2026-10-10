
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "WorkloadManager.h"
#include "ProcessSimulation.h"
#include "StrategyComparison.h"

using namespace std;

int readPositiveInteger(const string& prompt)
{
    int value;

    while (true)
    {
        cout << prompt;

        if (cin >> value && value > 0)
        {
            return value;
        }

        cout << "Invalid input. Enter a positive integer.\n";

        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

int readChoice(int minimum, int maximum)
{
    int choice;

    while (true)
    {
        cout << "Enter your choice: ";

        if (cin >> choice && choice >= minimum && choice <= maximum)
        {
            return choice;
        }

        cout << "Invalid choice. Select "
             << minimum << " to " << maximum << ".\n";

        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

bool ensureWorkload(WorkloadManager& manager)
{
    if (!manager.hasWorkload())
    {
        cout << "\nNo workload is currently loaded.\n";

        if (!manager.selectWorkload())
        {
            cout << "A workload must be selected before simulation.\n";
            return false;
        }

        return manager.hasWorkload();
    }

    cout << "\nCurrent workload: "
         << manager.getWorkloadName() << '\n';

    cout << "1. Use current workload\n";
    cout << "2. Select a different workload\n";
    cout << "3. Cancel\n";

    int choice = readChoice(1, 3);

    if (choice == 1)
    {
        return true;
    }

    if (choice == 2)
    {
        if (!manager.selectWorkload())
        {
            cout << "Workload selection cancelled. "
                    "Keeping the previously selected workload.\n";
        }

        return manager.hasWorkload();
    }

    return false;
}

int main()
{
    cout << "========================================\n";
    cout << "                MEMORYX\n";
    cout << "  Memory Allocation Simulation System\n";
    cout << "========================================\n\n";

    const int totalMemory =
        readPositiveInteger("Enter total simulated memory (MB): ");

    // One manager owns the selected workload for the whole application.
    WorkloadManager workloadManager;

    if (!workloadManager.hasWorkload())
    {
        cout << "\nNo saved workload was found.\n";

        if (!workloadManager.selectWorkload())
        {
            cout << "No workload selected. Exiting MemoryX.\n";
            return 0;
        }
    }

    bool running = true;

    while (running)
    {
        cout << "\n========================================\n";
        cout << "              MEMORYX MENU\n";
        cout << "========================================\n";

        cout << "Current Workload: "
             << workloadManager.getWorkloadName() << '\n';

        cout << "Total Memory: "
             << totalMemory << " MB\n\n";

        cout << "1. Process Simulation\n";
        cout << "2. Strategy Comparison\n";
        cout << "3. Exit\n";

        int choice = readChoice(1, 3);

        if (choice == 1)
        {
            if (!ensureWorkload(workloadManager))
            {
                continue;
            }

            ProcessSimulation simulation(
                workloadManager.getWorkloadCopy(),
                workloadManager.getWorkloadName()
            );

            simulation.showMenu(totalMemory);
        }
        else if (choice == 2)
        {
            if (!ensureWorkload(workloadManager))
            {
                continue;
            }

            StrategyComparison comparison;

            comparison.compareAll(
                totalMemory,
                workloadManager.getWorkloadCopy()
            );
        }
        else
        {
            cout << "\nExiting MemoryX. Thank you!\n";
            running = false;
        }
    }

    return 0;
}
