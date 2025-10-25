#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <future>
#include <cmath>
#include <algorithm>
#include <string>

#ifdef _WIN32
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#include <cstdio>
#endif

// Prime check function
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

// Worker: interleaved assignment, returns list of primes
std::vector<int> worker(int thread_id, int num_threads, int max_number) {
    std::vector<int> primes;
    for (int n = thread_id + 1; n <= max_number; n += num_threads) {
        if (isPrime(n)) {
            primes.push_back(n);
        }
    }
    return primes;
}

// Read config file
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
        // Trim whitespace
        auto trim = [](std::string& s) {
            size_t start = s.find_first_not_of(" \t");
            if (start == std::string::npos) { s.clear(); return; }
            size_t end = s.find_last_not_of(" \t");
            s = s.substr(start, end - start + 1);
        };
        trim(key);
        trim(value);
        trim(value);

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
        // fallback
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

    std::cout << "Starting " << num_threads << " threads (interleaved) to search 1 - "
              << max_number << " (batch mode)\n";

    // Launch async tasks
    std::vector<std::future<std::vector<int>>> futures;
    for (int tid = 0; tid < num_threads; ++tid) {
        futures.push_back(
            std::async(std::launch::async, worker, tid, num_threads, max_number)
        );
    }

    // Collect results
    std::vector<std::vector<int>> all_results;
    for (auto& fut : futures) {
        all_results.push_back(fut.get());
    }

    // Print all at once
    std::cout << "\n=== BATCH OUTPUT: Interleaved Assignment ===\n";
    int total = 0;
    for (int tid = 0; tid < num_threads; ++tid) {
        const auto& primes = all_results[tid];
        total += primes.size();
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

    std::cout << "\nTotal primes found: " << total << "\n";
    std::cout << "All threads completed.\n";

    // Wait for user to press any key before exiting
    waitForKeypress();

    return 0;
}