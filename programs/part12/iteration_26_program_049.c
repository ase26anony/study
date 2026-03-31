// Example structure
struct cache_info {
    int sizekb;
    int assoc;
    int line;
};

struct cache_info l1_cache, l2_cache;
// ... CPUID call gets descriptor byte ...
// ... switch statement routes to appropriate configuration ...
