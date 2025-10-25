#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <future>
#include <cmath>
#include <algorithm>
#include <mutex>
#include <string>

#ifdef _WIN32
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#include <cstdio>
#endif

// Prime check function (same as before)
bool isPrime(int n) {
    if (n < 2) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    int limit = static_cast<int>(std::sqrt(n));
    for (int i = 3; i <= limit; i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

// Worker function: returns list of primes in its range
std::vector<int> worker(int start, int end) {
    std::vector<int> primes;
    for (int num = start; num <= end; ++num) {
        if (isPrime(num)) {
            primes.push_back(num);
        }
    }
    return primes;
}

// Helper: Read config
bool readConfig(const std::string& filename, int& num_threads, int& max_num) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open config file '" << filename << "'\n";
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#' || line.find('=') == std::string::npos) continue;

        size_t eq = line.find('=');
        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);

        key.erase(key.find_last_not_of(" \t") + 1);
        key.erase(0, key.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));

        if (key == "threads") {
            num_threads = std::stoi(value);
        } else if (key == "max_number") {
            max_num = std::stoi(value);
        }
    }
    file.close();
    return num_threads > 0 && max_num > 0;
}

// Helper: wait for a single keypress (cross-platform)
void waitForKeypress() {
#ifdef _WIN32
    std::cout << "Press any key to exit...";
    _getch();
#else
    std::cout << "Press any key to exit...";
    struct termios oldt, newt;
    if (tcgetattr(STDIN_FILENO, &oldt) == 0) {
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        (void)getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    } else {
        // Fallback
        (void)getchar();
    }
#endif
    std::cout << '\n';
}

int main() {
    int num_threads, max_number;
    if (!readConfig("config.txt", num_threads, max_number)) {
        std::cerr << "Failed to read config. Exiting.\n";
        return 1;
    }

    if (max_number < 1) {
        std::cout << "No numbers to check.\n";
        return 0;
    }

    int chunk_size = (max_number + num_threads - 1) / num_threads;

    // We'll store futures to get results from threads
    std::vector<std::future<std::vector<int>>> futures;

    std::cout << "Starting " << num_threads << " threads to search 1 - " << max_number << " (batch mode)\n";

    // Launch threads
    for (int tid = 0; tid < num_threads; ++tid) {
        int start = tid * chunk_size + 1;
        int end = std::min((tid + 1) * chunk_size, max_number);
        if (start > max_number) break;

        // Launch async task that returns vector of primes
        futures.push_back(std::async(std::launch::async, worker, start, end));
    }

    // Wait for all and collect results
    std::vector<std::vector<int>> all_results;
    for (auto& fut : futures) {
        all_results.push_back(fut.get()); // blocks until ready
    }

    // Now print everything at once
    std::cout << "\n=== BATCH OUTPUT: All primes found ===\n";
    int total_primes = 0;
    for (size_t tid = 0; tid < all_results.size(); ++tid) {
        std::vector<int>& primes = all_results[tid];
        total_primes += primes.size();
        std::cout << "Thread-" << tid << " found: ";
        if (primes.empty()) {
            std::cout << "none";
        } else {
            for (size_t i = 0; i < primes.size(); ++i) {
                std::cout << primes[i];
                if (i < primes.size() - 1) std::cout << ", ";
            }
        }
        std::cout << '\n';
    }

    std::cout << "\nTotal primes found: " << total_primes << "\n";
    std::cout << "All threads completed.\n";

    // Wait for user to press any key before exiting
    waitForKeypress();

    return 0;
}