#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <cmath>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#include <cstdio>
#endif

// Global mutex to prevent garbled console output
std::mutex print_mutex;

// Helper: Read config file
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

// Helper: Current timestamp as string (with milliseconds)
std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto value = now_ms.time_since_epoch();
    long long ms = value.count() % 1000;

    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&now_time_t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms;
    return oss.str();
}

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

// Worker thread function
void worker(int thread_id, int start, int end) {
    for (int num = start; num <= end; ++num) {
        if (isPrime(num)) {
            std::lock_guard<std::mutex> lock(print_mutex);
            std::cout << "[Thread-" << thread_id << "] "
                      << getCurrentTimestamp() << " Found prime: " << num << '\n';
        }
    }
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
        std::cout << "No numbers to check (max_number < 1).\n";
        return 0;
    }

    // Compute chunk size (ceiling division)
    int chunk_size = (max_number + num_threads - 1) / num_threads;

    std::vector<std::thread> threads;
    std::cout << "Starting " << num_threads << " threads to search 1 - " << max_number << "\n\n";

    for (int tid = 0; tid < num_threads; ++tid) {
        int start = tid * chunk_size + 1;
        int end = std::min((tid + 1) * chunk_size, max_number);

        if (start > max_number) break; // In case more threads than numbers

        threads.emplace_back(worker, tid, start, end);
    }

    // Wait for all threads to finish
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    std::cout << "\nAll threads completed.\n";

    // Wait for user to press any key before exiting
    waitForKeypress();

    return 0;
}