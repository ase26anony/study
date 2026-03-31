#include <stdio.h>
#include <stdint.h>
#include <cpuid.h>
#include <string.h>

/* Mock structures matching the original code */
struct cache_desc {
    int sizekb;
    int assoc;
    int line;
};

/* Global to simulate xeon_mp flag */
static int xeon_mp = 0;

/* Function to process cache descriptors - mimics the uncovered logic */
static void process_cache_descriptor(uint8_t desc, struct cache_desc *level1, 
                                     struct cache_desc *level2, int *got_level1, 
                                     int *got_level2) {
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
            /* Other descriptors not in uncovered lines */
            break;
    }
}

/* Simulate CPUID leaf 0x02 processing */
static void test_cpuid_leaf2_descriptors(void) {
    /* All target descriptor bytes from uncovered lines */
    uint8_t target_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    int got_level1 = 0;
    int got_level2 = 0;
    
    printf("Testing all uncovered cache descriptor cases:\n");
    printf("============================================\n");
    
    for (size_t i = 0; i < sizeof(target_descriptors); i++) {
        /* Reset for each test */
        memset(&level1, 0, sizeof(level1));
        memset(&level2, 0, sizeof(level2));
        got_level1 = 0;
        got_level2 = 0;
        
        /* Special handling for case 0x49 with xeon_mp flag */
        if (target_descriptors[i] == 0x49) {
            /* Test both branches */
            printf("\nTesting descriptor 0x%02x:\n", target_descriptors[i]);
            
            /* First with xeon_mp = 1 (should skip) */
            xeon_mp = 1;
            process_cache_descriptor(target_descriptors[i], &level1, &level2, 
                                     &got_level1, &got_level2);
            printf("  With xeon_mp=1: L1: %dKB, L2: %dKB\n", 
                   level1.sizekb, level2.sizekb);
            
            /* Then with xeon_mp = 0 (should set L2) */
            xeon_mp = 0;
            process_cache_descriptor(target_descriptors[i], &level1, &level2, 
                                     &got_level1, &got_level2);
            printf("  With xeon_mp=0: L1: %dKB, L2: %dKB\n", 
                   level1.sizekb, level2.sizekb);
        } else {
            printf("\nTesting descriptor 0x%02x: ", target_descriptors[i]);
            process_cache_descriptor(target_descriptors[i], &level1, &level2, 
                                     &got_level1, &got_level2);
            
            if (got_level1) {
                printf("L1: %dKB, %d-way, %dB line\n", 
                       level1.sizekb, level1.assoc, level1.line);
            } else if (got_level2) {
                printf("L2: %dKB, %d-way, %dB line\n", 
                       level2.sizekb, level2.assoc, level2.line);
            } else {
                printf("No match (shouldn't happen)\n");
            }
        }
    }
}

/* Test actual CPUID leaf 0x02 if available */
static void test_real_cpuid_leaf2(void) {
    uint32_t eax, ebx, ecx, edx;
    
    printf("\n\nTesting real CPUID leaf 0x02:\n");
    printf("=============================\n");
    
    /* Check if CPUID leaf 0x02 is supported */
    __cpuid(0, eax, ebx, ecx, edx);
    if (eax < 2) {
        printf("CPUID leaf 0x02 not supported (max leaf = %u)\n", eax);
        return;
    }
    
    /* Get CPUID leaf 0x02 */
    __cpuid(2, eax, ebx, ecx, edx);
    
    /* First byte of AL indicates number of valid descriptor bytes */
    uint8_t num_descriptors = eax & 0xFF;
    
    printf("CPUID leaf 0x02 returned: EAX=0x%08x, EBX=0x%08x, ECX=0x%08x, EDX=0x%08x\n",
           eax, ebx, ecx, edx);
    printf("Number of valid descriptor bytes in first byte of AL: %u\n", num_descriptors);
    
    if (num_descriptors == 0 || num_descriptors == 1) {
        printf("Early return condition: AL=%u\n", num_descriptors);
        return;
    }
    
    /* Process descriptor bytes */
    uint8_t *regs = (uint8_t *)&eax;
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    int got_level1 = 0;
    int got_level2 = 0;
    
    printf("Processing descriptor bytes:\n");
    
    /* Iterate through all bytes in the registers */
    for (int i = 1; i < 16; i++) {  /* Start at 1 to skip AL */
        uint8_t desc = regs[i];
        
        /* Skip invalid descriptors (0x00) */
        if (desc == 0x00) {
            continue;
        }
        
        /* Check if this is a descriptor we care about */
        process_cache_descriptor(desc, &level1, &level2, &got_level1, &got_level2);
        
        if (got_level1 || got_level2) {
            printf("  Byte %d: 0x%02x -> ", i, desc);
            if (got_level1) {
                printf("L1: %dKB, %d-way, %dB line\n", 
                       level1.sizekb, level1.assoc, level1.line);
            } else {
                printf("L2: %dKB, %d-way, %dB line\n", 
                       level2.sizekb, level2.assoc, level2.line);
            }
        }
    }
}

/* Test CPUID leaf 0x04 (deterministic cache parameters) */
static void test_cpuid_leaf4(void) {
    uint32_t eax, ebx, ecx, edx;
    int cache_level = 0;
    
    printf("\n\nTesting CPUID leaf 0x04 (deterministic cache parameters):\n");
    printf("=========================================================\n");
    
    for (int i = 0; ; i++) {
        __cpuid_count(0x04, i, eax, ebx, ecx, edx);
        
        /* Cache type field in bits 4:0 of EAX */
        uint32_t cache_type = eax & 0x1F;
        
        if (cache_type == 0) {
            printf("No more caches (type=0 at index %d)\n", i);
            break;
        }
        
        cache_level = (eax >> 5) & 0x7;
        printf("Cache %d: type=%u, level=%u\n", i, cache_type, cache_level);
        
        /* Decode cache parameters */
        if (cache_type == 1 || cache_type == 2) { /* Data or unified cache */
            uint32_t line_size = (ebx & 0xFFF) + 1;
            uint32_t partitions = ((ebx >> 12) & 0x3FF) + 1;
            uint32_t associativity = ((ebx >> 22) & 0x3FF) + 1;
            uint32_t sets = ecx + 1;
            
            uint32_t size_kb = (line_size * partitions * associativity * sets) / 1024;
            
            printf("  Size: %uKB, Line: %uB, Ways: %u, Sets: %u\n",
                   size_kb, line_size, associativity, sets);
        }
    }
}

int main(void) {
    printf("Cache Descriptor Test Program\n");
    printf("=============================\n");
    
    /* Test 1: Direct test of all uncovered descriptor cases */
    test_cpuid_leaf2_descriptors();
    
    /* Test 2: Real CPUID leaf 0x02 (if supported) */
    test_real_cpuid_leaf2();
    
    /* Test 3: CPUID leaf 0x04 for deterministic cache parameters */
    test_cpuid_leaf4();
    
    /* Force compiler to keep all code */
    volatile int dummy = 0;
    if (dummy) {
        /* This will never execute but prevents dead code elimination */
        __cpuid(0, dummy, dummy, dummy, dummy);
    }
    
    return 0;
}
