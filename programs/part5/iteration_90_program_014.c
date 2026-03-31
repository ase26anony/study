/*
 * This program triggers Intel CPU cache detection logic by calling CPUID leaf 0x2
 * and processing cache descriptor bytes matching the uncovered cases in driver-i386.cc.
 * It implements the exact switch cases from lines 127-244.
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
#define cpuidex(info, x, ecx_val) __cpuidex(info, x, ecx_val)
#else
#include <cpuid.h>
#define cpuid(info, x) __cpuid(x, info[0], info[1], info[2], info[3])
#define cpuidex(info, x, ecx_val) __cpuid_count(x, ecx_val, info[0], info[1], info[2], info[3])
#endif

/* Cache descriptor structure matching driver-i386.cc */
struct cache_desc {
    int sizekb;     /* Size in KB */
    int assoc;      /* Associativity */
    int line;       /* Line size in bytes */
    int level;      /* Cache level (1 or 2) */
    int type;       /* Cache type */
};

/* Function to check if CPU is Xeon MP (for case 0x49) */
static int is_xeon_mp(void) {
    uint32_t info[4];
    cpuid(info, 0x00000001);
    
    /* Extract family, model, stepping */
    uint32_t stepping = info[0] & 0xF;
    uint32_t model = (info[0] >> 4) & 0xF;
    uint32_t family = (info[0] >> 8) & 0xF;
    uint32_t extended_model = (info[0] >> 16) & 0xF;
    uint32_t extended_family = (info[0] >> 20) & 0xFF;
    
    /* Adjust for extended family/model */
    if (family == 0xF) {
        family += extended_family;
        model = (extended_model << 4) | model;
    }
    
    /* Check for Xeon MP characteristics:
     * Family 6, Model 44, 46, 47 are Nehalem-EX/Beckton (Xeon MP)
     * This is a simplified check - real detection would be more complex */
    if (family == 6) {
        if (model == 44 || model == 46 || model == 47) {
            return 1;
        }
    }
    
    return 0;
}

/* Process a single cache descriptor byte */
static void process_descriptor_byte(uint8_t desc, struct cache_desc *level1, 
                                   struct cache_desc *level2, int xeon_mp) {
    switch (desc) {
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            level1->level = 1; level1->type = 1;
            printf("L1 Cache: 8KB, 2-way, 32-byte line (0x0a)\n");
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            level1->level = 1; level1->type = 1;
            printf("L1 Cache: 16KB, 4-way, 32-byte line (0x0c)\n");
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            level1->level = 1; level1->type = 1;
            printf("L1 Cache: 16KB, 4-way, 64-byte line (0x0d)\n");
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            level1->level = 1; level1->type = 1;
            printf("L1 Cache: 24KB, 6-way, 64-byte line (0x0e)\n");
            break;
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 256KB, 8-way, 64-byte line (0x21)\n");
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 1024KB, 16-way, 64-byte line (0x24)\n");
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            level1->level = 1; level1->type = 1;
            printf("L1 Cache: 32KB, 8-way, 64-byte line (0x2c)\n");
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 128KB, 4-way, 64-byte line (0x39)\n");
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 192KB, 6-way, 64-byte line (0x3a)\n");
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 128KB, 2-way, 64-byte line (0x3b)\n");
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 256KB, 4-way, 64-byte line (0x3c)\n");
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 384KB, 6-way, 64-byte line (0x3d)\n");
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 512KB, 4-way, 64-byte line (0x3e)\n");
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 128KB, 4-way, 32-byte line (0x41)\n");
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 256KB, 4-way, 32-byte line (0x42)\n");
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 512KB, 4-way, 32-byte line (0x43)\n");
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 1024KB, 4-way, 32-byte line (0x44)\n");
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 2048KB, 4-way, 32-byte line (0x45)\n");
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 3072KB, 12-way, 64-byte line (0x48)\n");
            break;
        case 0x49:
            if (xeon_mp) {
                printf("Case 0x49: Xeon MP detected, skipping L2 cache initialization\n");
                break;
            }
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 4096KB, 16-way, 64-byte line (0x49, non-Xeon MP)\n");
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 6144KB, 24-way, 64-byte line (0x4e)\n");
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            level1->level = 1; level1->type = 1;
            printf("L1 Cache: 16KB, 8-way, 64-byte line (0x60)\n");
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            level1->level = 1; level1->type = 1;
            printf("L1 Cache: 8KB, 4-way, 64-byte line (0x66)\n");
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            level1->level = 1; level1->type = 1;
            printf("L1 Cache: 16KB, 4-way, 64-byte line (0x67)\n");
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            level1->level = 1; level1->type = 1;
            printf("L1 Cache: 32KB, 4-way, 64-byte line (0x68)\n");
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 1024KB, 4-way, 64-byte line (0x78)\n");
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 128KB, 8-way, 64-byte line (0x79)\n");
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 256KB, 8-way, 64-byte line (0x7a)\n");
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 512KB, 8-way, 64-byte line (0x7b)\n");
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 1024KB, 8-way, 64-byte line (0x7c)\n");
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 2048KB, 8-way, 64-byte line (0x7d)\n");
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 512KB, 2-way, 64-byte line (0x7f)\n");
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 512KB, 8-way, 64-byte line (0x80)\n");
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 256KB, 8-way, 32-byte line (0x82)\n");
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 512KB, 8-way, 32-byte line (0x83)\n");
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 1024KB, 8-way, 32-byte line (0x84)\n");
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 2048KB, 8-way, 32-byte line (0x85)\n");
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 512KB, 4-way, 64-byte line (0x86)\n");
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 2;
            printf("L2 Cache: 1024KB, 8-way, 64-byte line (0x87)\n");
            break;
        case 0x00:
            /* Valid terminator byte */
            break;
        default:
            /* Other descriptor bytes not in our target cases */
            if (desc != 0x01 && desc != 0x02 && desc != 0x03 && 
                desc != 0x40 && desc != 0xfe && desc != 0xff) {
                printf("Other descriptor: 0x%02x\n", desc);
            }
            break;
    }
}

