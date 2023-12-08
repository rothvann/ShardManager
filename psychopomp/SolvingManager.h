#include <chrono>
#include "svc_discovery/BinManager.h"

// Create state
// Remove finished mutations
// Solve
// Add mutations

namespace psychopomp {

class SolvingManager {
public:
    SolvingManager(std::shared_ptr<BinManager> binManager, size_t periodMs, size_t numThreads);

private:
    std::shared_ptr<BinManager> binManager_;
    size_t periodMs_;
    size_t numThreads_;
};
}