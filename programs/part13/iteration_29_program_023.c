#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <cpuid.h>

// Simulate the cache descriptor structure from driver-i386.cc
struct cache_desc {
    int sizekb;
    int assoc;
    int line;
};

// Global flag to simulate xeon_mp condition
static int xeon_mp = 0;

// Function to process cache descriptors (mimicking the uncovered logic)
void process_cache_descriptor(uint8_t descriptor, struct cache_desc* level1, 
                              struct cache_desc* level2, int* got_level1, int* got_level2) {
    switch (descriptor) {
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            *got_level1 = 1;
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            *got_level1 = 1;
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            *got_level1 = 1;
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            *got_level1 = 1;
            break;
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            *got_level1 = 1;
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            *got_level2 = 1;
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            *got_level2 = 1;
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            *got_level2 = 1;
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            *got_level2 = 1;
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            *got_level2 = 1;
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x49:
            if (xeon_mp)
                break;
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            *got_level1 = 1;
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            *got_level1 = 1;
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            *got_level1 = 1;
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            *got_level1 = 1;
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            *got_level2 = 1;
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            *got_level2 = 1;
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            *got_level2 = 1;
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            *got_level2 = 1;
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            *got_level2 = 1;
            break;
        default:
            // Ignore other descriptors
            break;
    }
}

// Simulate CPUID leaf 0x02 with descriptor table
void simulate_cpuid_leaf2(uint8_t* descriptors, int count) {
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    int got_level1 = 0;
    int got_level2 = 0;
    
    printf("Processing %d cache descriptors:\n", count);
    
    for (int i = 0; i < count; i++) {
        uint8_t desc = descriptors[i];
        
        // Skip invalid descriptors (0x00) and FFh
        if (desc == 0x00 || desc == 0xFF) {
            continue;
        }
        
        printf("  Descriptor 0x%02x: ", desc);
        process_cache_descriptor(desc, &level1, &level2, &got_level1, &got_level2);
        
        // Check what we got
        if (desc >= 0x40 && desc <= 0x49) {
            printf("L2 Cache ");
        } else if (desc >= 0x0a && desc <= 0x0e) {
            printf("L1 Cache ");
        } else if (desc >= 0x21 && desc <= 0x24) {
            printf("L2 Cache ");
        } else {
            printf("Unknown type ");
        }
        printf("\n");
    }
    
    if (got_level1) {
        printf("L1 Cache: %dKB, %d-way associative, %d byte line size\n",
               level1.sizekb, level1.assoc, level1.line);
    }
    if (got_level2) {
        printf("L2 Cache: %dKB, %d-way associative, %d byte line size\n",
               level2.sizekb, level2.assoc, level2.line);
    }
}

// Real CPUID leaf 0x02 call
void call_real_cpuid_leaf2() {
    uint32_t eax, ebx, ecx, edx;
    
    printf("\n=== Real CPUID Leaf 0x02 ===\n");
    
    // Call CPUID leaf 0x02
    __cpuid_count(0x02, 0, eax, ebx, ecx, edx);
    
    // Check if AL > 1 (valid descriptor table)
    uint8_t al = eax & 0xFF;
    if (al == 1) {
        printf("CPU uses alternative cache detection method (AL=1)\n");
        return;
    } else if (al == 0) {
        printf("No cache descriptors returned (AL=0)\n");
        return;
    }
    
    printf("CPUID Leaf 0x02 returned: EAX=0x%08x, EBX=0x%08x, ECX=0x%08x, EDX=0x%08x\n",
           eax, ebx, ecx, edx);
    printf("Number of valid descriptor bytes: %d\n", al);
    
    // Extract descriptor bytes
    uint8_t descriptors[16];
    memcpy(descriptors, &eax, 4);
    memcpy(descriptors + 4, &ebx, 4);
    memcpy(descriptors + 8, &ecx, 4);
    memcpy(descriptors + 12, &edx, 4);
    
    // Process only valid descriptors (first 'al' bytes)
    simulate_cpuid_leaf2(descriptors, al);
}

