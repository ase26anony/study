struct cache_config {
    int sizekb;
    int assoc;
    int line;
};

struct cache_config l1_cache, l2_cache;
// ... CPUID execution ...
// Parse CPUID results into these case statements
