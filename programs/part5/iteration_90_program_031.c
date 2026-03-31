/*
 * This program triggers the CPUID-based cache detection logic
 * to exercise the uncovered lines in driver-i386.cc (cases 0x0a through 0x87).
 * It implements Intel's CPUID leaf 0x2 cache descriptor parsing
 * with Xeon MP detection for case 0x49.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Cross-platform CPUID intrinsic wrappers */
#if defined(_WIN32) || defined(_WIN64)
#include <intrin.h>
#define cpuid(info, leaf, subleaf) __cpuid(info, leaf)
#define cpuidex(info, leaf, subleaf) __cpuidex(info, leaf, subleaf)
#else
#include <cpuid.h>
#define cpuid(info, leaf, subleaf) __cpuid_count(leaf, subleaf, info[0], info[1], info[2], info[3])
#define cpuidex(info, leaf, subleaf) __cpuid_count(leaf, subleaf, info[0], info[1], info[2], info[3])
#endif

/* Cache descriptor structure matching driver-i386.cc */
struct cache_desc {
    int sizekb;     /* Size in KB */
    int assoc;      /* Associativity */
    int line;       /* Line size in bytes */
    int valid;      /* Whether this entry is populated */
};

/* Global to simulate xeon_mp variable */
static int xeon_mp = 0;

/* Parse CPUID leaf 0x1 to determine if this is Xeon MP */
static void detect_xeon_mp(void) {
    uint32_t info[4];
    cpuid(info, 0x1, 0);
    
    /* Extract family, model, stepping */
    uint32_t stepping = info[0] & 0xF;
    uint32_t model = (info[0] >> 4) & 0xF;
    uint32_t family = (info[0] >> 8) & 0xF;
    uint32_t extended_model = (info[0] >> 16) & 0xF;
    uint32_t extended_family = (info[0] >> 20) & 0xFF;
    
    /* Adjust for extended family/model */
    if (family == 0xF) {
        family += extended_family;
        model += (extended_model << 4);
    }
    
    /* Simple heuristic for Xeon MP: Family 15 (Pentium 4 Xeon),
       Model 3 or 4, and stepping > 0 */
    if (family == 15 && (model == 3 || model == 4) && stepping > 0) {
        xeon_mp = 1;
    }
    
    /* Alternative check: CPU brand string might contain "Xeon MP" */
    char brand[49] = {0};
    uint32_t brand_info[12];
    
    /* Get brand string (CPUID leaves 0x80000002-0x80000004) */
    for (int i = 0; i < 3; i++) {
        cpuid(brand_info, 0x80000002 + i, 0);
        memcpy(brand + i * 16, brand_info, 16);
    }
    
    /* Check for "Xeon" and "MP" in brand string */
    if (strstr(brand, "Xeon") && strstr(brand, "MP")) {
        xeon_mp = 1;
    }
}

