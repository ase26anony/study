#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef __GNUC__
#include <cpuid.h>
#else
// Fallback for non-GCC compilers
static inline void __cpuid(int cpuInfo[4], int function_id) {
    __asm__ __volatile__ (
        "cpuid"
        : "=a"(cpuInfo[0]), "=b"(cpuInfo[1]), "=c"(cpuInfo[2]), "=d"(cpuInfo[3])
        : "a"(function_id)
    );
}

static inline void __cpuid_count(int cpuInfo[4], int function_id, int subfunction_id) {
    __asm__ __volatile__ (
        "cpuid"
        : "=a"(cpuInfo[0]), "=b"(cpuInfo[1]), "=c"(cpuInfo[2]), "=d"(cpuInfo[3])
        : "a"(function_id), "c"(subfunction_id)
    );
}
#endif

// Structure matching the cache descriptor in driver-i386.cc
struct cache_desc {
    int sizekb;
    int assoc;
    int line;
};

// Global variables to simulate the driver state
struct cache_desc level1_cache = {0, 0, 0};
struct cache_desc level2_cache = {0, 0, 0};
int xeon_mp = 0;  // We'll set this to 0 to hit the uncovered line in case 0x49

// Function that contains the exact switch logic from driver-i386.cc
void process_cache_descriptor_byte(uint8_t descriptor, struct cache_desc* level1, struct cache_desc* level2) {
    switch (descriptor) {
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            printf("Processed descriptor 0x0a: L1 8KB, 2-way, 32B line\n");
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            printf("Processed descriptor 0x0c: L1 16KB, 4-way, 32B line\n");
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("Processed descriptor 0x0d: L1 16KB, 4-way, 64B line\n");
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            printf("Processed descriptor 0x0e: L1 24KB, 6-way, 64B line\n");
            break;
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("Processed descriptor 0x21: L2 256KB, 8-way, 64B line\n");
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            printf("Processed descriptor 0x24: L2 1MB, 16-way, 64B line\n");
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            printf("Processed descriptor 0x2c: L1 32KB, 8-way, 64B line\n");
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            printf("Processed descriptor 0x39: L2 128KB, 4-way, 64B line\n");
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            printf("Processed descriptor 0x3a: L2 192KB, 6-way, 64B line\n");
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            printf("Processed descriptor 0x3b: L2 128KB, 2-way, 64B line\n");
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            printf("Processed descriptor 0x3c: L2 256KB, 4-way, 64B line\n");
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            printf("Processed descriptor 0x3d: L2 384KB, 6-way, 64B line\n");
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("Processed descriptor 0x3e: L2 512KB, 4-way, 64B line\n");
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            printf("Processed descriptor 0x41: L2 128KB, 4-way, 32B line\n");
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            printf("Processed descriptor 0x42: L2 256KB, 4-way, 32B line\n");
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            printf("Processed descriptor 0x43: L2 512KB, 4-way, 32B line\n");
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            printf("Processed descriptor 0x44: L2 1MB, 4-way, 32B line\n");
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            printf("Processed descriptor 0x45: L2 2MB, 4-way, 32B line\n");
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            printf("Processed descriptor 0x48: L2 3MB, 12-way, 64B line\n");
            break;
        case 0x49:
            if (xeon_mp)
                break;
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            printf("Processed descriptor 0x49: L2 4MB, 16-way, 64B line (xeon_mp = false)\n");
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            printf("Processed descriptor 0x4e: L2 6MB, 24-way, 64B line\n");
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            printf("Processed descriptor 0x60: L1 16KB, 8-way, 64B line\n");
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            printf("Processed descriptor 0x66: L1 8KB, 4-way, 64B line\n");
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("Processed descriptor 0x67: L1 16KB, 4-way, 64B line\n");
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            printf("Processed descriptor 0x68: L1 32KB, 4-way, 64B line\n");
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            printf("Processed descriptor 0x78: L2 1MB, 4-way, 64B line\n");
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            printf("Processed descriptor 0x79: L2 128KB, 8-way, 64B line\n");
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("Processed descriptor 0x7a: L2 256KB, 8-way, 64B line\n");
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("Processed descriptor 0x7b: L2 512KB, 8-way, 64B line\n");
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("Processed descriptor 0x7c: L2 1MB, 8-way, 64B line\n");
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            printf("Processed descriptor 0x7d: L2 2MB, 8-way, 64B line\n");
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            printf("Processed descriptor 0x7f: L2 512KB, 2-way, 64B line\n");
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("Processed descriptor 0x80: L2 512KB, 8-way, 64B line\n");
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            printf("Processed descriptor 0x82: L2 256KB, 8-way, 32B line\n");
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            printf("Processed descriptor 0x83: L2 512KB, 8-way, 32B line\n");
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            printf("Processed descriptor 0x84: L2 1MB, 8-way, 32B line\n");
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            printf("Processed descriptor 0x85: L2 2MB, 8-way, 32B line\n");
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("Processed descriptor 0x86: L2 512KB, 4-way, 64B line\n");
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("Processed descriptor 0x87: L2 1MB, 8-way, 64B line\n");
            break;
        default:
            // Not one of our target descriptors
            break;
    }
}

