#include <iostream>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <cfloat>

using namespace std;

// ============================================================
//                    DATA STRUCTURES
// ============================================================

struct TrafficInterval
{
    int interval;
    int north;
    int south;
    int east;
    int west;
};

struct SimulationResult
{
    int greenTime;
    int redTime;
    double avgWaitTime;

    int totalVehicles;
    int totalServed;
    int leftoverVehicles;
    int peakQueue;
};

// ============================================================
//                      INPUT FUNCTION
// ============================================================

vector<TrafficInterval> takeInput(int &n)
{
    cout << "==============================================\n";
    cout << " SMART TRAFFIC INTERSECTION MANAGEMENT SYSTEM\n";
    cout << "==============================================\n";

    cout << "\nEnter number of time intervals: ";
    cin >> n;

    while (n <= 0)
    {
        cout << "Enter valid value: ";
        cin >> n;
    }

    vector<TrafficInterval> data(n);

    cout << "\nEnter vehicle arrivals for each road:\n";

    for (int i = 0; i < n; i++)
    {
        data[i].interval = i + 1;

        cout << "\nInterval " << i + 1 << ":\n";

        cout << "North Road Vehicles : ";
        cin >> data[i].north;

        cout << "South Road Vehicles : ";
        cin >> data[i].south;

        cout << "East Road Vehicles  : ";
        cin >> data[i].east;

        cout << "West Road Vehicles  : ";
        cin >> data[i].west;

        // Basic validation
        data[i].north = max(0, data[i].north);
        data[i].south = max(0, data[i].south);
        data[i].east  = max(0, data[i].east);
        data[i].west  = max(0, data[i].west);
    }

    return data;
}

// ============================================================
//                 DYNAMIC SERVICE RATE
// ============================================================
// Returns vehicles/sec based on current congestion.
// Higher queue -> slightly better utilization.
int getServiceRate(int queue)
{
    if (queue < 20)
        return 2;
    else if (queue < 50)
        return 3;
    else
        return 4;
}

// ============================================================
//                    TRAFFIC SIMULATION
// ============================================================

SimulationResult simulate(vector<TrafficInterval> &data,
                          int greenTime,
                          bool showTable = false)
{
    SimulationResult result;

    // Total cycle time = 80 seconds
    result.greenTime = greenTime;
    result.redTime = 80 - greenTime;

    result.totalVehicles = 0;
    result.totalServed = 0;
    result.leftoverVehicles = 0;
    result.avgWaitTime = 0.0;
    result.peakQueue = 0;

    double totalWait = 0.0;

    // Separate queues for each road
    int northQueue = 0;
    int southQueue = 0;
    int eastQueue = 0;
    int westQueue = 0;

    if (showTable)
    {
        cout << "\n===============================================================\n";
        cout << "                 INTERSECTION TRAFFIC SIMULATION\n";
        cout << "===============================================================\n";

        cout << left
             << setw(10) << "Int"
             << setw(12) << "Road"
             << setw(12) << "Incoming"
             << setw(12) << "Served"
             << setw(12) << "Queue"
             << setw(12) << "Traffic"
             << "\n";

        cout << "---------------------------------------------------------------\n";
    }

    for (auto &d : data)
    {
        vector<pair<string, int>> roads = {
            {"North", d.north},
            {"South", d.south},
            {"East",  d.east},
            {"West",  d.west}
        };

        vector<int*> queues = {
            &northQueue,
            &southQueue,
            &eastQueue,
            &westQueue
        };

        for (int i = 0; i < 4; i++)
        {
            // Add incoming vehicles to queue
            *queues[i] += roads[i].second;
            result.totalVehicles += roads[i].second;

            int currentQueue = *queues[i];

            // Dynamic service rate
            int serviceRate = getServiceRate(currentQueue);

            // Realistic capacity adjustment
            // Reduced by factor of 4 to model start-up delays,
            // reaction time, lane inefficiencies, etc.
            int capacity = max(1, (greenTime * serviceRate) / 4);

            // Vehicles that pass during green
            int served = min(currentQueue, capacity);

            // Update queue
            *queues[i] -= served;

            // Update statistics
            result.totalServed += served;
            result.peakQueue = max(result.peakQueue, *queues[i]);

            // Approximate waiting time contribution
            totalWait += (*queues[i] * result.redTime) / 2.0;

            // Congestion level
            string level;
            if (*queues[i] < 10)
                level = "LOW";
            else if (*queues[i] < 30)
                level = "MEDIUM";
            else
                level = "HIGH";

            // Display row if requested
            if (showTable)
            {
                cout << left
                     << setw(10) << d.interval
                     << setw(12) << roads[i].first
                     << setw(12) << roads[i].second
                     << setw(12) << served
                     << setw(12) << *queues[i]
                     << setw(12) << level
                     << "\n";
            }
        }
    }

    // Total leftover across all roads
    result.leftoverVehicles =
        northQueue + southQueue + eastQueue + westQueue;

    // Average waiting time per vehicle
    if (result.totalVehicles > 0)
        result.avgWaitTime = totalWait / result.totalVehicles;
    else
        result.avgWaitTime = 0.0;

    return result;
}

