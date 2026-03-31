#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <cpuid.h>

// Structures matching the original driver-i386.cc
struct cache_desc {
    int sizekb;
    int assoc;
    int line;
};

// Global variables to track cache levels
struct cache_desc level1 = {0, 0, 0};
struct cache_desc level2 = {0, 0, 0};
int xeon_mp = 0;  // We'll set this to 0 to hit the 0x49 case

// Function to process cache descriptor bytes (mimicking the original logic)
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
            printf("Case 0x24: L2 Cache - 1MB, 16-way, 64B line\n");
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
            printf("Case 0x44: L2 Cache - 1MB, 4-way, 32B line\n");
            break;
        case 0x45:
            level2.sizekb = 2048; level2.assoc = 4; level2.line = 32;
            printf("Case 0x45: L2 Cache - 2MB, 4-way, 32B line\n");
            break;
        case 0x48:
            level2.sizekb = 3072; level2.assoc = 12; level2.line = 64;
            printf("Case 0x48: L2 Cache - 3MB, 12-way, 64B line\n");
            break;
        case 0x49:
            if (xeon_mp) {
                printf("Case 0x49: Xeon MP detected, skipping L2 cache setup\n");
                break;
            }
            level2.sizekb = 4096; level2.assoc = 16; level2.line = 64;
            printf("Case 0x49: L2 Cache - 4MB, 16-way, 64B line (non-Xeon-MP)\n");
            break;
        case 0x4e:
            level2.sizekb = 6144; level2.assoc = 24; level2.line = 64;
            printf("Case 0x4e: L2 Cache - 6MB, 24-way, 64B line\n");
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
            printf("Case 0x78: L2 Cache - 1MB, 4-way, 64B line\n");
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
            printf("Case 0x7c: L2 Cache - 1MB, 8-way, 64B line\n");
            break;
        case 0x7d:
            level2.sizekb = 2048; level2.assoc = 8; level2.line = 64;
            printf("Case 0x7d: L2 Cache - 2MB, 8-way, 64B line\n");
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
            printf("Case 0x84: L2 Cache - 1MB, 8-way, 32B line\n");
            break;
        case 0x85:
            level2.sizekb = 2048; level2.assoc = 8; level2.line = 32;
            printf("Case 0x85: L2 Cache - 2MB, 8-way, 32B line\n");
            break;
        case 0x86:
            level2.sizekb = 512; level2.assoc = 4; level2.line = 64;
            printf("Case 0x86: L2 Cache - 512KB, 4-way, 64B line\n");
            break;
        case 0x87:
            level2.sizekb = 1024; level2.assoc = 8; level2.line = 64;
            printf("Case 0x87: L2 Cache - 1MB, 8-way, 64B line\n");
            break;
        default:
            // Ignore other descriptors as in the original code
            break;
    }
}

// Simulate CPUID leaf 0x02 with specific descriptor bytes
void simulate_cpuid_leaf2() {
    printf("\n=== Simulating CPUID Leaf 0x02 with target descriptors ===\n");
    
    // Target descriptor bytes from uncovered lines
    uint8_t target_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    // Simulate the iteration through descriptor bytes
    // First byte (AL) indicates number of valid descriptor bytes
    uint8_t first_byte = sizeof(target_descriptors) + 1; // +1 for the count byte itself
    
    printf("First byte (AL): 0x%02x (indicating %d descriptor bytes)\n", 
           first_byte, first_byte - 1);
    
    // Process each descriptor byte
    for (size_t i = 0; i < sizeof(target_descriptors); i++) {
        process_cache_descriptor(target_descriptors[i]);
    }
}

// Real CPUID leaf 0x02 call
void real_cpuid_leaf2() {
    printf("\n=== Real CPUID Leaf 0x02 Call ===\n");
    
    uint32_t eax, ebx, ecx, edx;
    
    // Call CPUID leaf 0x02
    __cpuid_count(0x02, 0, eax, ebx, ecx, edx);
    
    printf("CPUID Leaf 0x02 results:\n");
    printf("EAX: 0x%08x\n", eax);
    printf("EBX: 0x%08x\n", ebx);
    printf("ECX: 0x%08x\n", ecx);
    printf("EDX: 0x%08x\n", edx);
    
    // Check first byte of AL
    uint8_t first_byte = eax & 0xFF;
    printf("First byte (AL): 0x%02x\n", first_byte);
    
    if (first_byte == 0) {
        printf("No valid cache descriptors returned\n");
        return;
    }
    
    if (first_byte == 1) {
        printf("Using alternative cache determination method\n");
        return;
    }
    
    // Process descriptor bytes from all registers
    uint8_t *regs = (uint8_t*)&eax;
    int num_bytes = first_byte;
    
    printf("Processing %d descriptor bytes:\n", num_bytes);
    
    // Start from byte 1 (skip the count byte)
    for (int i = 1; i < num_bytes && i < 16; i++) {
        uint8_t descriptor = regs[i];
        
        // Skip null descriptors and descriptor 0xFF
        if (descriptor == 0x00 || descriptor == 0xFF) {
            continue;
        }
        
        printf("  Descriptor 0x%02x: ", descriptor);
        process_cache_descriptor(descriptor);
    }
}