/* Process a single cache descriptor byte */
static void process_descriptor(uint8_t desc, struct cache_desc *level1, 
                               struct cache_desc *level2) {
    switch (desc) {
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            level1->valid = 1;
            printf("L1 Cache: 8KB, 2-way, 32B line (0x0a)\n");
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            level1->valid = 1;
            printf("L1 Cache: 16KB, 4-way, 32B line (0x0c)\n");
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            level1->valid = 1;
            printf("L1 Cache: 16KB, 4-way, 64B line (0x0d)\n");
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            level1->valid = 1;
            printf("L1 Cache: 24KB, 6-way, 64B line (0x0e)\n");
            break;
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            level2->valid = 1;
            printf("L2 Cache: 256KB, 8-way, 64B line (0x21)\n");
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            level2->valid = 1;
            printf("L2 Cache: 1024KB, 16-way, 64B line (0x24)\n");
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            level1->valid = 1;
            printf("L1 Cache: 32KB, 8-way, 64B line (0x2c)\n");
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            level2->valid = 1;
            printf("L2 Cache: 128KB, 4-way, 64B line (0x39)\n");
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            level2->valid = 1;
            printf("L2 Cache: 192KB, 6-way, 64B line (0x3a)\n");
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            level2->valid = 1;
            printf("L2 Cache: 128KB, 2-way, 64B line (0x3b)\n");
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            level2->valid = 1;
            printf("L2 Cache: 256KB, 4-way, 64B line (0x3c)\n");
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            level2->valid = 1;
            printf("L2 Cache: 384KB, 6-way, 64B line (0x3d)\n");
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            level2->valid = 1;
            printf("L2 Cache: 512KB, 4-way, 64B line (0x3e)\n");
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            level2->valid = 1;
            printf("L2 Cache: 128KB, 4-way, 32B line (0x41)\n");
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            level2->valid = 1;
            printf("L2 Cache: 256KB, 4-way, 32B line (0x42)\n");
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            level2->valid = 1;
            printf("L2 Cache: 512KB, 4-way, 32B line (0x43)\n");
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            level2->valid = 1;
            printf("L2 Cache: 1024KB, 4-way, 32B line (0x44)\n");
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            level2->valid = 1;
            printf("L2 Cache: 2048KB, 4-way, 32B line (0x45)\n");
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            level2->valid = 1;
            printf("L2 Cache: 3072KB, 12-way, 64B line (0x48)\n");
            break;
        case 0x49:
            if (xeon_mp) {
                printf("Case 0x49: Xeon MP detected, skipping L2 cache entry\n");
                break;
            }
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            level2->valid = 1;
            printf("L2 Cache: 4096KB, 16-way, 64B line (0x49)\n");
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            level2->valid = 1;
            printf("L2 Cache: 6144KB, 24-way, 64B line (0x4e)\n");
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            level1->valid = 1;
            printf("L1 Cache: 16KB, 8-way, 64B line (0x60)\n");
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            level1->valid = 1;
            printf("L1 Cache: 8KB, 4-way, 64B line (0x66)\n");
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            level1->valid = 1;
            printf("L1 Cache: 16KB, 4-way, 64B line (0x67)\n");
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            level1->valid = 1;
            printf("L1 Cache: 32KB, 4-way, 64B line (0x68)\n");
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            level2->valid = 1;
            printf("L2 Cache: 1024KB, 4-way, 64B line (0x78)\n");
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            level2->valid = 1;
            printf("L2 Cache: 128KB, 8-way, 64B line (0x79)\n");
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            level2->valid = 1;
            printf("L2 Cache: 256KB, 8-way, 64B line (0x7a)\n");
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            level2->valid = 1;
            printf("L2 Cache: 512KB, 8-way, 64B line (0x7b)\n");
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            level2->valid = 1;
            printf("L2 Cache: 1024KB, 8-way, 64B line (0x7c)\n");
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            level2->valid = 1;
            printf("L2 Cache: 2048KB, 8-way, 64B line (0x7d)\n");
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            level2->valid = 1;
            printf("L2 Cache: 512KB, 2-way, 64B line (0x7f)\n");
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            level2->valid = 1;
            printf("L2 Cache: 512KB, 8-way, 64B line (0x80)\n");
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            level2->valid = 1;
            printf("L2 Cache: 256KB, 8-way, 32B line (0x82)\n");
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            level2->valid = 1;
            printf("L2 Cache: 512KB, 8-way, 32B line (0x83)\n");
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            level2->valid = 1;
            printf("L2 Cache: 1024KB, 8-way, 32B line (0x84)\n");
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            level2->valid = 1;
            printf("L2 Cache: 2048KB, 8-way, 32B line (0x85)\n");
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            level2->valid = 1;
            printf("L2 Cache: 512KB, 4-way, 64B line (0x86)\n");
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            level2->valid = 1;
            printf("L2 Cache: 1024KB, 8-way, 64B line (0x87)\n");
            break;
        default:
            /* Ignore other descriptor values */
            break;
    }
}

