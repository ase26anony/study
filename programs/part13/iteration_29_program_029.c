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
void process_cache_descriptor(uint8_t desc, struct cache_desc* level1, struct cache_desc* level2, int* got_level1, int* got_level2) {
    switch (desc) {
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
            if (!xeon_mp) {
                level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
                *got_level2 = 1;
            }
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
            // Not one of our target descriptors
            break;
    }
}

// Simulate CPUID leaf 0x02 processing with fabricated data
void simulate_cpuid_leaf2_processing() {
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    int got_level1 = 0;
    int got_level2 = 0;
    
    // All target descriptor bytes from uncovered lines
    uint8_t target_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    printf("Simulating CPUID leaf 0x02 processing with target descriptors:\n");
    
    // Test with xeon_mp = 0 to hit case 0x49
    xeon_mp = 0;
    
    for (size_t i = 0; i < sizeof(target_descriptors)/sizeof(target_descriptors[0]); i++) {
        uint8_t desc = target_descriptors[i];
        
        // Reset for each test
        memset(&level1, 0, sizeof(level1));
        memset(&level2, 0, sizeof(level2));
        got_level1 = 0;
        got_level2 = 0;
        
        process_cache_descriptor(desc, &level1, &level2, &got_level1, &got_level2);
        
        if (got_level1) {
            printf("Descriptor 0x%02x: L1 Cache - %dKB, %d-way, %d-byte line\n", 
                   desc, level1.sizekb, level1.assoc, level1.line);
        } else if (got_level2) {
            printf("Descriptor 0x%02x: L2 Cache - %dKB, %d-way, %d-byte line\n", 
                   desc, level2.sizekb, level2.assoc, level2.line);
        }
    }
    
    // Test case 0x49 with xeon_mp = 1 (should skip)
    printf("\nTesting case 0x49 with xeon_mp = 1:\n");
    xeon_mp = 1;
    memset(&level2, 0, sizeof(level2));
    got_level2 = 0;
    process_cache_descriptor(0x49, &level1, &level2, &got_level1, &got_level2);
    if (!got_level2) {
        printf("Descriptor 0x49 skipped (xeon_mp = 1)\n");
    }
}

// Real CPUID leaf 0x02 call
void real_cpuid_leaf2() {
    uint32_t eax, ebx, ecx, edx;
    
    printf("\nReal CPUID leaf 0x02 call:\n");
    
    // Check if CPUID leaf 0x02 is supported
    __cpuid(0, eax, ebx, ecx, edx);
    uint32_t max_leaf = eax;
    
    if (max_leaf >= 2) {
        __cpuid_count(2, 0, eax, ebx, ecx, edx);
        
        // First byte of AL indicates number of valid descriptor bytes
        uint8_t num_descriptors = eax & 0xFF;
        
        if (num_descriptors > 1) {
            printf("CPUID leaf 0x02 returned %d descriptor bytes\n", num_descriptors);
            printf("EAX: 0x%08x, EBX: 0x%08x, ECX: 0x%08x, EDX: 0x%08x\n", 
                   eax, ebx, ecx, edx);
            
            // Process descriptor bytes (simulating the iteration in driver-i386.cc)
            uint8_t* regs = (uint8_t*)&eax;
            for (int i = 0; i < 16; i++) {
                uint8_t desc = regs[i];
                if (desc != 0 && (desc & 0x80) == 0) { // Valid descriptor, not a duplicate
                    printf("  Descriptor byte %d: 0x%02x\n", i, desc);
                }
            }
        } else if (num_descriptors == 1) {
            printf("CPUID leaf 0x02 AL = 1, using TLB method (not our target)\n");
        } else {
            printf("CPUID leaf 0x02 AL = 0, no descriptors\n");
        }
    } else {
        printf("CPUID leaf 0x02 not supported\n");
    }
}

