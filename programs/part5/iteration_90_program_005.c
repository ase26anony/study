/*
 * This program triggers Intel CPU cache detection logic by calling CPUID leaf 0x2
 * and processing cache descriptor bytes matching the uncovered cases from driver-i386.cc.
 * It implements the exact switch cases for cache descriptor values 0x0a through 0x87.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Cross-platform CPUID intrinsic wrappers */
#if defined(_WIN32) || defined(_WIN64)
#include <intrin.h>
#include <windows.h>
#define cpuid(info, x) __cpuid(info, x)
#define cpuidex(info, x, y) __cpuidex(info, x, y)
#else
#include <cpuid.h>
#define cpuid(info, x) __cpuid(x, info[0], info[1], info[2], info[3])
#define cpuidex(info, x, y) __cpuid_count(x, y, info[0], info[1], info[2], info[3])
#endif

/* Cache descriptor structure matching driver-i386.cc */
struct cache_desc {
    int sizekb;     /* Size in KB */
    int assoc;      /* Associativity */
    int line;       /* Line size in bytes */
    int valid;      /* Whether this entry is populated */
};

/* Global flag to simulate xeon_mp check from CPUID leaf 0x1 */
static int xeon_mp = 0;

/* Function to get CPU family/model/stepping and set xeon_mp flag */
static void detect_cpu_type(void) {
    uint32_t regs[4] = {0};
    cpuid(regs, 1);
    
    /* Extract family/model/stepping */
    uint32_t stepping = regs[0] & 0xF;
    uint32_t model = (regs[0] >> 4) & 0xF;
    uint32_t family = (regs[0] >> 8) & 0xF;
    uint32_t extended_model = (regs[0] >> 16) & 0xF;
    uint32_t extended_family = (regs[0] >> 20) & 0xFF;
    
    /* For demonstration: simulate Xeon MP detection */
    /* Real detection would check specific family/model combinations */
    if (family == 0xF && extended_family == 0) {
        if (model == 0x6 || model == 0x7) { /* Example Xeon MP models */
            xeon_mp = 1;
        }
    }
    printf("CPU: Family %d, Model %d, Stepping %d, Xeon_MP=%d\n",
           family + extended_family, model + (extended_model << 4), 
           stepping, xeon_mp);
}

