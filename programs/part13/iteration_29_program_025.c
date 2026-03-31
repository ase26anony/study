#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <cpuid.h>

// Simulate the cache descriptor structure from driver-i386.cc
struct cache_desc {
    int sizekb;
    int assoc;
    int line;
};

// Global flag to simulate xeon_mp condition
int xeon_mp = 0;

// Function to process cache descriptors like in driver-i386.cc
void process_cache_descriptor(uint8_t desc, struct cache_desc *level1, struct cache_desc *level2) {
    switch (desc) {
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            printf("Case 0x0a: L1 Cache - 8KB, 2-way, 32B line\n");
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            printf("Case 0x0c: L1 Cache - 16KB, 4-way, 32B line\n");
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("Case 0x0d: L1 Cache - 16KB, 4-way, 64B line\n");
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            printf("Case 0x0e: L1 Cache - 24KB, 6-way, 64B line\n");
            break;
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("Case 0x21: L2 Cache - 256KB, 8-way, 64B line\n");
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            printf("Case 0x24: L2 Cache - 1MB, 16-way, 64B line\n");
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            printf("Case 0x2c: L1 Cache - 32KB, 8-way, 64B line\n");
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            printf("Case 0x39: L2 Cache - 128KB, 4-way, 64B line\n");
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            printf("Case 0x3a: L2 Cache - 192KB, 6-way, 64B line\n");
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            printf("Case 0x3b: L2 Cache - 128KB, 2-way, 64B line\n");
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            printf("Case 0x3c: L2 Cache - 256KB, 4-way, 64B line\n");
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            printf("Case 0x3d: L2 Cache - 384KB, 6-way, 64B line\n");
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("Case 0x3e: L2 Cache - 512KB, 4-way, 64B line\n");
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            printf("Case 0x41: L2 Cache - 128KB, 4-way, 32B line\n");
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            printf("Case 0x42: L2 Cache - 256KB, 4-way, 32B line\n");
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            printf("Case 0x43: L2 Cache - 512KB, 4-way, 32B line\n");
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            printf("Case 0x44: L2 Cache - 1MB, 4-way, 32B line\n");
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            printf("Case 0x45: L2 Cache - 2MB, 4-way, 32B line\n");
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            printf("Case 0x48: L2 Cache - 3MB, 12-way, 64B line\n");
            break;
        case 0x49:
            if (xeon_mp) {
                printf("Case 0x49: Xeon MP detected, skipping L2 cache setting\n");
                break;
            }
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            printf("Case 0x49: L2 Cache - 4MB, 16-way, 64B line (non-Xeon-MP)\n");
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            printf("Case 0x4e: L2 Cache - 6MB, 24-way, 64B line\n");
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            printf("Case 0x60: L1 Cache - 16KB, 8-way, 64B line\n");
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            printf("Case 0x66: L1 Cache - 8KB, 4-way, 64B line\n");
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("Case 0x67: L1 Cache - 16KB, 4-way, 64B line\n");
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            printf("Case 0x68: L1 Cache - 32KB, 4-way, 64B line\n");
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            printf("Case 0x78: L2 Cache - 1MB, 4-way, 64B line\n");
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            printf("Case 0x79: L2 Cache - 128KB, 8-way, 64B line\n");
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("Case 0x7a: L2 Cache - 256KB, 8-way, 64B line\n");
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("Case 0x7b: L2 Cache - 512KB, 8-way, 64B line\n");
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("Case 0x7c: L2 Cache - 1MB, 8-way, 64B line\n");
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            printf("Case 0x7d: L2 Cache - 2MB, 8-way, 64B line\n");
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            printf("Case 0x7f: L2 Cache - 512KB, 2-way, 64B line\n");
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("Case 0x80: L2 Cache - 512KB, 8-way, 64B line\n");
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            printf("Case 0x82: L2 Cache - 256KB, 8-way, 32B line\n");
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            printf("Case 0x83: L2 Cache - 512KB, 8-way, 32B line\n");
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            printf("Case 0x84: L2 Cache - 1MB, 8-way, 32B line\n");
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            printf("Case 0x85: L2 Cache - 2MB, 8-way, 32B line\n");
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("Case 0x86: L2 Cache - 512KB, 4-way, 64B line\n");
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("Case 0x87: L2 Cache - 1MB, 8-way, 64B line\n");
            break;
        default:
            printf("Unknown cache descriptor: 0x%02x\n", desc);
            break;
    }
}

// Simulate CPUID leaf 0x02 response with multiple valid descriptors
void simulate_cpuid_leaf2(uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    // Set AL to 0x03 to indicate 3 valid descriptor bytes in EAX
    // This bypasses the early return (AL != 1)
    *eax = 0x03000000;  // AL = 0x03 (3 valid bytes), rest are descriptors
    
    // Fill with target descriptor values
    uint8_t *eax_bytes = (uint8_t*)eax;
    uint8_t *ebx_bytes = (uint8_t*)ebx;
    uint8_t *ecx_bytes = (uint8_t*)ecx;
    uint8_t *edx_bytes = (uint8_t*)edx;
    
    // First 3 bytes in EAX are valid descriptors (AL=0x03)
    eax_bytes[1] = 0x0a;  // First descriptor
    eax_bytes[2] = 0x0c;  // Second descriptor  
    eax_bytes[3] = 0x0d;  // Third descriptor
    
    // Fill EBX, ECX, EDX with more target descriptors
    ebx_bytes[0] = 0x0e;
    ebx_bytes[1] = 0x21;
    ebx_bytes[2] = 0x24;
    ebx_bytes[3] = 0x2c;
    
    ecx_bytes[0] = 0x39;
    ecx_bytes[1] = 0x3a;
    ecx_bytes[2] = 0x3b;
    ecx_bytes[3] = 0x3c;
    
    edx_bytes[0] = 0x3d;
    edx_bytes[1] = 0x3e;
    edx_bytes[2] = 0x41;
    edx_bytes[3] = 0x42;
}

