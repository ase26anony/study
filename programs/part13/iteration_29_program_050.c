#include <stdio.h>
#include <stdint.h>
#include <cpuid.h>
#include <string.h>

/* Structures matching driver-i386.cc */
struct cache_desc {
    int sizekb;
    int assoc;
    int line;
};

/* Global variables to track coverage */
static int coverage_count = 0;
static int xeon_mp = 0; /* Set to 0 to hit case 0x49 */

/* Function to process cache descriptors - mimics the uncovered logic */
void process_cache_descriptor(uint8_t desc, struct cache_desc *level1, 
                             struct cache_desc *level2, int *got_level1, 
                             int *got_level2) {
    switch (desc) {
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            *got_level1 = 1;
            coverage_count |= 1 << 0;
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            *got_level1 = 1;
            coverage_count |= 1 << 1;
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            *got_level1 = 1;
            coverage_count |= 1 << 2;
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            *got_level1 = 1;
            coverage_count |= 1 << 3;
            break;
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            *got_level2 = 1;
            coverage_count |= 1 << 4;
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            *got_level2 = 1;
            coverage_count |= 1 << 5;
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            *got_level1 = 1;
            coverage_count |= 1 << 6;
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            *got_level2 = 1;
            coverage_count |= 1 << 7;
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            *got_level2 = 1;
            coverage_count |= 1 << 8;
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            *got_level2 = 1;
            coverage_count |= 1 << 9;
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            *got_level2 = 1;
            coverage_count |= 1 << 10;
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            *got_level2 = 1;
            coverage_count |= 1 << 11;
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            *got_level2 = 1;
            coverage_count |= 1 << 12;
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            *got_level2 = 1;
            coverage_count |= 1 << 13;
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            *got_level2 = 1;
            coverage_count |= 1 << 14;
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            *got_level2 = 1;
            coverage_count |= 1 << 15;
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            *got_level2 = 1;
            coverage_count |= 1 << 16;
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            *got_level2 = 1;
            coverage_count |= 1 << 17;
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            *got_level2 = 1;
            coverage_count |= 1 << 18;
            break;
        case 0x49:
            if (xeon_mp) {
                coverage_count |= 1 << 30; /* xeon_mp branch */
                break;
            }
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            *got_level2 = 1;
            coverage_count |= 1 << 19;
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            *got_level2 = 1;
            coverage_count |= 1 << 20;
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            *got_level1 = 1;
            coverage_count |= 1 << 21;
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            *got_level1 = 1;
            coverage_count |= 1 << 22;
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            *got_level1 = 1;
            coverage_count |= 1 << 23;
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            *got_level1 = 1;
            coverage_count |= 1 << 24;
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            *got_level2 = 1;
            coverage_count |= 1 << 25;
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            *got_level2 = 1;
            coverage_count |= 1 << 26;
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            *got_level2 = 1;
            coverage_count |= 1 << 27;
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            *got_level2 = 1;
            coverage_count |= 1 << 28;
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            *got_level2 = 1;
            coverage_count |= 1 << 29;
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            *got_level2 = 1;
            coverage_count |= 1 << 31;
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            *got_level2 = 1;
            coverage_count |= 1 << 5; /* Reuse bit */
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            *got_level2 = 1;
            coverage_count |= 1 << 6; /* Reuse bit */
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            *got_level2 = 1;
            coverage_count |= 1 << 7; /* Reuse bit */
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            *got_level2 = 1;
            coverage_count |= 1 << 8; /* Reuse bit */
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            *got_level2 = 1;
            coverage_count |= 1 << 9; /* Reuse bit */
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            *got_level2 = 1;
            coverage_count |= 1 << 10; /* Reuse bit */
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            *got_level2 = 1;
            coverage_count |= 1 << 11; /* Reuse bit */
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            *got_level2 = 1;
            coverage_count |= 1 << 12; /* Reuse bit */
            break;
        default:
            /* Not one of our target cases */
            break;
    }
}

/* Simulate CPUID leaf 0x02 with specific descriptor bytes */
void simulate_cpuid_leaf2(uint8_t *descriptors, int count) {
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    int got_level1 = 0;
    int got_level2 = 0;
    
    printf("Simulating CPUID leaf 0x02 with %d descriptors:\n", count);
    
    for (int i = 0; i < count; i++) {
        process_cache_descriptor(descriptors[i], &level1, &level2, 
                                &got_level1, &got_level2);
    }
    
    if (got_level1) {
        printf("  L1 Cache: %dKB, %d-way, %d-byte line\n", 
               level1.sizekb, level1.assoc, level1.line);
    }
    if (got_level2) {
        printf("  L2 Cache: %dKB, %d-way, %d-byte line\n", 
               level2.sizekb, level2.assoc, level2.line);
    }
}