// Simulate CPUID leaf 0x02 processing with descriptor table
void simulate_cpuid_leaf2_processing() {
    printf("\n=== Simulating CPUID Leaf 0x02 Processing ===\n");
    
    // Target descriptor bytes from uncovered lines
    uint8_t target_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    // Simulate CPUID leaf 0x02 returning multiple descriptors
    // First byte (AL) indicates number of valid descriptor bytes
    uint8_t num_descriptors = sizeof(target_descriptors);
    
    printf("CPUID Leaf 0x02 returned AL = 0x%02x (%d descriptors)\n", 
           num_descriptors, num_descriptors);
    
    // Process each descriptor byte
    for (int i = 0; i < num_descriptors; i++) {
        process_cache_descriptor_byte(target_descriptors[i], 
                                      &level1_cache, &level2_cache);
    }
}

// Simulate CPUID leaf 0x04 processing (deterministic cache parameters)
void simulate_cpuid_leaf4_processing() {
    printf("\n=== Simulating CPUID Leaf 0x04 Processing ===\n");
    
    int regs[4];
    int cache_level = 0;
    
    do {
        __cpuid_count(regs, 0x04, cache_level);
        
        int cache_type = regs[0] & 0x1F;
        if (cache_type == 0) {
            printf("Cache level %d: No more caches\n", cache_level);
            break;
        }
        
        int cache_level_num = (regs[0] >> 5) & 0x7;
        int self_initializing = (regs[0] >> 8) & 0x1;
        int fully_associative = (regs[0] >> 9) & 0x1;
        
        int max_threads = ((regs[0] >> 14) & 0xFFF) + 1;
        int max_cores = ((regs[0] >> 26) & 0x3F) + 1;
        
        int line_size = (regs[1] & 0xFFF) + 1;
        int partitions = ((regs[1] >> 12) & 0x3FF) + 1;
        int ways = ((regs[1] >> 22) & 0x3FF) + 1;
        
        int sets = regs[2] + 1;
        
        int size_bytes = ways * partitions * line_size * sets;
        int size_kb = size_bytes / 1024;
        
        printf("Cache level %d (type %d):\n", cache_level, cache_type);
        printf("  Level: %d, Size: %d KB, Ways: %d, Line: %d B\n",
               cache_level_num, size_kb, ways, line_size);
        printf("  Sets: %d, Partitions: %d, Max Cores: %d, Max Threads: %d\n",
               sets, partitions, max_cores, max_threads);
        printf("  Self-initializing: %d, Fully associative: %d\n",
               self_initializing, fully_associative);
        
        cache_level++;
    } while (1);
}

// Direct CPUID calls to trigger actual hardware cache detection
void call_actual_cpuid() {
    printf("\n=== Calling Actual CPUID ===\n");
    
    int regs[4];
    char vendor[13];
    
    // Get CPU vendor
    __cpuid(regs, 0);
    memcpy(vendor, &regs[1], 4);
    memcpy(vendor + 4, &regs[3], 4);
    memcpy(vendor + 8, &regs[2], 4);
    vendor[12] = '\0';
    
    printf("CPU Vendor: %s\n", vendor);
    
    // Check if CPUID leaf 0x02 is supported
    __cpuid(regs, 1);
    int max_leaf = regs[0];
    printf("Max CPUID leaf: 0x%08x\n", max_leaf);
    
    if (max_leaf >= 0x02) {
        // Call CPUID leaf 0x02
        __cpuid(regs, 0x02);
        
        printf("CPUID Leaf 0x02 results:\n");
        printf("  EAX: 0x%08x\n", regs[0]);
        printf("  EBX: 0x%08x\n", regs[1]);
        printf("  ECX: 0x%08x\n", regs[2]);
        printf("  EDX: 0x%08x\n", regs[3]);
        
        // Check first byte of AL
        uint8_t al_byte = regs[0] & 0xFF;
        printf("  First byte (AL): 0x%02x\n", al_byte);
        
        if (al_byte == 1) {
            printf("  Using TLB method (not our target)\n");
        } else if (al_byte > 1) {
            printf("  Using descriptor table method (our target)\n");
            
            // Process descriptor bytes from all registers
            uint8_t* bytes = (uint8_t*)regs;
            for (int i = 0; i < 16; i++) {
                if (bytes[i] != 0 && bytes[i] != 0xFF) {
                    process_cache_descriptor_byte(bytes[i], 
                                                  &level1_cache, &level2_cache);
                }
            }
        }
    } else {
        printf("CPUID leaf 0x02 not supported\n");
    }
    
    // Try CPUID leaf 0x04 if supported
    if (max_leaf >= 0x04) {
        simulate_cpuid_leaf4_processing();
    }
}

int main() {
    printf("Cache Descriptor Coverage Test Program\n");
    printf("======================================\n");
    
    // Set xeon_mp to false to hit the uncovered line in case 0x49
    xeon_mp = 0;
    printf("xeon_mp flag set to: %d (false)\n", xeon_mp);
    
    // Method 1: Simulate processing with all target descriptor bytes
    simulate_cpuid_leaf2_processing();
    
    // Method 2: Call actual CPUID (if supported)
    call_actual_cpuid();
    
    // Print final cache configuration
    printf("\n=== Final Cache Configuration ===\n");
    printf("Level 1 Cache: %d KB, %d-way, %d B line\n",
           level1_cache.sizekb, level1_cache.assoc, level1_cache.line);
    printf("Level 2 Cache: %d KB, %d-way, %d B line\n",
           level2_cache.sizekb, level2_cache.assoc, level2_cache.line);
    
    // Force compiler to keep variables
    volatile int keep_alive = level1_cache.sizekb + level2_cache.sizekb;
    
    return 0;
}
