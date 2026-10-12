#include "leak_detector.hpp"

#define new DBG_NEW

class UserSession {
public:
    UserSession(int id) : sessionId(id) {
        dataBuffer = new char[256];
    }
    ~UserSession() {
        delete[] dataBuffer;
    }
private:
    int sessionId;
    char* dataBuffer;
};

int main() {
    std::cout << "[*] Starting Memory Leak Detector demonstration...\n";

    int* safeValue = new int(42);
    delete safeValue;

    UserSession* safeSession = new UserSession(101);
    delete safeSession;

    std::cout << "[*] Simulating intentional memory leaks...\n";
    char* leakedString = new char[64];
    int* leakedArray = new int[10];
    UserSession* leakedSession = new UserSession(999);

    MemoryTracker::Instance().DumpReport();
    return 0;
}