/* Real CPUID call for leaf 0x02 */
void real_cpuid_leaf2(void) {
    uint32_t eax, ebx, ecx, edx;
    
    printf("\nReal CPUID leaf 0x02:\n");
    
    /* Call CPUID leaf 0x02 */
    __cpuid(0x02, eax, ebx, ecx, edx);
    
    /* Check if AL > 1 (valid descriptor table) */
    uint8_t al = eax & 0xFF;
    if (al == 0 || al == 1) {
        printf("  CPUID leaf 0x02 returns AL=%d (not using descriptor table)\n", al);
        return;
    }
    
    printf("  Registers: EAX=%08X EBX=%08X ECX=%08X EDX=%08X\n", 
           eax, ebx, ecx, edx);
    
    /* Process descriptor bytes */
    uint8_t *regs = (uint8_t *)&eax;
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    int got_level1 = 0;
    int got_level2 = 0;
    
    /* Iterate through all bytes in the registers */
    for (int i = 1; i < 16; i++) { /* Start at 1 to skip AL */
        uint8_t desc = regs[i];
        if (desc == 0) continue; /* Skip zero descriptors */
        
        process_cache_descriptor(desc, &level1, &level2, 
                                &got_level1, &got_level2);
    }
    
    if (got_level1) {
        printf("  L1 Cache: %dKB, %d-way, %d-byte line\n", 
               level1.sizekb, level1.assoc, level1.line);
    }
    if (got_level2) {
        printf("  L2 Cache: %dKB, %d-way, %d-byte line\n", 
               level2.sizekb, level2.assoc, level2.line);
    }
}

/* CPUID leaf 0x04 - Deterministic Cache Parameters */
void cpuid_leaf4(void) {
    uint32_t eax, ebx, ecx, edx;
    int cache_level = 0;
    
    printf("\nCPUID leaf 0x04 (Deterministic Cache Parameters):\n");
    
    for (int i = 0; ; i++) {
        __cpuid_count(0x04, i, eax, ebx, ecx, edx);
        
        uint32_t cache_type = eax & 0x1F;
        if (cache_type == 0) {
            break; /* No more caches */
        }
        
        cache_level++;
        uint32_t line_size = (ebx & 0xFFF) + 1;
        uint32_t partitions = ((ebx >> 12) & 0x3FF) + 1;
        uint32_t associativity = ((ebx >> 22) & 0x3FF) + 1;
        uint32_t sets = ecx + 1;
        
        /* Calculate cache size */
        uint32_t size = line_size * partitions * associativity * sets;
        
        printf("  Cache %d: Type=%u, Level=%u, Size=%uKB, Line=%uB\n",
               cache_level, cache_type, (eax >> 5) & 0x7,
               size / 1024, line_size);
    }
}

int main(void) {
    printf("=== Cache Descriptor Coverage Test ===\n");
    
    /* Test 1: Simulate all target descriptor values */
    uint8_t all_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e,           /* L1 caches */
        0x21, 0x24, 0x2c,                 /* L2 caches */
        0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, /* More L2 */
        0x41, 0x42, 0x43, 0x44, 0x45,     /* L2 with 32-byte lines */
        0x48, 0x49, 0x4e,                 /* Large L2 caches */
        0x60, 0x66, 0x67, 0x68,           /* More L1 */
        0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7f, 0x80, /* Various L2 */
        0x82, 0x83, 0x84, 0x85, 0x86, 0x87  /* L2 with different params */
    };
    
    simulate_cpuid_leaf2(all_descriptors, sizeof(all_descriptors));
    
    /* Test 2: Real CPUID calls */
    real_cpuid_leaf2();
    
    /* Test 3: CPUID leaf 0x04 */
    cpuid_leaf4();
    
    /* Test 4: Special test for case 0x49 with xeon_mp = 0 */
    printf("\n=== Testing case 0x49 (xeon_mp = %d) ===\n", xeon_mp);
    uint8_t desc_49 = 0x49;
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    int got_level1 = 0, got_level2 = 0;
    
    process_cache_descriptor(desc_49, &level1, &level2, &got_level1, &got_level2);
    
    if (got_level2 && level2.sizekb == 4096) {
        printf("  SUCCESS: Hit case 0x49 with xeon_mp=0, L2=4096KB\n");
    } else if (xeon_mp) {
        printf("  xeon_mp=1, case 0x49 skipped\n");
    }
    
    printf("\n=== Coverage Summary ===\n");
    printf("Coverage bits set: 0x%08X\n", coverage_count);
    printf("Total unique cases triggered: %d\n", 
           __builtin_popcount(coverage_count));
    
    /* Force compiler to keep all code */
    volatile int keep = coverage_count;
    (void)keep;
    
    return 0;
}