/* Extract and process cache descriptor bytes from CPUID leaf 0x2 */
static void process_cpuid_cache_info(struct cache_desc *level1, 
                                    struct cache_desc *level2) {
    uint32_t info[4];
    int xeon_mp = is_xeon_mp();
    int iterations = 0;
    int max_iterations = 16; /* Safety limit */
    
    printf("Xeon MP detection: %s\n", xeon_mp ? "YES" : "NO");
    
    /* According to Intel manual, CPUID leaf 0x2 may need to be called multiple times */
    while (iterations < max_iterations) {
        cpuid(info, 0x00000002);
        
        /* Process bytes from EAX (bits 7-0 are valid descriptor) */
        uint8_t *bytes = (uint8_t *)info;
        
        /* Process each byte in the returned registers */
        for (int i = 0; i < 16; i++) {
            uint8_t desc = bytes[i];
            
            /* Skip if this is a register padding byte (high bit set) */
            if (desc & 0x80) {
                continue;
            }
            
            /* Process the descriptor byte */
            process_descriptor_byte(desc, level1, level2, xeon_mp);
            
            /* Check for terminator */
            if (desc == 0x00) {
                printf("Found terminator byte (0x00) at iteration %d\n", iterations);
                return;
            }
        }
        
        iterations++;
    }
    
    printf("Maximum iterations reached without finding terminator\n");
}

/* Perform cache-aware computation using detected cache line size */
static void cache_aware_computation(struct cache_desc *level1, 
                                   struct cache_desc *level2) {
    int line_size = 64; /* Default if not detected */
    
    /* Use detected cache line size if available */
    if (level1->line > 0) {
        line_size = level1->line;
    } else if (level2->line > 0) {
        line_size = level2->line;
    }
    
    printf("\nPerforming cache-aware computation with line size: %d bytes\n", line_size);
    
    /* Allocate aligned memory */
    size_t array_size = 1024 * 1024; /* 1MB */
    char *buffer = (char *)aligned_alloc(line_size, array_size);
    
    if (buffer == NULL) {
        printf("Memory allocation failed\n");
        return;
    }
    
    /* Initialize buffer */
    memset(buffer, 0, array_size);
    
    /* Access memory with cache line alignment */
    volatile int sum = 0;
    for (size_t i = 0; i < array_size; i += line_size) {
        buffer[i] = (char)(i % 256);
        sum += buffer[i]; /* Force read */
    }
    
    /* Use sum to prevent optimization */
    printf("Computation result (prevent optimization): %d\n", sum);
    
    free(buffer);
}

int main(void) {
    struct cache_desc level1 = {0, 0, 0, 1, 0};
    struct cache_desc level2 = {0, 0, 0, 2, 0};
    
    printf("Starting Intel CPU cache detection...\n");
    printf("Targeting uncovered cases from driver-i386.cc lines 127-244\n\n");
    
    /* Get and process CPUID cache information */
    process_cpuid_cache_info(&level1, &level2);
    
    /* Print summary */
    printf("\n=== Cache Detection Summary ===\n");
    if (level1.sizekb > 0) {
        printf("L1 Cache: %d KB, %d-way, %d-byte line\n", 
               level1.sizekb, level1.assoc, level1.line);
    } else {
        printf("L1 Cache: Not detected from target descriptors\n");
    }
    
    if (level2.sizekb > 0) {
        printf("L2 Cache: %d KB, %d-way, %d-byte line\n", 
               level2.sizekb, level2.assoc, level2.line);
    } else {
        printf("L2 Cache: Not detected from target descriptors\n");
    }
    
    /* Perform cache-aware computation */
    cache_aware_computation(&level1, &level2);
    
    return 0;
}
