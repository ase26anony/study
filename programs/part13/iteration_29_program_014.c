#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <cpuid.h>

// Structure matching cache_desc from driver-i386.cc
struct cache_desc {
    int sizekb;
    int assoc;
    int line;
};

// Global variables to track cache levels
struct cache_desc level1 = {0, 0, 0};
struct cache_desc level2 = {0, 0, 0};
int xeon_mp = 0;  // We'll set this to 0 to hit the 0x49 case

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
            printf("Processed 0x24: L2 1MB, 16-way, 64B line\n");
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
            printf("Processed 0x44: L2 1MB, 4-way, 32B line\n");
            break;
        case 0x45:
            level2.sizekb = 2048; level2.assoc = 4; level2.line = 32;
            printf("Processed 0x45: L2 2MB, 4-way, 32B line\n");
            break;
        case 0x48:
            level2.sizekb = 3072; level2.assoc = 12; level2.line = 64;
            printf("Processed 0x48: L2 3MB, 12-way, 64B line\n");
            break;
        case 0x49:
            if (xeon_mp)
                break;
            level2.sizekb = 4096; level2.assoc = 16; level2.line = 64;
            printf("Processed 0x49: L2 4MB, 16-way, 64B line (xeon_mp=0)\n");
            break;
        case 0x4e:
            level2.sizekb = 6144; level2.assoc = 24; level2.line = 64;
            printf("Processed 0x4e: L2 6MB, 24-way, 64B line\n");
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
            printf("Processed 0x78: L2 1MB, 4-way, 64B line\n");
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
            printf("Processed 0x7c: L2 1MB, 8-way, 64B line\n");
            break;
        case 0x7d:
            level2.sizekb = 2048; level2.assoc = 8; level2.line = 64;
            printf("Processed 0x7d: L2 2MB, 8-way, 64B line\n");
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
            printf("Processed 0x84: L2 1MB, 8-way, 32B line\n");
            break;
        case 0x85:
            level2.sizekb = 2048; level2.assoc = 8; level2.line = 32;
            printf("Processed 0x85: L2 2MB, 8-way, 32B line\n");
            break;
        case 0x86:
            level2.sizekb = 512; level2.assoc = 4; level2.line = 64;
            printf("Processed 0x86: L2 512KB, 4-way, 64B line\n");
            break;
        case 0x87:
            level2.sizekb = 1024; level2.assoc = 8; level2.line = 64;
            printf("Processed 0x87: L2 1MB, 8-way, 64B line\n");
            break;
        default:
            printf("Unknown descriptor: 0x%02x\n", descriptor);
            break;
    }
}

