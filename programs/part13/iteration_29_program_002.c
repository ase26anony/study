#include <stdio.h>
#include <stdint.h>
#include <cpuid.h>
#include <string.h>
#include <stdlib.h>

// Structure matching the cache descriptor in driver-i386.cc
struct cache_desc {
    int sizekb;
    int assoc;
    int line;
};

// Global variables to track cache levels
struct cache_desc level1 = {0, 0, 0};
struct cache_desc level2 = {0, 0, 0};
int xeon_mp = 0;  // Set to 0 to hit the 0x49 case

// Function to process cache descriptor bytes (mimicking the uncovered logic)
void process_cache_descriptor(uint8_t descriptor) {
    switch (descriptor) {
        case 0x0a:
            level1.sizekb = 8; level1.assoc = 2; level1.line = 32;
            printf("Processed 0x0a: L1 8KB, 2-way, 32B line\n");
            break;
        case 0x0c:
            level1.sizekb = 16; level1.assoc = 4; level1.line = 32;
            printf("Processed 0x0c: L1 16KB, 4-way, 32B line\n");
            break;
        case 0x0d:
            level1.sizekb = 16; level1.assoc = 4; level1.line = 64;
            printf("Processed 0x0d: L1 16KB, 4-way, 64B line\n");
            break;
        case 0x0e:
            level1.sizekb = 24; level1.assoc = 6; level1.line = 64;
            printf("Processed 0x0e: L1 24KB, 6-way, 64B line\n");
            break;
        case 0x21:
            level2.sizekb = 256; level2.assoc = 8; level2.line = 64;
            printf("Processed 0x21: L2 256KB, 8-way, 64B line\n");
            break;
        case 0x24:
            level2.sizekb = 1024; level2.assoc = 16; level2.line = 64;
            printf("Processed 0x24: L2 1024KB, 16-way, 64B line\n");
            break;
        case 0x2c:
            level1.sizekb = 32; level1.assoc = 8; level1.line = 64;
            printf("Processed 0x2c: L1 32KB, 8-way, 64B line\n");
            break;
        case 0x39:
            level2.sizekb = 128; level2.assoc = 4; level2.line = 64;
            printf("Processed 0x39: L2 128KB, 4-way, 64B line\n");
            break;
        case 0x3a:
            level2.sizekb = 192; level2.assoc = 6; level2.line = 64;
            printf("Processed 0x3a: L2 192KB, 6-way, 64B line\n");
            break;
        case 0x3b:
            level2.sizekb = 128; level2.assoc = 2; level2.line = 64;
            printf("Processed 0x3b: L2 128KB, 2-way, 64B line\n");
            break;
        case 0x3c:
            level2.sizekb = 256; level2.assoc = 4; level2.line = 64;
            printf("Processed 0x3c: L2 256KB, 4-way, 64B line\n");
            break;
        case 0x3d:
            level2.sizekb = 384; level2.assoc = 6; level2.line = 64;
            printf("Processed 0x3d: L2 384KB, 6-way, 64B line\n");
            break;
        case 0x3e:
            level2.sizekb = 512; level2.assoc = 4; level2.line = 64;
            printf("Processed 0x3e: L2 512KB, 4-way, 64B line\n");
            break;
        case 0x41:
            level2.sizekb = 128; level2.assoc = 4; level2.line = 32;
            printf("Processed 0x41: L2 128KB, 4-way, 32B line\n");
            break;
        case 0x42:
            level2.sizekb = 256; level2.assoc = 4; level2.line = 32;
            printf("Processed 0x42: L2 256KB, 4-way, 32B line\n");
            break;
        case 0x43:
            level2.sizekb = 512; level2.assoc = 4; level2.line = 32;
            printf("Processed 0x43: L2 512KB, 4-way, 32B line\n");
            break;
        case 0x44:
            level2.sizekb = 1024; level2.assoc = 4; level2.line = 32;
            printf("Processed 0x44: L2 1024KB, 4-way, 32B line\n");
            break;
        case 0x45:
            level2.sizekb = 2048; level2.assoc = 4; level2.line = 32;
            printf("Processed 0x45: L2 2048KB, 4-way, 32B line\n");
            break;
        case 0x48:
            level2.sizekb = 3072; level2.assoc = 12; level2.line = 64;
            printf("Processed 0x48: L2 3072KB, 12-way, 64B line\n");
            break;
        case 0x49:
            if (xeon_mp) {
                printf("Processed 0x49: Xeon MP detected, skipping\n");
                break;
            }
            level2.sizekb = 4096; level2.assoc = 16; level2.line = 64;
            printf("Processed 0x49: L2 4096KB, 16-way, 64B line (non-Xeon-MP)\n");
            break;
        case 0x4e:
            level2.sizekb = 6144; level2.assoc = 24; level2.line = 64;
            printf("Processed 0x4e: L2 6144KB, 24-way, 64B line\n");
            break;
        case 0x60:
            level1.sizekb = 16; level1.assoc = 8; level1.line = 64;
            printf("Processed 0x60: L1 16KB, 8-way, 64B line\n");
            break;
        case 0x66:
            level1.sizekb = 8; level1.assoc = 4; level1.line = 64;
            printf("Processed 0x66: L1 8KB, 4-way, 64B line\n");
            break;
        case 0x67:
            level1.sizekb = 16; level1.assoc = 4; level1.line = 64;
            printf("Processed 0x67: L1 16KB, 4-way, 64B line\n");
            break;
        case 0x68:
            level1.sizekb = 32; level1.assoc = 4; level1.line = 64;
            printf("Processed 0x68: L1 32KB, 4-way, 64B line\n");
            break;
        case 0x78:
            level2.sizekb = 1024; level2.assoc = 4; level2.line = 64;
            printf("Processed 0x78: L2 1024KB, 4-way, 64B line\n");
            break;
        case 0x79:
            level2.sizekb = 128; level2.assoc = 8; level2.line = 64;
            printf("Processed 0x79: L2 128KB, 8-way, 64B line\n");
            break;
        case 0x7a:
            level2.sizekb = 256; level2.assoc = 8; level2.line = 64;
            printf("Processed 0x7a: L2 256KB, 8-way, 64B line\n");
            break;
        case 0x7b:
            level2.sizekb = 512; level2.assoc = 8; level2.line = 64;
            printf("Processed 0x7b: L2 512KB, 8-way, 64B line\n");
            break;
        case 0x7c:
            level2.sizekb = 1024; level2.assoc = 8; level2.line = 64;
            printf("Processed 0x7c: L2 1024KB, 8-way, 64B line\n");
            break;
        case 0x7d:
            level2.sizekb = 2048; level2.assoc = 8; level2.line = 64;
            printf("Processed 0x7d: L2 2048KB, 8-way, 64B line\n");
            break;
        case 0x7f:
            level2.sizekb = 512; level2.assoc = 2; level2.line = 64;
            printf("Processed 0x7f: L2 512KB, 2-way, 64B line\n");
            break;
        case 0x80:
            level2.sizekb = 512; level2.assoc = 8; level2.line = 64;
            printf("Processed 0x80: L2 512KB, 8-way, 64B line\n");
            break;
        case 0x82:
            level2.sizekb = 256; level2.assoc = 8; level2.line = 32;
            printf("Processed 0x82: L2 256KB, 8-way, 32B line\n");
            break;
        case 0x83:
            level2.sizekb = 512; level2.assoc = 8; level2.line = 32;
            printf("Processed 0x83: L2 512KB, 8-way, 32B line\n");
            break;
        case 0x84:
            level2.sizekb = 1024; level2.assoc = 8; level2.line = 32;
            printf("Processed 0x84: L2 1024KB, 8-way, 32B line\n");
            break;
        case 0x85:
            level2.sizekb = 2048; level2.assoc = 8; level2.line = 32;
            printf("Processed 0x85: L2 2048KB, 8-way, 32B line\n");
            break;
        case 0x86:
            level2.sizekb = 512; level2.assoc = 4; level2.line = 64;
            printf("Processed 0x86: L2 512KB, 4-way, 64B line\n");
            break;
        case 0x87:
            level2.sizekb = 1024; level2.assoc = 8; level2.line = 64;
            printf("Processed 0x87: L2 1024KB, 8-way, 64B line\n");
            break;
        default:
            // Skip invalid descriptors (0x00, 0x01, 0xff)
            if (descriptor != 0x00 && descriptor != 0x01 && descriptor != 0xff) {
                printf("Unhandled descriptor: 0x%02x\n", descriptor);
            }
            break;
    }
}

