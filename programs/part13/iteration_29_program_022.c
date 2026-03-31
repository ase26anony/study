#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <cpuid.h>

// Structures matching driver-i386.cc
struct cache_desc {
    int sizekb;
    int assoc;
    int line;
};

// Global to simulate xeon_mp condition
int xeon_mp = 0;

// Function to process cache descriptors (mimicking the uncovered logic)
void process_cache_descriptor(uint8_t desc, struct cache_desc* level1, struct cache_desc* level2) {
    switch(desc) {
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            printf("Case 0x0a: L1 8KB, 2-way, 32B line\n");
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            printf("Case 0x0c: L1 16KB, 4-way, 32B line\n");
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("Case 0x0d: L1 16KB, 4-way, 64B line\n");
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            printf("Case 0x0e: L1 24KB, 6-way, 64B line\n");
            break;
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("Case 0x21: L2 256KB, 8-way, 64B line\n");
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            printf("Case 0x24: L2 1MB, 16-way, 64B line\n");
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            printf("Case 0x2c: L1 32KB, 8-way, 64B line\n");
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            printf("Case 0x39: L2 128KB, 4-way, 64B line\n");
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            printf("Case 0x3a: L2 192KB, 6-way, 64B line\n");
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            printf("Case 0x3b: L2 128KB, 2-way, 64B line\n");
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            printf("Case 0x3c: L2 256KB, 4-way, 64B line\n");
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            printf("Case 0x3d: L2 384KB, 6-way, 64B line\n");
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("Case 0x3e: L2 512KB, 4-way, 64B line\n");
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            printf("Case 0x41: L2 128KB, 4-way, 32B line\n");
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            printf("Case 0x42: L2 256KB, 4-way, 32B line\n");
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            printf("Case 0x43: L2 512KB, 4-way, 32B line\n");
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            printf("Case 0x44: L2 1MB, 4-way, 32B line\n");
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            printf("Case 0x45: L2 2MB, 4-way, 32B line\n");
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            printf("Case 0x48: L2 3MB, 12-way, 64B line\n");
            break;
        case 0x49:
            if (xeon_mp) {
                printf("Case 0x49: Xeon MP detected, skipping\n");
                break;
            }
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            printf("Case 0x49: L2 4MB, 16-way, 64B line (non-Xeon-MP)\n");
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            printf("Case 0x4e: L2 6MB, 24-way, 64B line\n");
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            printf("Case 0x60: L1 16KB, 8-way, 64B line\n");
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            printf("Case 0x66: L1 8KB, 4-way, 64B line\n");
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("Case 0x67: L1 16KB, 4-way, 64B line\n");
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            printf("Case 0x68: L1 32KB, 4-way, 64B line\n");
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            printf("Case 0x78: L2 1MB, 4-way, 64B line\n");
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            printf("Case 0x79: L2 128KB, 8-way, 64B line\n");
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("Case 0x7a: L2 256KB, 8-way, 64B line\n");
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("Case 0x7b: L2 512KB, 8-way, 64B line\n");
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("Case 0x7c: L2 1MB, 8-way, 64B line\n");
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            printf("Case 0x7d: L2 2MB, 8-way, 64B line\n");
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            printf("Case 0x7f: L2 512KB, 2-way, 64B line\n");
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("Case 0x80: L2 512KB, 8-way, 64B line\n");
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            printf("Case 0x82: L2 256KB, 8-way, 32B line\n");
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            printf("Case 0x83: L2 512KB, 8-way, 32B line\n");
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            printf("Case 0x84: L2 1MB, 8-way, 32B line\n");
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            printf("Case 0x85: L2 2MB, 8-way, 32B line\n");
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("Case 0x86: L2 512KB, 4-way, 64B line\n");
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("Case 0x87: L2 1MB, 8-way, 64B line\n");
            break;
        default:
            printf("Unknown descriptor: 0x%02x\n", desc);
            break;
    }
}

// Simulate CPUID leaf 0x02 with specific descriptor bytes
void simulate_cpuid_leaf2(uint8_t* descriptors, int count) {
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    
    printf("\n=== Simulating CPUID Leaf 0x02 Descriptors ===\n");
    for (int i = 0; i < count; i++) {
        process_cache_descriptor(descriptors[i], &level1, &level2);
    }
}

