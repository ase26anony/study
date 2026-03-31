/*
 * This program is designed to trigger the specific CPUID cache detection logic
 * in driver-i386.cc lines 127-244. It calls CPUID leaf 0x2 to retrieve cache
 * descriptor bytes and processes them according to Intel's specification,
 * implementing the exact case statements from the uncovered code block.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Cross-platform CPUID intrinsic wrappers */
#if defined(_WIN32) || defined(_WIN64)
#include <intrin.h>
#include <windows.h>
#else
#include <cpuid.h>
#endif

/* Cache descriptor structure matching driver-i386.cc */
struct cache_desc {
    int sizekb;     /* Size in KB */
    int assoc;      /* Associativity */
    int line;       /* Line size in bytes */
};

/* CPUID wrapper function */
static void cpuid(uint32_t leaf, uint32_t subleaf,
                  uint32_t *eax, uint32_t *ebx,
                  uint32_t *ecx, uint32_t *edx) {
#if defined(_WIN32) || defined(_WIN64)
    int regs[4];
    __cpuidex(regs, leaf, subleaf);
    *eax = regs[0];
    *ebx = regs[1];
    *ecx = regs[2];
    *edx = regs[3];
#else
    __cpuid_count(leaf, subleaf, *eax, *ebx, *ecx, *edx);
#endif
}

/* Extract cache descriptor bytes from CPUID leaf 0x2 results */
static void process_cache_descriptors(uint32_t eax, uint32_t ebx,
                                      uint32_t ecx, uint32_t edx,
                                      uint8_t *descriptors, int *count) {
    uint8_t *bytes = (uint8_t *)&eax;
    for (int i = 0; i < 4; i++) {
        if (bytes[i] != 0x00 && bytes[i] != 0xFF) {
            descriptors[(*count)++] = bytes[i];
        }
    }
    
    bytes = (uint8_t *)&ebx;
    for (int i = 0; i < 4; i++) {
        if (bytes[i] != 0x00 && bytes[i] != 0xFF) {
            descriptors[(*count)++] = bytes[i];
        }
    }
    
    bytes = (uint8_t *)&ecx;
    for (int i = 0; i < 4; i++) {
        if (bytes[i] != 0x00 && bytes[i] != 0xFF) {
            descriptors[(*count)++] = bytes[i];
        }
    }
    
    bytes = (uint8_t *)&edx;
    for (int i = 0; i < 4; i++) {
        if (bytes[i] != 0x00 && bytes[i] != 0xFF) {
            descriptors[(*count)++] = bytes[i];
        }
    }
}

