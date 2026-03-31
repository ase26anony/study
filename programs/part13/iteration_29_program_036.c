#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <cpuid.h>

// Mock structures matching the original driver-i386.cc
struct cache_desc {
    int sizekb;
    int assoc;
    int line;
};

// Global variables to track cache levels
struct cache_desc level1 = {0, 0, 0};
struct cache_desc level2 = {0, 0, 0};
int xeon_mp = 0;  // Set to 0 to hit the case 0x49 uncovered line

// Function to process cache descriptor bytes (mimicking the uncovered switch block)
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
            if (xeon_mp)
                break;
            level2.sizekb = 4096; level2.assoc = 16; level2.line = 64;
            printf("Processed 0x49: L2 4096KB, 16-way, 64B line (xeon_mp=%d)\n", xeon_mp);
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
            printf("Unknown descriptor: 0x%02x\n", descriptor);
            break;
    }
}

// Simulate CPUID leaf 0x02 with specific descriptor bytes
void simulate_cpuid_leaf2(uint8_t* descriptors, int count) {
    // First byte (AL) indicates number of valid descriptor bytes
    // We need AL > 1 to avoid early return (AL=1 uses different method)
    uint8_t first_byte = count + 1;  // Ensure AL > 1
    
    printf("CPUID Leaf 0x02: AL = 0x%02x (%d descriptors)\n", first_byte, count);
    
    // Process each descriptor byte
    for (int i = 0; i < count; i++) {
        // Skip null bytes and reserved values
        if (descriptors[i] != 0x00 && descriptors[i] != 0x01) {
            process_cache_descriptor(descriptors[i]);
        }
    }
}

// Test CPUID leaf 0x04 (deterministic cache parameters)
void test_cpuid_leaf4() {
    unsigned int eax, ebx, ecx, edx;
    int cache_index = 0;
    
    printf("\nTesting CPUID Leaf 0x04:\n");
    
    do {
        __cpuid_count(0x04, cache_index, eax, ebx, ecx, edx);
        
        int cache_type = eax & 0x1F;
        if (cache_type == 0) {
            printf("Cache index %d: No more caches\n", cache_index);
            break;
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
        
        printf("Cache %d: Type=%d, Level=%d, Size=%dKB, Ways=%d, Line=%dB\n",
               cache_index, cache_type, cache_level, size, ways, line_size);
        
        cache_index++;
    } while (1);
}

int main() {
    printf("=== Cache Descriptor Coverage Test ===\n\n");
    
    // All target descriptor bytes from uncovered lines
    uint8_t target_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    int num_descriptors = sizeof(target_descriptors) / sizeof(target_descriptors[0]);
    
    // Test 1: Simulate CPUID leaf 0x02 with all target descriptors
    printf("Test 1: Simulating CPUID leaf 0x02 with %d target descriptors\n", num_descriptors);
    simulate_cpuid_leaf2(target_descriptors, num_descriptors);
    
    // Test 2: Test case 0x49 with xeon_mp = 0 (to hit uncovered line)
    printf("\nTest 2: Testing case 0x49 with xeon_mp = 0\n");
    xeon_mp = 0;
    uint8_t single_descriptor[] = {0x49};
    simulate_cpuid_leaf2(single_descriptor, 1);
    
    // Test 3: Test case 0x49 with xeon_mp = 1 (should skip setting values)
    printf("\nTest 3: Testing case 0x49 with xeon_mp = 1\n");
    xeon_mp = 1;
    level2.sizekb = 0;  // Reset
    simulate_cpuid_leaf2(single_descriptor, 1);
    printf("After xeon_mp=1: level2.sizekb = %d (should be 0)\n", level2.sizekb);
    
    // Test 4: Actual CPUID calls (if supported)
    printf("\nTest 4: Actual CPUID calls\n");
    
    unsigned int eax, ebx, ecx, edx;
    
    // Check if CPUID leaf 0x02 is supported
    __cpuid(0, eax, ebx, ecx, edx);
    unsigned int max_leaf = eax;
    
    if (max_leaf >= 0x02) {
        printf("CPUID leaf 0x02 is supported (max leaf = 0x%x)\n", max_leaf);
        
        // Call CPUID leaf 0x02
        __cpuid(0x02, eax, ebx, ecx, edx);
        
        printf("CPUID(0x02): EAX=0x%08x, EBX=0x%08x, ECX=0x%08x, EDX=0x%08x\n",
               eax, ebx, ecx, edx);
        
        // Process the descriptor bytes
        uint8_t* bytes = (uint8_t*)&eax;
        int num_bytes = bytes[0];  // First byte of AL
        
        if (num_bytes > 1) {
            printf("Processing %d descriptor bytes from CPUID leaf 0x02:\n", num_bytes);
            
            // Process EAX (skip first byte which is the count)
            for (int i = 1; i < 4 && num_bytes > 0; i++, num_bytes--) {
                process_cache_descriptor(bytes[i]);
            }
            
            // Process EBX
            bytes = (uint8_t*)&ebx;
            for (int i = 0; i < 4 && num_bytes > 0; i++, num_bytes--) {
                process_cache_descriptor(bytes[i]);
            }
            
            // Process ECX
            bytes = (uint8_t*)&ecx;
            for (int i = 0; i < 4 && num_bytes > 0; i++, num_bytes--) {
                process_cache_descriptor(bytes[i]);
            }
            
            // Process EDX
            bytes = (uint8_t*)&edx;
            for (int i = 0; i < 4 && num_bytes > 0; i++, num_bytes--) {
                process_cache_descriptor(bytes[i]);
            }
        } else if (num_bytes == 1) {
            printf("CPUID leaf 0x02 AL=1, using TLB method (not covered)\n");
        }
    } else {
        printf("CPUID leaf 0x02 not supported\n");
    }
    
    // Test CPUID leaf 0x04 if supported
    if (max_leaf >= 0x04) {
        test_cpuid_leaf4();
    }
    
    printf("\n=== Test Complete ===\n");
    printf("Final L1 Cache: %dKB, %d-way, %dB line\n", 
           level1.sizekb, level1.assoc, level1.line);
    printf("Final L2 Cache: %dKB, %d-way, %dB line\n", 
           level2.sizekb, level2.assoc, level2.line);
    
    return 0;
}