// Real CPUID leaf 0x04 call
void call_real_cpuid_leaf4() {
    uint32_t eax, ebx, ecx, edx;
    int cache_index = 0;
    
    printf("\n=== Real CPUID Leaf 0x04 ===\n");
    
    do {
        __cpuid_count(0x04, cache_index, eax, ebx, ecx, edx);
        
        int cache_type = eax & 0x1F;
        if (cache_type == 0) {
            break; // No more caches
        }
        
        int cache_level = (eax >> 5) & 0x7;
        int self_initializing = (eax >> 8) & 0x1;
        int fully_associative = (eax >> 9) & 0x1;
        int max_threads = ((eax >> 14) & 0xFFF) + 1;
        int max_cores = ((eax >> 26) & 0x3F) + 1;
        
        int line_size = (ebx & 0xFFF) + 1;
        int partitions = ((ebx >> 12) & 0x3FF) + 1;
        int ways = ((ebx >> 22) & 0x3FF) + 1;
        
        int sets = ecx + 1;
        
        int size = ways * partitions * line_size * sets / 1024;
        
        printf("Cache %d: Type=%d, Level=%d, Size=%dKB, ", 
               cache_index, cache_type, cache_level, size);
        printf("Line=%dB, Ways=%d, Sets=%d\n", line_size, ways, sets);
        
        cache_index++;
    } while (1);
}

// Test all uncovered cases with simulated data
void test_all_uncovered_cases() {
    printf("\n=== Testing All Uncovered Cases ===\n");
    
    // All the target descriptor values from uncovered lines
    uint8_t all_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e,           // L1 caches
        0x21, 0x24, 0x2c,                 // L2 caches
        0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, // More L2
        0x41, 0x42, 0x43, 0x44, 0x45,     // L2 with 32-byte lines
        0x48, 0x49, 0x4e,                 // Large L2 caches
        0x60, 0x66, 0x67, 0x68,           // More L1
        0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7f, 0x80, // Various L2
        0x82, 0x83, 0x84, 0x85, 0x86, 0x87  // L2 with different configs
    };
    
    int count = sizeof(all_descriptors) / sizeof(all_descriptors[0]);
    
    // First test with xeon_mp = 0 (to hit case 0x49)
    xeon_mp = 0;
    printf("\nTesting with xeon_mp = 0 (should set L2 for 0x49):\n");
    simulate_cpuid_leaf2(all_descriptors, count);
    
    // Then test with xeon_mp = 1 (to skip case 0x49)
    xeon_mp = 1;
    printf("\nTesting with xeon_mp = 1 (should skip L2 for 0x49):\n");
    simulate_cpuid_leaf2(all_descriptors, count);
}

// Test specific edge cases
void test_edge_cases() {
    printf("\n=== Testing Edge Cases ===\n");
    
    // Test case: AL > 1 with mixed valid descriptors
    uint8_t mixed_descriptors[] = {0x03, 0x0a, 0x0c, 0x21, 0x49, 0x7a, 0xFF};
    printf("\nMixed descriptors including 0xFF (invalid):\n");
    simulate_cpuid_leaf2(mixed_descriptors, sizeof(mixed_descriptors));
    
    // Test case: Only L1 descriptors
    uint8_t l1_only[] = {0x0a, 0x0c, 0x0d, 0x0e, 0x66, 0x67, 0x68};
    printf("\nL1 cache descriptors only:\n");
    simulate_cpuid_leaf2(l1_only, sizeof(l1_only));
    
    // Test case: Only L2 descriptors
    uint8_t l2_only[] = {0x21, 0x24, 0x39, 0x3a, 0x49, 0x78, 0x79};
    printf("\nL2 cache descriptors only:\n");
    simulate_cpuid_leaf2(l2_only, sizeof(l2_only));
}

int main() {
    printf("Cache Descriptor Coverage Test Program\n");
    printf("======================================\n");
    
    // Test 1: Call real CPUID functions
    call_real_cpuid_leaf2();
    call_real_cpuid_leaf4();
    
    // Test 2: Simulate all uncovered cases
    test_all_uncovered_cases();
    
    // Test 3: Edge cases
    test_edge_cases();
    
    // Force compiler to keep all code (prevent dead code elimination)
    volatile int keep = 0;
    if (keep) {
        // This will never execute but prevents optimization
        __cpuid_count(0x00, 0, keep, keep, keep, keep);
    }
    
    return 0;
}