/* Process a single cache descriptor byte */
static void process_descriptor(uint8_t desc, struct cache_desc *level1, 
                               struct cache_desc *level2, int *l1_found, int *l2_found) {
    switch (desc) {
        /* L1 Data Cache cases */
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            level1->valid = 1; *l1_found = 1;
            printf("  Descriptor 0x%02x: L1 Data Cache 8KB, 2-way, 32B line\n", desc);
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            level1->valid = 1; *l1_found = 1;
            printf("  Descriptor 0x%02x: L1 Data Cache 16KB, 4-way, 32B line\n", desc);
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            level1->valid = 1; *l1_found = 1;
            printf("  Descriptor 0x%02x: L1 Data Cache 16KB, 4-way, 64B line\n", desc);
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            level1->valid = 1; *l1_found = 1;
            printf("  Descriptor 0x%02x: L1 Data Cache 24KB, 6-way, 64B line\n", desc);
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            level1->valid = 1; *l1_found = 1;
            printf("  Descriptor 0x%02x: L1 Data Cache 32KB, 8-way, 64B line\n", desc);
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            level1->valid = 1; *l1_found = 1;
            printf("  Descriptor 0x%02x: L1 Data Cache 16KB, 8-way, 64B line\n", desc);
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            level1->valid = 1; *l1_found = 1;
            printf("  Descriptor 0x%02x: L1 Data Cache 8KB, 4-way, 64B line\n", desc);
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            level1->valid = 1; *l1_found = 1;
            printf("  Descriptor 0x%02x: L1 Data Cache 16KB, 4-way, 64B line\n", desc);
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            level1->valid = 1; *l1_found = 1;
            printf("  Descriptor 0x%02x: L1 Data Cache 32KB, 4-way, 64B line\n", desc);
            break;
            
        /* L2 Cache cases */
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 256KB, 8-way, 64B line\n", desc);
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 1024KB, 16-way, 64B line\n", desc);
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 128KB, 4-way, 64B line\n", desc);
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 192KB, 6-way, 64B line\n", desc);
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 128KB, 2-way, 64B line\n", desc);
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 256KB, 4-way, 64B line\n", desc);
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 384KB, 6-way, 64B line\n", desc);
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 512KB, 4-way, 64B line\n", desc);
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 128KB, 4-way, 32B line\n", desc);
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 256KB, 4-way, 32B line\n", desc);
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 512KB, 4-way, 32B line\n", desc);
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 1024KB, 4-way, 32B line\n", desc);
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 2048KB, 4-way, 32B line\n", desc);
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 3072KB, 12-way, 64B line\n", desc);
            break;
        case 0x49:
            if (!xeon_mp) {
                level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
                level2->valid = 1; *l2_found = 1;
                printf("  Descriptor 0x%02x: L2 Cache 4096KB, 16-way, 64B line (non-Xeon-MP)\n", desc);
            } else {
                printf("  Descriptor 0x%02x: Skipped due to Xeon MP\n", desc);
            }
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 6144KB, 24-way, 64B line\n", desc);
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 1024KB, 4-way, 64B line\n", desc);
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 128KB, 8-way, 64B line\n", desc);
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 256KB, 8-way, 64B line\n", desc);
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 512KB, 8-way, 64B line\n", desc);
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 1024KB, 8-way, 64B line\n", desc);
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 2048KB, 8-way, 64B line\n", desc);
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 512KB, 2-way, 64B line\n", desc);
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 512KB, 8-way, 64B line\n", desc);
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 256KB, 8-way, 32B line\n", desc);
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 512KB, 8-way, 32B line\n", desc);
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 1024KB, 8-way, 32B line\n", desc);
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 2048KB, 8-way, 32B line\n", desc);
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 512KB, 4-way, 64B line\n", desc);
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            level2->valid = 1; *l2_found = 1;
            printf("  Descriptor 0x%02x: L2 Cache 1024KB, 8-way, 64B line\n", desc);
            break;
            
        /* Valid descriptor but not in uncovered lines - ignore */
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x06:
        case 0x08:
        case 0x09:
        case 0x0b:
        case 0x10:
        case 0x15:
        case 0x1a:
        case 0x88:
        case 0x89:
        case 0x8a:
        case 0x8d:
        case 0x90:
        case 0x96:
        case 0x9b:
            printf("  Descriptor 0x%02x: Valid but not in uncovered lines\n", desc);
            break;
            
        default:
            /* Invalid or reserved descriptor */
            if (desc & 0x80) {
                printf("  Descriptor 0x%02x: Invalid (bit 7 set)\n", desc);
            }
            break;
    }
}

/* Extract and process cache descriptors from CPUID leaf 0x2 */
static void detect_cache_descriptors(struct cache_desc *level1, struct cache_desc *level2) {
    uint32_t regs[4];
    uint8_t descriptors[16];
    int iteration = 0;
    int l1_found = 0, l2_found = 0;
    
    printf("\nCPUID Leaf 0x2 Cache Descriptor Analysis:\n");
    
    /* According to Intel manual, CPUID leaf 0x2 may need multiple calls */
    for (iteration = 0; iteration < 16; iteration++) {
        cpuid(regs, 2);
        
        /* Extract descriptor bytes from EAX, EBX, ECX, EDX */
        descriptors[0] = (regs[0] >> 0) & 0xFF;
        descriptors[1] = (regs[0] >> 8) & 0xFF;
        descriptors[2] = (regs[0] >> 16) & 0xFF;
        descriptors[3] = (regs[0] >> 24) & 0xFF;
        descriptors[4] = (regs[1] >> 0) & 0xFF;
        descriptors[5] = (regs[1] >> 8) & 0xFF;
        descriptors[6] = (regs[1] >> 16) & 0xFF;
        descriptors[7] = (regs[1] >> 24) & 0xFF;
        descriptors[8] = (regs[2] >> 0) & 0xFF;
        descriptors[9] = (regs[2] >> 8) & 0xFF;
        descriptors[10] = (regs[2] >> 16) & 0xFF;
        descriptors[11] = (regs[2] >> 24) & 0xFF;
        descriptors[12] = (regs[3] >> 0) & 0xFF;
        descriptors[13] = (regs[3] >> 8) & 0xFF;
        descriptors[14] = (regs[3] >> 16) & 0xFF;
        descriptors[15] = (regs[3] >> 24) & 0xFF;
        
        printf("Iteration %d: ", iteration);
        for (int i = 0; i < 16; i++) {
            printf("%02x ", descriptors[i]);
        }
        printf("\n");
        
        /* Process each descriptor byte */
        for (int i = 0; i < 16; i++) {
            if (descriptors[i] == 0x00) {
                /* Terminator byte found */
                printf("Terminator byte (0x00) found at position %d\n", i);
                return;
            }
            
            /* Skip if bit 7 is set (invalid descriptor) */
            if (descriptors[i] & 0x80) {
                continue;
            }
            
            process_descriptor(descriptors[i], level1, level2, &l1_found, &l2_found);
        }
        
        /* Check if EAX[7:0] indicates no more iterations needed */
        if ((regs[0] & 0xFF) == 1) {
            printf("EAX[7:0] = 1, stopping iterations\n");
            break;
        }
    }
}