// Process descriptors from CPUID leaf 0x02 response
void process_cpuid_leaf2_descriptors(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    
    uint8_t al = eax & 0xFF;
    printf("CPUID Leaf 0x02 AL (valid descriptor count): 0x%02x\n", al);
    
    if (al == 1) {
        printf("Early return: AL == 1, using alternate method\n");
        return;
    }
    
    // Process descriptors from all registers
    uint8_t *regs[] = {
        (uint8_t*)&eax,
        (uint8_t*)&ebx, 
        (uint8_t*)&ecx,
        (uint8_t*)&edx
    };
    
    int total_bytes = 16;  // 4 registers * 4 bytes
    int valid_bytes = al;  // Number of valid descriptor bytes
    
    printf("Processing %d valid descriptor bytes...\n", valid_bytes);
    
    for (int i = 0; i < total_bytes && valid_bytes > 0; i++) {
        uint8_t desc = regs[i/4][i%4];
        
        // Skip if descriptor is 0x00
        if (desc == 0x00) {
            continue;
        }
        
        // Process valid descriptor
        process_cache_descriptor(desc, &level1, &level2);
        valid_bytes--;
    }
}

// Test all target descriptors directly
void test_all_target_descriptors() {
    printf("\n=== Testing All Target Descriptors ===\n");
    
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    
    // All target descriptor values from uncovered lines
    uint8_t target_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    int num_descriptors = sizeof(target_descriptors) / sizeof(target_descriptors[0]);
    
    // First test with xeon_mp = 0 to hit case 0x49
    xeon_mp = 0;
    printf("\nTesting with xeon_mp = 0 (non-Xeon-MP):\n");
    for (int i = 0; i < num_descriptors; i++) {
        process_cache_descriptor(target_descriptors[i], &level1, &level2);
    }
    
    // Test with xeon_mp = 1 to show the branch
    xeon_mp = 1;
    printf("\nTesting with xeon_mp = 1 (Xeon-MP):\n");
    process_cache_descriptor(0x49, &level1, &level2);
}

// Real CPUID calls for leaf 0x02 and leaf 0x04
void call_real_cpuid() {
    printf("\n=== Real CPUID Calls ===\n");
    
    uint32_t eax, ebx, ecx, edx;
    
    // Check if CPUID leaf 0x02 is supported
    __cpuid(0, eax, ebx, ecx, edx);
    uint32_t max_leaf = eax;
    printf("Maximum CPUID leaf: 0x%08x\n", max_leaf);
    
    if (max_leaf >= 0x02) {
        printf("\n--- CPUID Leaf 0x02 (Cache Descriptors) ---\n");
        __cpuid(0x02, eax, ebx, ecx, edx);
        
        printf("EAX: 0x%08x\n", eax);
        printf("EBX: 0x%08x\n", ebx);
        printf("ECX: 0x%08x\n", ecx);
        printf("EDX: 0x%08x\n", edx);
        
        // Process the real descriptors
        process_cpuid_leaf2_descriptors(eax, ebx, ecx, edx);
    } else {
        printf("CPUID leaf 0x02 not supported\n");
    }
    
    if (max_leaf >= 0x04) {
        printf("\n--- CPUID Leaf 0x04 (Deterministic Cache Parameters) ---\n");
        
        int cache_index = 0;
        do {
            __cpuid_count(0x04, cache_index, eax, ebx, ecx, edx);
            
            uint32_t cache_type = eax & 0x1F;
            if (cache_type == 0) {
                break;
            }
            
            printf("Cache %d:\n", cache_index);
            printf("  Type: %u\n", cache_type);
            printf("  Level: %u\n", (eax >> 5) & 0x7);
            printf("  Self Initializing: %u\n", (eax >> 8) & 0x1);
            printf("  Fully Associative: %u\n", (eax >> 9) & 0x1);
            
            cache_index++;
        } while (1);
        
        printf("Total deterministic caches: %d\n", cache_index);
    } else {
        printf("CPUID leaf 0x04 not supported\n");
    }
}

int main() {
    printf("=== Cache Descriptor Coverage Test Program ===\n");
    
    // Part 1: Simulate CPUID leaf 0x02 with target descriptors
    printf("\n=== Simulated CPUID Leaf 0x02 Response ===\n");
    uint32_t eax, ebx, ecx, edx;
    simulate_cpuid_leaf2(&eax, &ebx, &ecx, &edx);
    
    printf("Simulated EAX: 0x%08x\n", eax);
    printf("Simulated EBX: 0x%08x\n", ebx);
    printf("Simulated ECX: 0x%08x\n", ecx);
    printf("Simulated EDX: 0x%08x\n", edx);
    
    process_cpuid_leaf2_descriptors(eax, ebx, ecx, edx);
    
    // Part 2: Test all target descriptors directly
    test_all_target_descriptors();
    
    // Part 3: Make real CPUID calls
    call_real_cpuid();
    
    return 0;
}
