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

// Global variables to track cache levels
struct cache_desc level1 = {0, 0, 0};
struct cache_desc level2 = {0, 0, 0};
int xeon_mp = 0;  // We'll set this to 0 to hit the 0x49 case

// Function to process cache descriptor bytes (mimicking the uncovered logic)
void process_cache_descriptor(uint8_t descriptor) {
    switch (descriptor) {
        case 0x0a:
            level1.sizekb = 8; level1.assoc = 2; level1.line = 32;
            printf("Case 0x0a: L1 Cache - 8KB, 2-way, 32B line\n");
            break;
        case 0x0c:
            level1.sizekb = 16; level1.assoc = 4; level1.line = 32;
            printf("Case 0x0c: L1 Cache - 16KB, 4-way, 32B line\n");
            break;
        case 0x0d:
            level1.sizekb = 16; level1.assoc = 4; level1.line = 64;
            printf("Case 0x0d: L1 Cache - 16KB, 4-way, 64B line\n");
            break;
        case 0x0e:
            level1.sizekb = 24; level1.assoc = 6; level1.line = 64;
            printf("Case 0x0e: L1 Cache - 24KB, 6-way, 64B line\n");
            break;
        case 0x21:
            level2.sizekb = 256; level2.assoc = 8; level2.line = 64;
            printf("Case 0x21: L2 Cache - 256KB, 8-way, 64B line\n");
            break;
        case 0x24:
            level2.sizekb = 1024; level2.assoc = 16; level2.line = 64;
            printf("Case 0x24: L2 Cache - 1024KB, 16-way, 64B line\n");
            break;
        case 0x2c:
            level1.sizekb = 32; level1.assoc = 8; level1.line = 64;
            printf("Case 0x2c: L1 Cache - 32KB, 8-way, 64B line\n");
            break;
        case 0x39:
            level2.sizekb = 128; level2.assoc = 4; level2.line = 64;
            printf("Case 0x39: L2 Cache - 128KB, 4-way, 64B line\n");
            break;
        case 0x3a:
            level2.sizekb = 192; level2.assoc = 6; level2.line = 64;
            printf("Case 0x3a: L2 Cache - 192KB, 6-way, 64B line\n");
            break;
        case 0x3b:
            level2.sizekb = 128; level2.assoc = 2; level2.line = 64;
            printf("Case 0x3b: L2 Cache - 128KB, 2-way, 64B line\n");
            break;
        case 0x3c:
            level2.sizekb = 256; level2.assoc = 4; level2.line = 64;
            printf("Case 0x3c: L2 Cache - 256KB, 4-way, 64B line\n");
            break;
        case 0x3d:
            level2.sizekb = 384; level2.assoc = 6; level2.line = 64;
            printf("Case 0x3d: L2 Cache - 384KB, 6-way, 64B line\n");
            break;
        case 0x3e:
            level2.sizekb = 512; level2.assoc = 4; level2.line = 64;
            printf("Case 0x3e: L2 Cache - 512KB, 4-way, 64B line\n");
            break;
        case 0x41:
            level2.sizekb = 128; level2.assoc = 4; level2.line = 32;
            printf("Case 0x41: L2 Cache - 128KB, 4-way, 32B line\n");
            break;
        case 0x42:
            level2.sizekb = 256; level2.assoc = 4; level2.line = 32;
            printf("Case 0x42: L2 Cache - 256KB, 4-way, 32B line\n");
            break;
        case 0x43:
            level2.sizekb = 512; level2.assoc = 4; level2.line = 32;
            printf("Case 0x43: L2 Cache - 512KB, 4-way, 32B line\n");
            break;
        case 0x44:
            level2.sizekb = 1024; level2.assoc = 4; level2.line = 32;
            printf("Case 0x44: L2 Cache - 1024KB, 4-way, 32B line\n");
            break;
        case 0x45:
            level2.sizekb = 2048; level2.assoc = 4; level2.line = 32;
            printf("Case 0x45: L2 Cache - 2048KB, 4-way, 32B line\n");
            break;
        case 0x48:
            level2.sizekb = 3072; level2.assoc = 12; level2.line = 64;
            printf("Case 0x48: L2 Cache - 3072KB, 12-way, 64B line\n");
            break;
        case 0x49:
            if (xeon_mp)
                break;
            level2.sizekb = 4096; level2.assoc = 16; level2.line = 64;
            printf("Case 0x49: L2 Cache - 4096KB, 16-way, 64B line (xeon_mp = false)\n");
            break;
        case 0x4e:
            level2.sizekb = 6144; level2.assoc = 24; level2.line = 64;
            printf("Case 0x4e: L2 Cache - 6144KB, 24-way, 64B line\n");
            break;
        case 0x60:
            level1.sizekb = 16; level1.assoc = 8; level1.line = 64;
            printf("Case 0x60: L1 Cache - 16KB, 8-way, 64B line\n");
            break;
        case 0x66:
            level1.sizekb = 8; level1.assoc = 4; level1.line = 64;
            printf("Case 0x66: L1 Cache - 8KB, 4-way, 64B line\n");
            break;
        case 0x67:
            level1.sizekb = 16; level1.assoc = 4; level1.line = 64;
            printf("Case 0x67: L1 Cache - 16KB, 4-way, 64B line\n");
            break;
        case 0x68:
            level1.sizekb = 32; level1.assoc = 4; level1.line = 64;
            printf("Case 0x68: L1 Cache - 32KB, 4-way, 64B line\n");
            break;
        case 0x78:
            level2.sizekb = 1024; level2.assoc = 4; level2.line = 64;
            printf("Case 0x78: L2 Cache - 1024KB, 4-way, 64B line\n");
            break;
        case 0x79:
            level2.sizekb = 128; level2.assoc = 8; level2.line = 64;
            printf("Case 0x79: L2 Cache - 128KB, 8-way, 64B line\n");
            break;
        case 0x7a:
            level2.sizekb = 256; level2.assoc = 8; level2.line = 64;
            printf("Case 0x7a: L2 Cache - 256KB, 8-way, 64B line\n");
            break;
        case 0x7b:
            level2.sizekb = 512; level2.assoc = 8; level2.line = 64;
            printf("Case 0x7b: L2 Cache - 512KB, 8-way, 64B line\n");
            break;
        case 0x7c:
            level2.sizekb = 1024; level2.assoc = 8; level2.line = 64;
            printf("Case 0x7c: L2 Cache - 1024KB, 8-way, 64B line\n");
            break;
        case 0x7d:
            level2.sizekb = 2048; level2.assoc = 8; level2.line = 64;
            printf("Case 0x7d: L2 Cache - 2048KB, 8-way, 64B line\n");
            break;
        case 0x7f:
            level2.sizekb = 512; level2.assoc = 2; level2.line = 64;
            printf("Case 0x7f: L2 Cache - 512KB, 2-way, 64B line\n");
            break;
        case 0x80:
            level2.sizekb = 512; level2.assoc = 8; level2.line = 64;
            printf("Case 0x80: L2 Cache - 512KB, 8-way, 64B line\n");
            break;
        case 0x82:
            level2.sizekb = 256; level2.assoc = 8; level2.line = 32;
            printf("Case 0x82: L2 Cache - 256KB, 8-way, 32B line\n");
            break;
        case 0x83:
            level2.sizekb = 512; level2.assoc = 8; level2.line = 32;
            printf("Case 0x83: L2 Cache - 512KB, 8-way, 32B line\n");
            break;
        case 0x84:
            level2.sizekb = 1024; level2.assoc = 8; level2.line = 32;
            printf("Case 0x84: L2 Cache - 1024KB, 8-way, 32B line\n");
            break;
        case 0x85:
            level2.sizekb = 2048; level2.assoc = 8; level2.line = 32;
            printf("Case 0x85: L2 Cache - 2048KB, 8-way, 32B line\n");
            break;
        case 0x86:
            level2.sizekb = 512; level2.assoc = 4; level2.line = 64;
            printf("Case 0x86: L2 Cache - 512KB, 4-way, 64B line\n");
            break;
        case 0x87:
            level2.sizekb = 1024; level2.assoc = 8; level2.line = 64;
            printf("Case 0x87: L2 Cache - 1024KB, 8-way, 64B line\n");
            break;
        default:
            // Not one of our target cases
            break;
    }
}

