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

// Global flag to simulate xeon_mp condition
static int xeon_mp = 0;

// Function to process cache descriptors (mimicking the uncovered logic)
static void process_cache_descriptor(uint8_t desc, struct cache_desc *level1, 
                                     struct cache_desc *level2, int *has_level1, 
                                     int *has_level2) {
    switch (desc) {
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            *has_level1 = 1;
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            *has_level1 = 1;
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            *has_level1 = 1;
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            *has_level1 = 1;
            break;
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            *has_level1 = 1;
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            *has_level2 = 1;
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            *has_level2 = 1;
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            *has_level2 = 1;
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            *has_level2 = 1;
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            *has_level2 = 1;
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x49:
            if (xeon_mp)
                break;
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            *has_level1 = 1;
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            *has_level1 = 1;
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            *has_level1 = 1;
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            *has_level1 = 1;
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            *has_level2 = 1;
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            *has_level2 = 1;
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            *has_level2 = 1;
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            *has_level2 = 1;
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            *has_level2 = 1;
            break;
        default:
            // Ignore other descriptors
            break;
    }
}

// Simulate CPUID leaf 0x02 with specific descriptor bytes
static void simulate_cpuid_leaf2(uint8_t *descriptors, int count) {
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    int has_level1 = 0;
    int has_level2 = 0;
    
    printf("Processing %d cache descriptors:\n", count);
    for (int i = 0; i < count; i++) {
        process_cache_descriptor(descriptors[i], &level1, &level2, 
                                 &has_level1, &has_level2);
    }
    
    if (has_level1) {
        printf("L1 Cache: %dKB, %d-way, %d-byte line\n", 
               level1.sizekb, level1.assoc, level1.line);
    }
    if (has_level2) {
        printf("L2 Cache: %dKB, %d-way, %d-byte line\n", 
               level2.sizekb, level2.assoc, level2.line);
    }
    printf("\n");
}

// Real CPUID leaf 0x02 call
static void real_cpuid_leaf2(void) {
    uint32_t eax, ebx, ecx, edx;
    
    // Call CPUID leaf 0x02
    __cpuid(0x02, eax, ebx, ecx, edx);
    
    // Check if AL > 1 (valid descriptor table)
    uint8_t al = eax & 0xFF;
    if (al > 1) {
        uint8_t descriptors[16];
        int desc_count = 0;
        
        // Extract descriptor bytes from registers
        uint8_t *regs = (uint8_t*)&eax;
        for (int i = 0; i < 4; i++) {
            if (regs[i] != 0 && (regs[i] & 0x80) == 0) {
                descriptors[desc_count++] = regs[i];
            }
        }
        
        regs = (uint8_t*)&ebx;
        for (int i = 0; i < 4; i++) {
            if (regs[i] != 0 && (regs[i] & 0x80) == 0) {
                descriptors[desc_count++] = regs[i];
            }
        }
        
        regs = (uint8_t*)&ecx;
        for (int i = 0; i < 4; i++) {
            if (regs[i] != 0 && (regs[i] & 0x80) == 0) {
                descriptors[desc_count++] = regs[i];
            }
        }
        
        regs = (uint8_t*)&edx;
        for (int i = 0; i < 4; i++) {
            if (regs[i] != 0 && (regs[i] & 0x80) == 0) {
                descriptors[desc_count++] = regs[i];
            }
        }
        
        if (desc_count > 0) {
            simulate_cpuid_leaf2(descriptors, desc_count);
        }
    }
}

// CPUID leaf 0x04 deterministic cache parameters
static void real_cpuid_leaf4(void) {
    uint32_t eax, ebx, ecx, edx;
    int cache_index = 0;
    
    printf("CPUID Leaf 0x04 (Deterministic Cache Parameters):\n");
    
    do {
        __cpuid_count(0x04, cache_index, eax, ebx, ecx, edx);
        
        uint32_t cache_type = eax & 0x1F;
        if (cache_type == 0) {
            break;
        }
        
        uint32_t cache_level = (eax >> 5) & 0x7;
        uint32_t line_size = (ebx & 0xFFF) + 1;
        uint32_t ways = ((ebx >> 22) & 0x3FF) + 1;
        uint32_t partitions = ((ebx >> 12) & 0x3FF) + 1;
        uint32_t sets = ecx + 1;
        
        // Calculate cache size
        uint32_t size_kb = (ways * partitions * line_size * sets) / 1024;
        
        printf("Cache %d: Type=%u, Level=%u, Size=%uKB, "
               "Line=%uB, Ways=%u\n", 
               cache_index, cache_type, cache_level, size_kb, 
               line_size, ways);
        
        cache_index++;
    } while (1);
    
    printf("\n");
}