/* Extract descriptor bytes from CPUID results */
static void extract_descriptors(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx,
                                struct cache_desc *level1, struct cache_desc *level2) {
    uint8_t *bytes = (uint8_t *)&eax;
    for (int i = 0; i < 4; i++) {
        if (bytes[i] != 0x00 && (bytes[i] & 0x80) == 0) {
            process_descriptor(bytes[i], level1, level2);
        }
    }
    
    bytes = (uint8_t *)&ebx;
    for (int i = 0; i < 4; i++) {
        if (bytes[i] != 0x00 && (bytes[i] & 0x80) == 0) {
            process_descriptor(bytes[i], level1, level2);
        }
    }
    
    bytes = (uint8_t *)&ecx;
    for (int i = 0; i < 4; i++) {
        if (bytes[i] != 0x00 && (bytes[i] & 0x80) == 0) {
            process_descriptor(bytes[i], level1, level2);
        }
    }
    
    bytes = (uint8_t *)&edx;
    for (int i = 0; i < 4; i++) {
        if (bytes[i] != 0x00 && (bytes[i] & 0x80) == 0) {
            process_descriptor(bytes[i], level1, level2);
        }
    }
}

/* Perform cache-aware computation using detected cache line size */
static void cache_aware_computation(struct cache_desc *level1, struct cache_desc *level2) {
    int line_size = 64; /* Default if not detected */
    if (level1->valid && level1->line > 0) {
        line_size = level1->line;
    } else if (level2->valid && level2->line > 0) {
        line_size = level2->line;
    }
    
    /* Allocate array with cache line alignment */
    int array_size = 1024 * 1024; /* 1MB */
    char *buffer = (char *)aligned_alloc(line_size, array_size);
    if (!buffer) {
        return;
    }
    
    /* Perform strided access pattern that respects cache lines */
    volatile int sum = 0;
    for (int i = 0; i < array_size; i += line_size) {
        buffer[i] = (char)(i % 256);
        sum += buffer[i];
    }
    
    /* Use sum to prevent optimization */
    printf("Cache-aware computation result: %d (using %d byte line size)\n", sum, line_size);
    
    free(buffer);
}

int main(void) {
    struct cache_desc level1 = {0, 0, 0, 0};
    struct cache_desc level2 = {0, 0, 0, 0};
    
    printf("Starting CPUID-based cache detection...\n");
    
    /* Step 1: Detect Xeon MP status */
    detect_xeon_mp();
    printf("Xeon MP detection: %s\n", xeon_mp ? "YES" : "NO");
    
    /* Step 2: Call CPUID leaf 0x2 multiple times */
    uint32_t info[4];
    int iterations = 0;
    
    do {
        cpuid(info, 0x2, 0);
        
        /* Check if EAX indicates valid descriptors */
        if ((info[0] & 0xFF) == 0) {
            break; /* No more descriptors */
        }
        
        printf("CPUID leaf 0x2, iteration %d: EAX=%08X EBX=%08X ECX=%08X EDX=%08X\n",
               iterations, info[0], info[1], info[2], info[3]);
        
        /* Extract and process descriptor bytes */
        extract_descriptors(info[0], info[1], info[2], info[3], &level1, &level2);
        
        iterations++;
        
        /* According to Intel spec, we may need to call CPUID(2) multiple times
           until we get a descriptor with all bits in AL = 0 */
    } while (iterations < 10 && (info[0] & 0xFF) != 0);
    
    /* Step 3: Print final cache information */
    printf("\nFinal Cache Configuration:\n");
    if (level1.valid) {
        printf("L1 Cache: %dKB, %d-way, %dB line\n", 
               level1.sizekb, level1.assoc, level1.line);
    }
    if (level2.valid) {
        printf("L2 Cache: %dKB, %d-way, %dB line\n", 
               level2.sizekb, level2.assoc, level2.line);
    }
    
    /* Step 4: Perform cache-aware computation */
    cache_aware_computation(&level1, &level2);
    
    return 0;
}