/* Demonstrate cache-aware computation using detected cache line size */
static void cache_aware_computation(struct cache_desc *level1, struct cache_desc *level2) {
    const int ARRAY_SIZE = 1024 * 1024; /* 1MB */
    static volatile int *array = NULL;
    int sum = 0;
    
    if (!level1->valid && !level2->valid) {
        printf("\nNo cache information available for computation\n");
        return;
    }
    
    /* Use the largest detected cache line size */
    int cache_line = 64; /* Default */
    if (level1->valid && level1->line > cache_line) cache_line = level1->line;
    if (level2->valid && level2->line > cache_line) cache_line = level2->line;
    
    printf("\nCache-aware computation using %d byte cache line:\n", cache_line);
    
    /* Allocate aligned memory */
    #if defined(_WIN32) || defined(_WIN64)
    array = (int*)_aligned_malloc(ARRAY_SIZE * sizeof(int), cache_line);
    #else
    if (posix_memalign((void**)&array, cache_line, ARRAY_SIZE * sizeof(int)) != 0) {
        printf("Memory allocation failed\n");
        return;
    }
    #endif
    
    if (!array) {
        printf("Memory allocation failed\n");
        return;
    }
    
    /* Initialize array with stride equal to cache line size */
    int elements_per_line = cache_line / sizeof(int);
    for (int i = 0; i < ARRAY_SIZE; i += elements_per_line) {
        array[i] = i;
    }
    
    /* Access array with cache-line aligned stride */
    for (int i = 0; i < ARRAY_SIZE; i += elements_per_line) {
        sum += array[i];
    }
    
    printf("Computation result: %d (prevents optimization removal)\n", sum);
    
    /* Clean up */
    #if defined(_WIN32) || defined(_WIN64)
    _aligned_free((void*)array);
    #else
    free((void*)array);
    #endif
}

int main(void) {
    struct cache_desc level1 = {0};
    struct cache_desc level2 = {0};
    
    printf("=== Intel CPU Cache Descriptor Detection ===\n");
    
    /* Step 1: Detect CPU type (sets xeon_mp flag) */
    detect_cpu_type();
    
    /* Step 2: Get cache descriptors via CPUID leaf 0x2 */
    detect_cache_descriptors(&level1, &level2);
    
    /* Step 3: Print detected cache information */
    printf("\n=== Detected Cache Configuration ===\n");
    if (level1.valid) {
        printf("L1 Data Cache: %d KB, %d-way, %d byte line\n",
               level1.sizekb, level1.assoc, level1.line);
    } else {
        printf("L1 Data Cache: Not detected in uncovered descriptors\n");
    }
    
    if (level2.valid) {
        printf("L2 Cache: %d KB, %d-way, %d byte line\n",
               level2.sizekb, level2.assoc, level2.line);
    } else {
        printf("L2 Cache: Not detected in uncovered descriptors\n");
    }
    
    /* Step 4: Perform cache-aware computation */
    cache_aware_computation(&level1, &level2);
    
    return 0;
}