// Simulate CPUID leaf 0x02 with fabricated data containing target descriptors
void simulate_cpuid_leaf2() {
    printf("\n=== Simulating CPUID Leaf 0x02 ===\n");
    
    // Create a buffer with all target descriptor bytes
    // First byte (AL) = 0x1e (30 descriptors) to bypass early return
    uint8_t descriptors[] = {
        0x1e,  // Number of valid descriptor bytes (AL)
        // Target descriptors from uncovered lines
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    int num_descriptors = descriptors[0];
    printf("AL = 0x%02x (%d descriptor bytes)\n", descriptors[0], num_descriptors);
    
    // Process each descriptor byte (skip the first byte which is the count)
    for (int i = 1; i <= num_descriptors && i < sizeof(descriptors); i++) {
        process_cache_descriptor(descriptors[i]);
    }
}

// Call actual CPUID leaf 0x02 and process results
void call_real_cpuid_leaf2() {
    printf("\n=== Calling Real CPUID Leaf 0x02 ===\n");
    
    uint32_t eax, ebx, ecx, edx;
    
    // Call CPUID leaf 0x02
    __cpuid_count(0x02, 0, eax, ebx, ecx, edx);
    
    printf("CPUID(0x02) returned: EAX=0x%08x EBX=0x%08x ECX=0x%08x EDX=0x%08x\n", 
           eax, ebx, ecx, edx);
    
    // Check if AL > 1 (valid descriptor table)
    uint8_t al = eax & 0xFF;
    if (al == 0 || al == 1) {
        printf("AL = %d, using alternative cache detection method\n", al);
        return;
    }
    
    printf("AL = %d, processing descriptor bytes\n", al);
    
    // Process bytes from EAX (skip AL), EBX, ECX, EDX
    uint8_t *regs = (uint8_t*)&eax;
    for (int i = 1; i < 4; i++) {  // Skip AL (i=0)
        if (regs[i] != 0x00 && regs[i] != 0x01 && regs[i] != 0xFF) {
            process_cache_descriptor(regs[i]);
        }
    }
    
    regs = (uint8_t*)&ebx;
    for (int i = 0; i < 4; i++) {
        if (regs[i] != 0x00 && regs[i] != 0x01 && regs[i] != 0xFF) {
            process_cache_descriptor(regs[i]);
        }
    }
    
    regs = (uint8_t*)&ecx;
    for (int i = 0; i < 4; i++) {
        if (regs[i] != 0x00 && regs[i] != 0x01 && regs[i] != 0xFF) {
            process_cache_descriptor(regs[i]);
        }
    }
    
    regs = (uint8_t*)&edx;
    for (int i = 0; i < 4; i++) {
        if (regs[i] != 0x00 && regs[i] != 0x01 && regs[i] != 0xFF) {
            process_cache_descriptor(regs[i]);
        }
    }
}

// Call CPUID leaf 0x04 (deterministic cache parameters)
void call_cpuid_leaf4() {
    printf("\n=== Calling CPUID Leaf 0x04 ===\n");
    
    uint32_t eax, ebx, ecx, edx;
    int cache_level = 0;
    
    do {
        __cpuid_count(0x04, cache_level, eax, ebx, ecx, edx);
        
        int cache_type = eax & 0x1F;
        if (cache_type == 0) {
            printf("Cache level %d: No more caches\n", cache_level);
            break;
        }
        
        int cache_level_num = (eax >> 5) & 0x7;
        int self_initializing = (eax >> 8) & 0x1;
        int fully_associative = (eax >> 9) & 0x1;
        int max_threads = ((eax >> 14) & 0xFFF) + 1;
        int max_cores = ((eax >> 26) & 0x3F) + 1;
        
        int line_size = (ebx & 0xFFF) + 1;
        int partitions = ((ebx >> 12) & 0x3FF) + 1;
        int ways = ((ebx >> 22) & 0x3FF) + 1;
        
        int sets = ecx + 1;
        
        int size = ways * partitions * line_size * sets / 1024;
        
        printf("Cache level %d: Type=%d, Level=%d, Size=%dKB, Ways=%d, Line=%dB\n",
               cache_level, cache_type, cache_level_num, size, ways, line_size);
        
        cache_level++;
    } while (1);
}

// Test specific descriptor values directly
void test_specific_descriptors() {
    printf("\n=== Testing Specific Descriptor Values ===\n");
    
    // All target descriptor values from uncovered lines
    uint8_t target_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    for (int i = 0; i < sizeof(target_descriptors); i++) {
        // Reset cache structures for each test
        level1.sizekb = level1.assoc = level1.line = 0;
        level2.sizekb = level2.assoc = level2.line = 0;
        
        printf("\nTesting descriptor 0x%02x:\n", target_descriptors[i]);
        process_cache_descriptor(target_descriptors[i]);
        
        if (level1.sizekb > 0) {
            printf("  -> L1: %dKB, %d-way, %dB line\n", 
                   level1.sizekb, level1.assoc, level1.line);
        }
        if (level2.sizekb > 0) {
            printf("  -> L2: %dKB, %d-way, %dB line\n", 
                   level2.sizekb, level2.assoc, level2.line);
        }
    }
}

int main() {
    printf("Cache Descriptor Coverage Test Program\n");
    printf("======================================\n");
    
    // Ensure xeon_mp is false to hit the 0x49 case
    xeon_mp = 0;
    printf("xeon_mp = %d (will hit 0x49 case)\n", xeon_mp);
    
    // Method 1: Simulate CPUID with fabricated data
    simulate_cpuid_leaf2();
    
    // Method 2: Test specific descriptors directly
    test_specific_descriptors();
    
    // Method 3: Call real CPUID (if supported)
    call_real_cpuid_leaf2();
    
    // Method 4: Call CPUID leaf 0x04
    call_cpuid_leaf4();
    
    printf("\n=== Final Cache Configuration ===\n");
    printf("L1 Cache: %dKB, %d-way, %dB line\n", 
           level1.sizekb, level1.assoc, level1.line);
    printf("L2 Cache: %dKB, %d-way, %dB line\n", 
           level2.sizekb, level2.assoc, level2.line);
    
    return 0;
}
