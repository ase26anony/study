struct cache_info {
    int sizekb;    // Cache size in KB
    int assoc;     // Associativity (ways)
    int line;      // Cache line size in bytes
};

struct cache_info *level1;  // L1 cache
struct cache_info *level2;  // L2 cache