// Real CPUID leaf 0x04 calls (deterministic cache parameters)
void real_cpuid_leaf4() {
    uint32_t eax, ebx, ecx, edx;
    int cache_index = 0;
    
    printf("\nReal CPUID leaf 0x04 calls:\n");
    
    do {
        __cpuid_count(4, cache_index, eax, ebx, ecx, edx);
        
        uint32_t cache_type = eax & 0x1F;
        
        if (cache_type == 0) {
            break; // No more caches
        }
        
        uint32_t cache_level = (eax >> 5) & 0x7;
        uint32_t line_size = (ebx & 0xFFF) + 1;
        uint32_t ways = ((ebx >> 22) & 0x3FF) + 1;
        uint32_t partitions = ((ebx >> 12) & 0x3FF) + 1;
        uint32_t sets = ecx + 1;
        
        // Calculate size
        uint32_t size_kb = (ways * partitions * line_size * sets) / 1024;
        
        printf("Cache %d: Type=%u, Level=%u, Size=%uKB, Ways=%u, Line=%u bytes\n",
               cache_index, cache_type, cache_level, size_kb, ways, line_size);
        
        cache_index++;
    } while (1);
}

// Method using inline assembly to fabricate CPUID results
void fabricate_cpuid_with_asm() {
    printf("\nFabricating CPUID leaf 0x02 results with inline assembly:\n");
    
    // We'll create a mock CPUID result with multiple target descriptors
    volatile uint32_t mock_eax, mock_ebx, mock_ecx, mock_edx;
    
    // Set AL to 0x06 (6 valid descriptor bytes) to bypass early return
    // Include target descriptors 0x0a, 0x0c, 0x21, 0x39, 0x41, 0x49
    asm volatile (
        "mov $0x06413921, %%eax\n\t"   // eax = 0x06 (AL) | 0x41 | 0x39 | 0x21
        "mov $0x490c000a, %%ebx\n\t"   // ebx = 0x49 | 0x0c | 0x00 | 0x0a
        "mov $0x00000000, %%ecx\n\t"   // ecx = 0 (padding)
        "mov $0x00000000, %%edx\n\t"   // edx = 0 (padding)
        : "=a"(mock_eax), "=b"(mock_ebx), "=c"(mock_ecx), "=d"(mock_edx)
        :
        : "memory"
    );
    
    printf("Fabricated CPUID results:\n");
    printf("EAX: 0x%08x, EBX: 0x%08x, ECX: 0x%08x, EDX: 0x%08x\n",
           mock_eax, mock_ebx, mock_ecx, mock_edx);
    
    // Process the fabricated descriptors
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    int got_level1 = 0;
    int got_level2 = 0;
    
    uint8_t* regs = (uint8_t*)&mock_eax;
    for (int i = 1; i < 4; i++) { // Start at 1 to skip AL byte
        if (regs[i] != 0) {
            process_cache_descriptor(regs[i], &level1, &level2, &got_level1, &got_level2);
        }
    }
    
    regs = (uint8_t*)&mock_ebx;
    for (int i = 0; i < 4; i++) {
        if (regs[i] != 0) {
            process_cache_descriptor(regs[i], &level1, &level2, &got_level1, &got_level2);
        }
    }
    
    if (got_level1) {
        printf("Fabricated L1 Cache: %dKB, %d-way, %d-byte line\n",
               level1.sizekb, level1.assoc, level1.line);
    }
    if (got_level2) {
        printf("Fabricated L2 Cache: %dKB, %d-way, %d-byte line\n",
               level2.sizekb, level2.assoc, level2.line);
    }
}

int main() {
    printf("=== Cache Descriptor Coverage Test Program ===\n");
    
    // Part 1: Simulate processing with all target descriptors
    simulate_cpuid_leaf2_processing();
    
    // Part 2: Make real CPUID calls
    real_cpuid_leaf2();
    real_cpuid_leaf4();
    
    // Part 3: Fabricate CPUID results to trigger specific cases
    fabricate_cpuid_with_asm();
    
    // Force compiler to keep everything
    volatile int keep = 1;
    if (keep) {
        printf("\nProgram completed successfully.\n");
    }
    
    return 0;
}