// ============================================================
//                 BINARY SEARCH OPTIMIZATION
// ============================================================

SimulationResult optimizeSignal(vector<TrafficInterval> &data)
{
    // Green time search range (cycle = 80 sec)
    int lo = 15;
    int hi = 60;

    int optimalGreen = 15;
    double bestWait = DBL_MAX;

    int step = 1;

    cout << "\n\n==============================================\n";
    cout << "          SIGNAL OPTIMIZATION PROCESS\n";
    cout << "==============================================\n";

    cout << left
         << setw(8) << "Step"
         << setw(12) << "Green"
         << setw(15) << "Avg Wait"
         << "\n";

    cout << "--------------------------------------\n";

    while (lo <= hi)
    {
        int mid = (lo + hi) / 2;

        SimulationResult current = simulate(data, mid);
        SimulationResult next = simulate(data, mid + 1);

        // Track best result
        if (current.avgWaitTime < bestWait)
        {
            bestWait = current.avgWaitTime;
            optimalGreen = mid;
        }

        // Display current test
        cout << left
             << setw(8) << step
             << setw(12) << mid
             << setw(15) << fixed << setprecision(2)
             << current.avgWaitTime
             << "\n";

        // Binary search direction
        if (next.avgWaitTime < current.avgWaitTime)
            lo = mid + 1;
        else
            hi = mid - 1;

        step++;
    }

    cout << "--------------------------------------\n";

    // Run final simulation with detailed table
    return simulate(data, optimalGreen, true);
}

// ============================================================
//                 SIGNAL VISUALIZATION
// ============================================================

void displaySignal(int green)
{
    int red = 80 - green;

    cout << "\nSignal Cycle Visualization:\n\n";

    cout << "[ ";

    for (int i = 0; i < green / 2; i++)
        cout << "G";

    for (int i = 0; i < red / 2; i++)
        cout << "R";

    cout << " ]\n";

    cout << "Green Time : " << green << " sec\n";
    cout << "Red Time   : " << red << " sec\n";
}

// ============================================================
//                    FINAL RESULTS
// ============================================================

void displayResults(SimulationResult &res)
{
    cout << "\n\n==============================================\n";
    cout << "                FINAL RESULTS\n";
    cout << "==============================================\n";

    cout << fixed << setprecision(2);

    cout << "Optimal Green Time  : " << res.greenTime << " sec\n";
    cout << "Red Signal Time     : " << res.redTime << " sec\n";
    cout << "Average Wait Time   : " << res.avgWaitTime << " sec\n";

    cout << "Total Vehicles      : " << res.totalVehicles << "\n";
    cout << "Vehicles Served     : " << res.totalServed << "\n";
    cout << "Leftover Vehicles   : " << res.leftoverVehicles << "\n";
    cout << "Peak Queue Size     : " << res.peakQueue << "\n";

    string status;
    if (res.avgWaitTime < 10)
        status = "Smooth Traffic";
    else if (res.avgWaitTime < 30)
        status = "Moderate Traffic";
    else
        status = "Heavy Congestion";

    cout << "Traffic Status      : " << status << "\n";

    displaySignal(res.greenTime);

    cout << "==============================================\n";
}

// ============================================================
//                          MAIN
// ============================================================

int main()
{
    int n;

    vector<TrafficInterval> data = takeInput(n);

    SimulationResult optimal = optimizeSignal(data);

    displayResults(optimal);

    return 0;
}