// Function to simulate CPUID leaf 0x02 with specific descriptor bytes
void simulate_cpuid_leaf2(uint8_t* descriptors, int count) {
    printf("\n=== Simulating CPUID Leaf 0x02 with %d descriptors ===\n", count);
    
    // First byte indicates number of valid descriptor bytes (must be > 1)
    // We'll use a value that ensures we process all our descriptors
    uint8_t first_byte = count + 1;  // Make sure it's > 1
    
    printf("First byte (AL): 0x%02x (indicates %d descriptors)\n", first_byte, first_byte);
    
    // Process each descriptor byte
    for (int i = 0; i < count; i++) {
        process_cache_descriptor(descriptors[i]);
    }
}

// Function to call real CPUID leaf 0x04 (deterministic cache parameters)
void call_cpuid_leaf4() {
    printf("\n=== Calling CPUID Leaf 0x04 (Deterministic Cache Parameters) ===\n");
    
    uint32_t eax, ebx, ecx, edx;
    int cache_index = 0;
    
    do {
        __cpuid_count(0x04, cache_index, eax, ebx, ecx, edx);
        
        uint32_t cache_type = eax & 0x1F;
        if (cache_type == 0) {
            printf("Cache index %d: No more caches\n", cache_index);
            break;
        }
        
        uint32_t cache_level = (eax >> 5) & 0x07;
        uint32_t self_initializing = (eax >> 8) & 0x01;
        uint32_t fully_associative = (eax >> 9) & 0x01;
        uint32_t max_threads = ((eax >> 14) & 0xFFF) + 1;
        uint32_t max_cores = ((eax >> 26) & 0x3F) + 1;
        
        uint32_t line_size = (ebx & 0xFFF) + 1;
        uint32_t partitions = ((ebx >> 12) & 0x3FF) + 1;
        uint32_t ways = ((ebx >> 22) & 0x3FF) + 1;
        
        uint32_t sets = ecx + 1;
        
        uint32_t size = ways * partitions * line_size * sets / 1024;
        
        printf("Cache index %d: Level %d, Type %d, Size %dKB, %d-way, %dB line\n",
               cache_index, cache_level, cache_type, size, ways, line_size);
        
        cache_index++;
    } while (1);
}