// Real CPUID leaf 0x04 calls (deterministic cache parameters)
void real_cpuid_leaf4() {
    printf("\n=== Real CPUID Leaf 0x04 Calls ===\n");
    
    uint32_t eax, ebx, ecx, edx;
    int cache_level = 0;
    
    while (1) {
        __cpuid_count(0x04, cache_level, eax, ebx, ecx, edx);
        
        uint32_t cache_type = eax & 0x1F;
        
        printf("Cache Level %d:\n", cache_level);
        printf("  Cache Type: %u (0 = No more caches)\n", cache_type);
        printf("  Cache Level: %u\n", (eax >> 5) & 0x7);
        printf("  Self Initializing: %u\n", (eax >> 8) & 0x1);
        printf("  Fully Associative: %u\n", (eax >> 9) & 0x1);
        printf("  Max Threads: %u\n", (eax >> 14) & 0x3FF);
        printf("  Max Cores: %u\n", (eax >> 26) & 0x3F);
        
        if (cache_type == 0) {
            printf("No more caches to enumerate\n");
            break;
        }
        
        uint32_t ways = ((ebx >> 22) & 0x3FF) + 1;
        uint32_t partitions = ((ebx >> 12) & 0x3FF) + 1;
        uint32_t line_size = (ebx & 0xFFF) + 1;
        uint32_t sets = ecx + 1;
        
        printf("  Ways of Associativity: %u\n", ways);
        printf("  Partitions: %u\n", partitions);
        printf("  Line Size: %u bytes\n", line_size);
        printf("  Sets: %u\n", sets);
        
        // Calculate cache size
        uint32_t size_kb = (ways * partitions * line_size * sets) / 1024;
        printf("  Calculated Size: %u KB\n", size_kb);
        
        cache_level++;
    }
}

// Test specific case 0x49 with xeon_mp = false
void test_case_0x49() {
    printf("\n=== Testing Case 0x49 (Xeon MP check) ===\n");
    
    // Reset cache structures
    memset(&level1, 0, sizeof(level1));
    memset(&level2, 0, sizeof(level2));
    
    // Set xeon_mp to false to hit the uncovered line
    xeon_mp = 0;
    
    printf("xeon_mp = %d (false)\n", xeon_mp);
    process_cache_descriptor(0x49);
    
    printf("Result: L2 Cache size = %d KB\n", level2.sizekb);
    
    // Now test with xeon_mp = true
    printf("\nTesting with xeon_mp = true:\n");
    memset(&level2, 0, sizeof(level2));
    xeon_mp = 1;
    
    printf("xeon_mp = %d (true)\n", xeon_mp);
    process_cache_descriptor(0x49);
    
    printf("Result: L2 Cache size = %d KB\n", level2.sizekb);
}

int main() {
    printf("Cache Descriptor Coverage Test Program\n");
    printf("======================================\n");
    
    // Test 1: Simulate all target descriptor cases
    simulate_cpuid_leaf2();
    
    // Test 2: Make real CPUID calls
    real_cpuid_leaf2();
    
    // Test 3: Test CPUID leaf 0x04
    real_cpuid_leaf4();
    
    // Test 4: Specifically test case 0x49 with xeon_mp condition
    test_case_0x49();
    
    // Print final cache configuration
    printf("\n=== Final Cache Configuration ===\n");
    printf("L1 Cache: %d KB, %d-way, %d byte line\n", 
           level1.sizekb, level1.assoc, level1.line);
    printf("L2 Cache: %d KB, %d-way, %d byte line\n", 
           level2.sizekb, level2.assoc, level2.line);
    
    return 0;
}