/* Determine if CPU is Xeon MP based on CPUID leaf 0x1 */
static int is_xeon_mp(void) {
    uint32_t eax, ebx, ecx, edx;
    cpuid(1, 0, &eax, &ebx, &ecx, &edx);
    
    /* Extract family, model, stepping */
    uint32_t family = (eax >> 8) & 0xF;
    uint32_t model = (eax >> 4) & 0xF;
    uint32_t extended_family = (eax >> 20) & 0xFF;
    uint32_t extended_model = (eax >> 16) & 0xF;
    
    if (family == 0xF) {
        family = extended_family + family;
        model = (extended_model << 4) + model;
    }
    
    /* Xeon MP detection heuristic */
    /* This mimics the logic in driver-i386.cc */
    if (family == 0xF && model >= 0x6) {
        /* Check for Xeon MP characteristics */
        uint32_t brand = ebx & 0xFF;
        if (brand == 0x0 || brand == 0x1) {
            /* Likely Xeon MP */
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
            printf("L1 Cache: 8KB, 2-way, 32-byte line (0x%02x)\n", desc);
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            printf("L1 Cache: 16KB, 4-way, 32-byte line (0x%02x)\n", desc);
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("L1 Cache: 16KB, 4-way, 64-byte line (0x%02x)\n", desc);
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            printf("L1 Cache: 24KB, 6-way, 64-byte line (0x%02x)\n", desc);
            break;
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 256KB, 8-way, 64-byte line (0x%02x)\n", desc);
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            printf("L2 Cache: 1024KB, 16-way, 64-byte line (0x%02x)\n", desc);
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            printf("L1 Cache: 32KB, 8-way, 64-byte line (0x%02x)\n", desc);
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 128KB, 4-way, 64-byte line (0x%02x)\n", desc);
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            printf("L2 Cache: 192KB, 6-way, 64-byte line (0x%02x)\n", desc);
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            printf("L2 Cache: 128KB, 2-way, 64-byte line (0x%02x)\n", desc);
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 256KB, 4-way, 64-byte line (0x%02x)\n", desc);
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            printf("L2 Cache: 384KB, 6-way, 64-byte line (0x%02x)\n", desc);
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 512KB, 4-way, 64-byte line (0x%02x)\n", desc);
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 128KB, 4-way, 32-byte line (0x%02x)\n", desc);
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 256KB, 4-way, 32-byte line (0x%02x)\n", desc);
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 512KB, 4-way, 32-byte line (0x%02x)\n", desc);
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 1024KB, 4-way, 32-byte line (0x%02x)\n", desc);
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 2048KB, 4-way, 32-byte line (0x%02x)\n", desc);
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            printf("L2 Cache: 3072KB, 12-way, 64-byte line (0x%02x)\n", desc);
            break;
        case 0x49:
            if (xeon_mp) {
                printf("Descriptor 0x49: Xeon MP detected, skipping L2 cache\n");
                break;
            }
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            printf("L2 Cache: 4096KB, 16-way, 64-byte line (0x%02x)\n", desc);
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            printf("L2 Cache: 6144KB, 24-way, 64-byte line (0x%02x)\n", desc);
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            printf("L1 Cache: 16KB, 8-way, 64-byte line (0x%02x)\n", desc);
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            printf("L1 Cache: 8KB, 4-way, 64-byte line (0x%02x)\n", desc);
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("L1 Cache: 16KB, 4-way, 64-byte line (0x%02x)\n", desc);
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            printf("L1 Cache: 32KB, 4-way, 64-byte line (0x%02x)\n", desc);
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 1024KB, 4-way, 64-byte line (0x%02x)\n", desc);
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 128KB, 8-way, 64-byte line (0x%02x)\n", desc);
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 256KB, 8-way, 64-byte line (0x%02x)\n", desc);
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 512KB, 8-way, 64-byte line (0x%02x)\n", desc);
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 1024KB, 8-way, 64-byte line (0x%02x)\n", desc);
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 2048KB, 8-way, 64-byte line (0x%02x)\n", desc);
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            printf("L2 Cache: 512KB, 2-way, 64-byte line (0x%02x)\n", desc);
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 512KB, 8-way, 64-byte line (0x%02x)\n", desc);
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            printf("L2 Cache: 256KB, 8-way, 32-byte line (0x%02x)\n", desc);
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            printf("L2 Cache: 512KB, 8-way, 32-byte line (0x%02x)\n", desc);
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            printf("L2 Cache: 1024KB, 8-way, 32-byte line (0x%02x)\n", desc);
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            printf("L2 Cache: 2048KB, 8-way, 32-byte line (0x%02x)\n", desc);
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 512KB, 4-way, 64-byte line (0x%02x)\n", desc);
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 1024KB, 8-way, 64-byte line (0x%02x)\n", desc);
            break;
        default:
            /* Ignore other descriptors */
            break;
    }
}

/* Perform cache-aware computation using detected cache line size */
static void cache_aware_computation(int cache_line_size) {
    if (cache_line_size <= 0) {
        cache_line_size = 64; /* Default */
    }
    
    /* Align array to cache line boundary */
    const int array_size = 1024 * 1024; /* 1MB */
    char *buffer = (char*)aligned_alloc(cache_line_size, array_size);
    if (!buffer) {
        return;
    }
    
    /* Initialize buffer */
    memset(buffer, 0, array_size);
    
    /* Access memory with cache line awareness */
    volatile int sum = 0;
    for (int i = 0; i < array_size; i += cache_line_size) {
        buffer[i] = (char)(i % 256);
        sum += buffer[i];
    }
    
    /* Prevent optimization */
    printf("Cache-aware computation result: %d (using %d-byte lines)\n", 
           sum, cache_line_size);
    
    free(buffer);
}

int main(void) {
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    uint8_t descriptors[256];
    int desc_count = 0;
    int iterations = 0;
    int xeon_mp = 0;
    
    printf("Starting CPUID cache detection...\n");
    
    /* Step 1: Check for Xeon MP */
    xeon_mp = is_xeon_mp();
    printf("Xeon MP detection: %s\n", xeon_mp ? "YES" : "NO");
    
    /* Step 2: Call CPUID leaf 0x2 multiple times */
    while (iterations < 16) { /* Safety limit */
        uint32_t eax, ebx, ecx, edx;
        cpuid(2, iterations, &eax, &ebx, &ecx, &edx);
        
        /* Check if all bytes are zero (termination condition) */
        if (eax == 0 && ebx == 0 && ecx == 0 && edx == 0) {
            break;
        }
        
        /* Process the descriptor bytes */
        process_cache_descriptors(eax, ebx, ecx, edx, descriptors, &desc_count);
        
        /* Check for terminator byte in the descriptors */
        int found_terminator = 0;
        for (int i = 0; i < desc_count; i++) {
            if (descriptors[i] == 0x00) {
                found_terminator = 1;
                break;
            }
        }
        
        if (found_terminator) {
            break;
        }
        
        iterations++;
    }
    
    printf("Found %d cache descriptor bytes\n", desc_count);
    
    /* Step 3: Process all collected descriptors */
    for (int i = 0; i < desc_count; i++) {
        if (descriptors[i] != 0x00 && descriptors[i] != 0xFF) {
            process_descriptor_byte(descriptors[i], &level1, &level2, xeon_mp);
        }
    }
    
    /* Step 4: Print final cache information */
    printf("\nFinal Cache Configuration:\n");
    if (level1.sizekb > 0) {
        printf("L1 Cache: %dKB, %d-way, %d-byte line\n",
               level1.sizekb, level1.assoc, level1.line);
    }
    if (level2.sizekb > 0) {
        printf("L2 Cache: %dKB, %d-way, %d-byte line\n",
               level2.sizekb, level2.assoc, level2.line);
    }
    
    /* Step 5: Perform cache-aware computation */
    int cache_line_size = level1.line > 0 ? level1.line : 
                         (level2.line > 0 ? level2.line : 64);
    cache_aware_computation(cache_line_size);
    
    return 0;
}
