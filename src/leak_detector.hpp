#pragma once
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <cstdlib>

struct AllocationInfo {
    size_t size;
    const char* file;
    int line;
};

class MemoryTracker {
public:
    static MemoryTracker& Instance() {
        static MemoryTracker instance;
        return instance;
    }

    void RecordAllocation(void* ptr, size_t size, const char* file, int line) {
        std::lock_guard<std::mutex> lock(mtx);
        allocations[ptr] = {size, file, line};
        totalAllocated += size;
        activeBytes += size;
        if (activeBytes > peakBytes) peakBytes = activeBytes;
        allocationCount++;
    }

    void RecordDeallocation(void* ptr) {
        if (!ptr) return;
        std::lock_guard<std::mutex> lock(mtx);
        auto it = allocations.find(ptr);
        if (it != allocations.end()) {
            activeBytes -= it->second.size;
            allocations.erase(it);
        }
    }

    void DumpReport() {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "\n========================================================\n";
        std::cout << "             MEMORY LEAK AUDIT REPORT                  \n";
        std::cout << "========================================================\n";
        std::cout << "Total Allocations : " << allocationCount << "\n";
        std::cout << "Peak Memory Used  : " << peakBytes << " bytes\n";
        std::cout << "Active Leaks Count: " << allocations.size() << "\n";
        std::cout << "Total Leaked Bytes: " << activeBytes << " bytes\n\n";

        if (allocations.empty()) {
            std::cout << "[+] EXCELLENT: No memory leaks detected!\n";
        } else {
            std::cout << "[-] DETECTED LEAKS:\n";
            std::cout << "--------------------------------------------------------\n";
            for (const auto& pair : allocations) {
                std::cout << "  Address: " << pair.first
                          << " | Size: " << pair.second.size << " bytes"
                          << " | Location: " << pair.second.file << ":" << pair.second.line << "\n";
            }
            std::cout << "--------------------------------------------------------\n";
        }
        std::cout << "========================================================\n";
    }

private:
    std::map<void*, AllocationInfo> allocations;
    std::mutex mtx;
    size_t totalAllocated = 0;
    size_t activeBytes = 0;
    size_t peakBytes = 0;
    size_t allocationCount = 0;
};

void* operator new(size_t size, const char* file, int line) {
    void* ptr = std::malloc(size);
    if (!ptr) throw std::bad_alloc();
    MemoryTracker::Instance().RecordAllocation(ptr, size, file, line);
    return ptr;
}

void* operator new[](size_t size, const char* file, int line) {
    void* ptr = std::malloc(size);
    if (!ptr) throw std::bad_alloc();
    MemoryTracker::Instance().RecordAllocation(ptr, size, file, line);
    return ptr;
}

void operator delete(void* ptr) noexcept {
    MemoryTracker::Instance().RecordDeallocation(ptr);
    std::free(ptr);
}

void operator delete[](void* ptr) noexcept {
    MemoryTracker::Instance().RecordDeallocation(ptr);
    std::free(ptr);
}

#define DBG_NEW new(__FILE__, __LINE__)
