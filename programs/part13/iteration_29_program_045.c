#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <cpuid.h>

// Mock structures matching driver-i386.cc
struct cache_desc {
    int sizekb;
    int assoc;
    int line;
};

// Global flag to simulate xeon_mp condition
int xeon_mp = 0;

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
            // Not a cache descriptor we're interested in
            break;
    }
}

// Simulate CPUID leaf 0x02 with fabricated data
void simulate_cpuid_leaf2(uint8_t* descriptors, int count) {
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    int got_level1 = 0;
    int got_level2 = 0;
    
    printf("Simulating CPUID leaf 0x02 with %d descriptors:\n", count);
    
    for (int i = 0; i < count; i++) {
        process_cache_descriptor(descriptors[i], &level1, &level2, &got_level1, &got_level2);
    }
    
    if (got_level1) {
        printf("  L1 Cache: %d KB, %d-way, %d-byte line\n", 
               level1.sizekb, level1.assoc, level1.line);
    }
    if (got_level2) {
        printf("  L2 Cache: %d KB, %d-way, %d-byte line\n", 
               level2.sizekb, level2.assoc, level2.line);
    }
    printf("\n");
}

// Real CPUID call for leaf 0x02
void call_real_cpuid_leaf2() {
    uint32_t eax, ebx, ecx, edx;
    
    printf("Calling real CPUID leaf 0x02:\n");
    
    // Check if leaf 0x02 is supported
    __cpuid(0, eax, ebx, ecx, edx);
    uint32_t max_leaf = eax;
    
    if (max_leaf < 2) {
        printf("  CPUID leaf 0x02 not supported\n");
        return;
    }
    
    // Call leaf 0x02
    __cpuid(2, eax, ebx, ecx, edx);
    
    // Check first byte of AL (eax)
    uint8_t first_byte = eax & 0xFF;
    
    if (first_byte == 1) {
        printf("  AL = 1: Using alternate method (not our target)\n");
        return;
    }
    
    if (first_byte == 0) {
        printf("  AL = 0: No valid descriptors\n");
        return;
    }
    
    // Process descriptors (mimicking the real logic)
    printf("  AL = 0x%02x: %d valid descriptor bytes\n", first_byte, first_byte);
    
    // Extract bytes from registers
    uint8_t* regs = (uint8_t*)&eax;
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    int got_level1 = 0;
    int got_level2 = 0;
    
    // Process all bytes (skip first byte which is the count)
    for (int i = 1; i < 16 && i <= first_byte; i++) {
        uint8_t descriptor = regs[i];
        if (descriptor == 0x00) continue; // Null descriptor
        
        process_cache_descriptor(descriptor, &level1, &level2, &got_level1, &got_level2);
    }
    
    if (got_level1) {
        printf("  L1 Cache: %d KB, %d-way, %d-byte line\n", 
               level1.sizekb, level1.assoc, level1.line);
    }
    if (got_level2) {
        printf("  L2 Cache: %d KB, %d-way, %d-byte line\n", 
               level2.sizekb, level2.assoc, level2.line);
    }
    printf("\n");
}

// Real CPUID call for leaf 0x04 (deterministic cache parameters)
void call_real_cpuid_leaf4() {
    uint32_t eax, ebx, ecx, edx;
    int cache_index = 0;
    
    printf("Calling CPUID leaf 0x04 (deterministic cache parameters):\n");
    
    do {
        __cpuid_count(0x04, cache_index, eax, ebx, ecx, edx);
        
        uint32_t cache_type = eax & 0x1F;
        
        if (cache_type == 0) {
            printf("  No more caches at index %d\n", cache_index);
            break;
        }
        
        uint32_t cache_level = (eax >> 5) & 0x7;
        uint32_t self_initializing = (eax >> 8) & 0x1;
        uint32_t fully_associative = (eax >> 9) & 0x1;
        uint32_t max_threads = ((eax >> 14) & 0xFFF) + 1;
        uint32_t max_cores = ((eax >> 26) & 0x3F) + 1;
        
        uint32_t line_size = (ebx & 0xFFF) + 1;
        uint32_t partitions = ((ebx >> 12) & 0x3FF) + 1;
        uint32_t ways = ((ebx >> 22) & 0x3FF) + 1;
        
        uint32_t sets = ecx + 1;
        
        uint32_t size = ways * partitions * line_size * sets / 1024;
        
        printf("  Cache %d: Type=%u, Level=%u, Size=%u KB, ", 
               cache_index, cache_type, cache_level, size);
        printf("Line=%u B, Ways=%u, Sets=%u\n", line_size, ways, sets);
        
        cache_index++;
    } while (1);
    
    printf("\n");
}

int main() {
    printf("=== Cache Descriptor Coverage Test ===\n\n");
    
    // Test 1: Real CPUID calls
    printf("1. Testing with real CPUID:\n");
    call_real_cpuid_leaf2();
    call_real_cpuid_leaf4();
    
    // Test 2: Simulate all uncovered cases
    printf("2. Simulating all target cache descriptors:\n");
    
    // All the target descriptor values from uncovered lines
    uint8_t all_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    // Test with xeon_mp = false (to hit case 0x49)
    xeon_mp = 0;
    printf("Testing with xeon_mp = false:\n");
    simulate_cpuid_leaf2(all_descriptors, sizeof(all_descriptors));
    
    // Test with xeon_mp = true (to show the branch)
    xeon_mp = 1;
    printf("Testing with xeon_mp = true (case 0x49 should skip):\n");
    simulate_cpuid_leaf2(all_descriptors, sizeof(all_descriptors));
    
    // Test 3: Simulate specific scenarios that would trigger the logic
    printf("3. Simulating specific CPUID leaf 0x02 scenarios:\n");
    
    // Scenario 1: AL = 0x03 (3 valid bytes) with L1 and L2 descriptors
    uint8_t scenario1[] = {0x03, 0x0a, 0x21, 0x00}; // AL=3, then descriptors
    simulate_cpuid_leaf2(scenario1, 4);
    
    // Scenario 2: AL = 0x06 with mixed descriptors
    uint8_t scenario2[] = {0x06, 0x0c, 0x2c, 0x39, 0x41, 0x66, 0x00};
    simulate_cpuid_leaf2(scenario2, 7);
    
    // Scenario 3: AL = 0x08 with many descriptors including 0x49
    xeon_mp = 0; // Ensure we hit the 0x49 case
    uint8_t scenario3[] = {0x08, 0x49, 0x4e, 0x60, 0x78, 0x7a, 0x82, 0x86, 0x87};
    simulate_cpuid_leaf2(scenario3, 9);
    
    printf("=== Test Complete ===\n");
    
    return 0;
}