int main() {
    printf("Cache Descriptor Coverage Test Program\n");
    printf("======================================\n");
    
    // Set xeon_mp to false to hit the 0x49 case
    xeon_mp = 0;
    printf("xeon_mp set to: %d (will trigger 0x49 case)\n", xeon_mp);
    
    // All the target descriptor values from uncovered lines
    uint8_t target_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    int num_descriptors = sizeof(target_descriptors) / sizeof(target_descriptors[0]);
    
    // Method 1: Simulate CPUID leaf 0x02 with all target descriptors
    simulate_cpuid_leaf2(target_descriptors, num_descriptors);
    
    // Method 2: Try real CPUID calls
    printf("\n=== Attempting Real CPUID Calls ===\n");
    
    uint32_t eax, ebx, ecx, edx;
    
    // Check if CPUID leaf 0x02 is supported
    __cpuid(0x00, eax, ebx, ecx, edx);
    uint32_t max_leaf = eax;
    printf("Maximum CPUID leaf: 0x%08x\n", max_leaf);
    
    if (max_leaf >= 0x02) {
        // Call CPUID leaf 0x02
        __cpuid(0x02, eax, ebx, ecx, edx);
        
        printf("CPUID Leaf 0x02 results:\n");
        printf("EAX: 0x%08x\n", eax);
        printf("EBX: 0x%08x\n", ebx);
        printf("ECX: 0x%08x\n", ecx);
        printf("EDX: 0x%08x\n", edx);
        
        // Check first byte of AL (must not be 0 or 1 to use descriptor table)
        uint8_t first_byte = eax & 0xFF;
        printf("First byte (AL): 0x%02x\n", first_byte);
        
        if (first_byte > 1) {
            // Process descriptor bytes from all registers
            uint8_t* regs = (uint8_t*)&eax;
            for (int i = 0; i < 16; i++) {
                if (regs[i] != 0 && (regs[i] & 0x80) == 0) {
                    process_cache_descriptor(regs[i]);
                }
            }
        }
    } else {
        printf("CPUID leaf 0x02 not supported on this processor\n");
    }
    
    // Call CPUID leaf 0x04 if supported
    if (max_leaf >= 0x04) {
        call_cpuid_leaf4();
    }
    
    // Summary
    printf("\n=== Summary ===\n");
    printf("L1 Cache configured: %dKB, %d-way, %dB line\n", 
           level1.sizekb, level1.assoc, level1.line);
    printf("L2 Cache configured: %dKB, %d-way, %dB line\n", 
           level2.sizekb, level2.assoc, level2.line);
    
    return 0;
}
