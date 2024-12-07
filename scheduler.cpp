#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <iomanip>
#include <algorithm>

using namespace std;

// Define the Job structure
struct Job {
    int id;
    double arrivalTime;
    double burstTime;
    double remainingTime;
    double completionTime;
    double turnaroundTime;
    double waitingTime;
};

// Function to parse input file
vector<Job> parseInputFile(const string& filename) {
    vector<Job> jobs;
    ifstream file(filename);
    double arrival, burst;
    int id = 0;

    while (file >> arrival >> burst) {
        jobs.push_back({id++, arrival, burst, burst, 0, 0, 0});
    }

    return jobs;
}

// FCFS scheduling algorithm
void executeFCFS(vector<Job>& jobs) {
    double current_time = 0;

    for (auto& job : jobs) {
        if (current_time < job.arrivalTime) {
            current_time = job.arrivalTime;
        }

        job.completionTime = current_time + job.burstTime;
        job.turnaroundTime = job.completionTime - job.arrivalTime;
        job.waitingTime = job.turnaroundTime - job.burstTime;

        current_time = job.completionTime;
    }
}

// SJF scheduling algorithm
void performSJF(vector<Job>& jobs) {
    auto compare = [](Job* a, Job* b) {
        return a->burstTime > b->burstTime;
    };

    priority_queue<Job*, vector<Job*>, decltype(compare)> jobQueue(compare);
    vector<bool> isCompleted(jobs.size(), false);
    double current_time = 0;

    while (true) {
        for (auto& job : jobs) {
            if (!isCompleted[job.id] && job.arrivalTime <= current_time) {
                jobQueue.push(&job);
            }
        }

        if (jobQueue.empty()) {
            current_time++;
            continue;
        }

        auto job = jobQueue.top();
        jobQueue.pop();

        isCompleted[job->id] = true;
        job->completionTime = current_time + job->burstTime;
        job->turnaroundTime = job->completionTime - job->arrivalTime;
        job->waitingTime = job->turnaroundTime - job->burstTime;

        current_time = job->completionTime;

        if (all_of(isCompleted.begin(), isCompleted.end(), [](bool val) { return val; })) {
            break;
        }
    }
}

// RR scheduling algorithm with new metrics
void handleRoundRobin(vector<Job>& jobs, int timeSlice, int overheadTime, double& totalSimulationTime, double& avgQueueLength, int& maxQueueLength) {
    queue<Job*> readyJobs;
    double current_time = 0;
    vector<bool> completedJobs(jobs.size(), false);
    vector<bool> inQueue(jobs.size(), false);
    double queueLengthSum = 0;
    int queueLengthCount = 0;

    while (true) {
        for (auto& job : jobs) {
            if (!completedJobs[job.id] && !inQueue[job.id] && job.arrivalTime <= current_time) {
                readyJobs.push(&job);
                inQueue[job.id] = true;
            }
        }

        maxQueueLength = max(maxQueueLength, static_cast<int>(readyJobs.size()));
        queueLengthSum += readyJobs.size();
        queueLengthCount++;

        if (readyJobs.empty()) {
            current_time++;
            continue;
        }

        auto job = readyJobs.front();
        readyJobs.pop();

        double timeToExecute = min(timeSlice / 1000.0, job->remainingTime);
        job->remainingTime -= timeToExecute;
        current_time += timeToExecute;

        if (job->remainingTime <= 0) {
            completedJobs[job->id] = true;
            job->completionTime = current_time;
            job->turnaroundTime = job->completionTime - job->arrivalTime;
            job->waitingTime = job->turnaroundTime - job->burstTime;
        } else {
            current_time += overheadTime / 1000.0;
            readyJobs.push(job);
        }

        if (all_of(completedJobs.begin(), completedJobs.end(), [](bool val) { return val; })) {
            break;
        }
    }

    totalSimulationTime = current_time;
    avgQueueLength = queueLengthSum / queueLengthCount;
}

// Function to evaluate performance metrics
void computePerformanceMetrics(const vector<Job>& jobs) {
    double totalWaitingTime = 0, totalTurnaroundTime = 0;

    for (const auto& job : jobs) {
        totalWaitingTime += job.waitingTime;
        totalTurnaroundTime += job.turnaroundTime;
    }

    cout << fixed << setprecision(2);
    cout << "Average Waiting Time: " << totalWaitingTime / jobs.size() << " seconds\n";
    cout << "Average Turnaround Time: " << totalTurnaroundTime / jobs.size() << " seconds\n";
}

// Experimentation with RR parameters
void conductRRAnalysis(vector<Job> jobs) {
    vector<int> overheadOptions = {0, 5, 10, 15, 20, 25};
    vector<int> quantumValues = {50, 100, 250, 500};

    for (int overhead : overheadOptions) {
        for (int quantum : quantumValues) {
            double totalSimulationTime = 0;
            double avgQueueLength = 0;
            int maxQueueLength = 0;

            cout << "Round Robin with Quantum: " << quantum << "ms and Overhead: " << overhead << "ms\n";
            auto tempJobs = jobs;
            handleRoundRobin(tempJobs, quantum, overhead, totalSimulationTime, avgQueueLength, maxQueueLength);

            computePerformanceMetrics(tempJobs);
            cout << "Total Simulation Time: " << totalSimulationTime << " seconds\n";
            cout << "Average Ready Queue Length: " << avgQueueLength << "\n";
            cout << "Maximum Ready Queue Length: " << maxQueueLength << "\n";
            cout << "---------------------------\n";
        }
    }
}

// Main function
int main() {
    string filename = "process_data.txt";
    vector<Job> jobs = parseInputFile(filename);

    cout << "First-Come, First-Served Scheduling:\n";
    auto fcfsJobs = jobs;
    executeFCFS(fcfsJobs);
    computePerformanceMetrics(fcfsJobs);

    cout << "\nShortest-Job-First Scheduling:\n";
    auto sjfJobs = jobs;
    performSJF(sjfJobs);
    computePerformanceMetrics(sjfJobs);

    cout << "\nRound-Robin Scheduling Analysis:\n";
    conductRRAnalysis(jobs);

    return 0;
}
