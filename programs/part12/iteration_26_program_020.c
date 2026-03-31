struct cache_info {
    int sizekb;
    int assoc;
    int line;
};

struct cache_info level1, level2;
// ... CPUID call returns descriptor byte in eax/ebx/ecx/edx
// switch(descriptor_byte) { ... }