// Simulate CPUID leaf 0x02 with descriptor table
void simulate_cpuid_leaf2() {
    printf("\n=== Simulating CPUID Leaf 0x02 ===\n");
    
    // Create a descriptor table with all target values
    // First byte (AL) = 0x1e (30 descriptors) to bypass early return
    uint8_t descriptors[] = {
        0x1e,  // Number of valid descriptor bytes
        // All the target descriptor values from uncovered lines
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    // Process each descriptor byte (skip the first count byte)
    for (int i = 1; i < sizeof(descriptors); i++) {
        if (descriptors[i] != 0x00 && descriptors[i] != 0xff) {
            process_cache_descriptor(descriptors[i]);
        }
    }
}

// Real CPUID leaf 0x02 call
void real_cpuid_leaf2() {
    printf("\n=== Real CPUID Leaf 0x02 ===\n");
    
    uint32_t eax, ebx, ecx, edx;
    
    // Call CPUID leaf 0x02
    __cpuid_count(0x02, 0, eax, ebx, ecx, edx);
    
    // Check if AL > 1 (valid descriptor table)
    uint8_t al = eax & 0xFF;
    if (al > 1) {
        printf("CPUID Leaf 0x02 returned AL=0x%02x (%d descriptors)\n", al, al);
        
        // Process bytes from EAX (skip AL)
        uint8_t *bytes = (uint8_t*)&eax;
        for (int i = 1; i < 4 && al > 0; i++, al--) {
            if (bytes[i] != 0x00 && bytes[i] != 0xff) {
                process_cache_descriptor(bytes[i]);
            }
        }
        
        // Process EBX
        bytes = (uint8_t*)&ebx;
        for (int i = 0; i < 4 && al > 0; i++, al--) {
            if (bytes[i] != 0x00 && bytes[i] != 0xff) {
                process_cache_descriptor(bytes[i]);
            }
        }
        
        // Process ECX
        bytes = (uint8_t*)&ecx;
        for (int i = 0; i < 4 && al > 0; i++, al--) {
            if (bytes[i] != 0x00 && bytes[i] != 0xff) {
                process_cache_descriptor(bytes[i]);
            }
        }
        
        // Process EDX
        bytes = (uint8_t*)&edx;
        for (int i = 0; i < 4 && al > 0; i++, al--) {
            if (bytes[i] != 0x00 && bytes[i] != 0xff) {
                process_cache_descriptor(bytes[i]);
            }
        }
    } else {
        printf("CPUID Leaf 0x02 AL=0x%02x (not using descriptor method)\n", al);
    }
}

// CPUID leaf 0x04 (deterministic cache parameters)
void cpuid_leaf4() {
    printf("\n=== CPUID Leaf 0x04 ===\n");
    
    uint32_t eax, ebx, ecx, edx;
    int cache_level = 0;
    
    // Call CPUID leaf 0x04 repeatedly until cache type is 0
    do {
        __cpuid_count(0x04, cache_level, eax, ebx, ecx, edx);
        
        uint32_t cache_type = eax & 0x1F;
        if (cache_type == 0) {
            break;
        }
        
        uint32_t cache_level_num = (eax >> 5) & 0x7;
        uint32_t ways = ((ebx >> 22) & 0x3FF) + 1;
        uint32_t partitions = ((ebx >> 12) & 0x3FF) + 1;
        uint32_t line_size = (ebx & 0xFFF) + 1;
        uint32_t sets = ecx + 1;
        
        // Calculate size
        uint32_t size_kb = (ways * partitions * line_size * sets) / 1024;
        
        printf("Cache Level %u: Type=%u, %uKB, %u-way, %uB line\n",
               cache_level_num, cache_type, size_kb, ways, line_size);
        
        cache_level++;
    } while (1);
}

// Test specific case 0x49 with xeon_mp = 0
void test_case_0x49() {
    printf("\n=== Testing Case 0x49 (xeon_mp=0) ===\n");
    
    // Reset cache descriptors
    level1.sizekb = 0; level1.assoc = 0; level1.line = 0;
    level2.sizekb = 0; level2.assoc = 0; level2.line = 0;
    
    // Set xeon_mp to 0 to hit the uncovered line
    xeon_mp = 0;
    
    // Process descriptor 0x49
    process_cache_descriptor(0x49);
    
    printf("Result: L2 size=%dKB, assoc=%d, line=%dB\n",
           level2.sizekb, level2.assoc, level2.line);
}

int main() {
    printf("Cache Descriptor Coverage Test Program\n");
    printf("======================================\n");
    
    // Test 1: Simulate all target descriptor values
    simulate_cpuid_leaf2();
    
    // Test 2: Real CPUID leaf 0x02 (if supported)
    real_cpuid_leaf2();
    
    // Test 3: CPUID leaf 0x04
    cpuid_leaf4();
    
    // Test 4: Specific test for case 0x49
    test_case_0x49();
    
    // Print final cache configuration
    printf("\n=== Final Cache Configuration ===\n");
    printf("L1 Cache: %dKB, %d-way, %dB line\n",
           level1.sizekb, level1.assoc, level1.line);
    printf("L2 Cache: %dKB, %d-way, %dB line\n",
           level2.sizekb, level2.assoc, level2.line);
    
    return 0;
}