// Real CPUID calls for leaf 0x02 and leaf 0x04
void call_real_cpuid() {
    uint32_t eax, ebx, ecx, edx;
    
    printf("\n=== Real CPUID Calls ===\n");
    
    // Try CPUID leaf 0x02 (cache descriptors)
    __cpuid_count(0x02, 0, eax, ebx, ecx, edx);
    
    printf("CPUID Leaf 0x02: EAX=0x%08x EBX=0x%08x ECX=0x%08x EDX=0x%08x\n", 
           eax, ebx, ecx, edx);
    
    // Check if AL > 1 (valid descriptor table)
    uint8_t al = eax & 0xFF;
    if (al > 1) {
        printf("Valid descriptor table with %d bytes\n", al);
        
        // Process descriptor bytes from all registers
        uint8_t* regs = (uint8_t*)&eax;
        struct cache_desc level1 = {0, 0, 0};
        struct cache_desc level2 = {0, 0, 0};
        
        // Skip the first byte (AL) and process up to AL bytes
        for (int i = 1; i <= al && i < 16; i++) {
            if (regs[i] != 0 && (regs[i] & 0x80) == 0) { // Valid descriptor
                process_cache_descriptor(regs[i], &level1, &level2);
            }
        }
    } else if (al == 1) {
        printf("CPUID leaf 0x02 returns AL=1, using alternative method\n");
    } else {
        printf("CPUID leaf 0x02 not supported or invalid\n");
    }
    
    // Try CPUID leaf 0x04 (deterministic cache parameters)
    printf("\nCPUID Leaf 0x04 (Deterministic Cache Parameters):\n");
    for (int i = 0; ; i++) {
        __cpuid_count(0x04, i, eax, ebx, ecx, edx);
        
        uint32_t cache_type = eax & 0x1F;
        if (cache_type == 0) {
            printf("No more caches at index %d\n", i);
            break;
        }
        
        uint32_t cache_level = (eax >> 5) & 0x7;
        uint32_t ways = ((ebx >> 22) & 0x3FF) + 1;
        uint32_t partitions = ((ebx >> 12) & 0x3FF) + 1;
        uint32_t line_size = (ebx & 0xFFF) + 1;
        uint32_t sets = ecx + 1;
        
        uint32_t size_kb = (ways * partitions * line_size * sets) / 1024;
        
        printf("Cache[%d]: Type=%u, Level=%u, Size=%uKB, Ways=%u, Line=%uB\n",
               i, cache_type, cache_level, size_kb, ways, line_size);
    }
}

int main() {
    // All target descriptor values from uncovered lines
    uint8_t target_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    printf("=== Cache Descriptor Coverage Test ===\n");
    
    // Test 1: Simulate all target descriptors with xeon_mp = false
    printf("\nTest 1: Simulating all target descriptors (xeon_mp=false)\n");
    xeon_mp = 0;
    simulate_cpuid_leaf2(target_descriptors, sizeof(target_descriptors));
    
    // Test 2: Test case 0x49 with xeon_mp = true
    printf("\nTest 2: Testing case 0x49 with xeon_mp=true\n");
    xeon_mp = 1;
    uint8_t desc_49 = 0x49;
    struct cache_desc l1 = {0, 0, 0};
    struct cache_desc l2 = {0, 0, 0};
    process_cache_descriptor(desc_49, &l1, &l2);
    
    // Test 3: Test case 0x49 with xeon_mp = false (to hit the uncovered line)
    printf("\nTest 3: Testing case 0x49 with xeon_mp=false (targeting uncovered line)\n");
    xeon_mp = 0;
    process_cache_descriptor(desc_49, &l1, &l2);
    
    // Test 4: Make real CPUID calls
    call_real_cpuid();
    
    // Test 5: Simulate a CPUID leaf 0x02 response with AL > 1
    printf("\nTest 5: Simulating CPUID leaf 0x02 with AL=0x06 (6 descriptor bytes)\n");
    uint32_t simulated_regs[4];
    simulated_regs[0] = 0x06010203; // AL=0x06, descriptors: 0x01, 0x02, 0x03
    simulated_regs[1] = 0x04050607; // descriptors: 0x04, 0x05, 0x06, 0x07
    simulated_regs[2] = 0x08090a0b; // descriptors: 0x08, 0x09, 0x0a, 0x0b
    simulated_regs[3] = 0x0c0d0e0f; // descriptors: 0x0c, 0x0d, 0x0e, 0x0f
    
    uint8_t* reg_bytes = (uint8_t*)simulated_regs;
    struct cache_desc sim_l1 = {0, 0, 0};
    struct cache_desc sim_l2 = {0, 0, 0};
    
    // Process first 6 bytes (AL=0x06)
    for (int i = 1; i <= 0x06; i++) {
        if (reg_bytes[i] != 0 && (reg_bytes[i] & 0x80) == 0) {
            process_cache_descriptor(reg_bytes[i], &sim_l1, &sim_l2);
        }
    }
    
    return 0;
}