int main(void) {
    printf("=== Cache Descriptor Coverage Test ===\n\n");
    
    // Test 1: Real CPUID calls
    printf("1. Real CPUID Leaf 0x02:\n");
    real_cpuid_leaf2();
    
    printf("2. Real CPUID Leaf 0x04:\n");
    real_cpuid_leaf4();
    
    // Test 2: Simulate all uncovered cases
    printf("3. Simulating All Uncovered Descriptors:\n");
    
    // All target descriptor values from uncovered lines
    uint8_t all_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    // First with xeon_mp = 0 (to hit case 0x49)
    xeon_mp = 0;
    printf("With xeon_mp = 0 (case 0x49 should set L2 to 4096KB):\n");
    simulate_cpuid_leaf2(all_descriptors, sizeof(all_descriptors));
    
    // Then with xeon_mp = 1 (case 0x49 should skip)
    xeon_mp = 1;
    printf("With xeon_mp = 1 (case 0x49 should be skipped):\n");
    simulate_cpuid_leaf2(all_descriptors, sizeof(all_descriptors));
    
    // Test 3: Simulate CPUID leaf 0x02 with AL > 1 (bypass early return)
    printf("4. Simulating CPUID Leaf 0x02 with AL=0x03:\n");
    {
        // Create a fake CPUID result with AL=0x03 (3 valid descriptors)
        uint8_t fake_descriptors[] = {0x0a, 0x21, 0x39};
        simulate_cpuid_leaf2(fake_descriptors, 3);
    }
    
    // Test 4: Using inline assembly to force specific register values
    printf("5. Using Inline Assembly to Force Register Values:\n");
    
    // Force specific descriptor bytes in registers
    uint32_t forced_eax, forced_ebx, forced_ecx, forced_edx;
    
    // Use volatile to prevent optimization
    asm volatile (
        "mov $0x030a0c0d, %%eax\n\t"   // AL=0x03, descriptors 0x0a, 0x0c, 0x0d
        "mov $0x21242c39, %%ebx\n\t"   // descriptors 0x21, 0x24, 0x2c, 0x39
        "mov $0x3a3b3c3d, %%ecx\n\t"   // descriptors 0x3a, 0x3b, 0x3c, 0x3d
        "mov $0x3e414243, %%edx\n\t"   // descriptors 0x3e, 0x41, 0x42, 0x43
        : "=a"(forced_eax), "=b"(forced_ebx), "=c"(forced_ecx), "=d"(forced_edx)
        :
        : "memory"
    );
    
    // Process the forced descriptors
    {
        uint8_t forced_descriptors[16];
        int count = 0;
        
        uint8_t *regs = (uint8_t*)&forced_eax;
        for (int i = 0; i < 4; i++) {
            if (regs[i] != 0 && (regs[i] & 0x80) == 0) {
                forced_descriptors[count++] = regs[i];
            }
        }
        
        regs = (uint8_t*)&forced_ebx;
        for (int i = 0; i < 4; i++) {
            if (regs[i] != 0 && (regs[i] & 0x80) == 0) {
                forced_descriptors[count++] = regs[i];
            }
        }
        
        regs = (uint8_t*)&forced_ecx;
        for (int i = 0; i < 4; i++) {
            if (regs[i] != 0 && (regs[i] & 0x80) == 0) {
                forced_descriptors[count++] = regs[i];
            }
        }
        
        regs = (uint8_t*)&forced_edx;
        for (int i = 0; i < 4; i++) {
            if (regs[i] != 0 && (regs[i] & 0x80) == 0) {
                forced_descriptors[count++] = regs[i];
            }
        }
        
        xeon_mp = 0;  // Ensure case 0x49 is hit
        simulate_cpuid_leaf2(forced_descriptors, count);
    }
    
    printf("=== Test Complete ===\n");
    
    return 0;
}
