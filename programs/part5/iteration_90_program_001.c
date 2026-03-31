/*
 * This program triggers Intel CPU cache detection logic by calling CPUID leaf 0x2
 * and processing cache descriptor bytes matching the uncovered cases from driver-i386.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Cross-platform CPUID intrinsics */
#if defined(_WIN32) || defined(_WIN64)
#include <intrin.h>
#else
#include <cpuid.h>
#endif

/* Cache descriptor structure matching driver-i386.cc logic */
struct cache_desc {
    int sizekb;    /* Size in KB */
    int assoc;     /* Associativity */
    int line;      /* Line size in bytes */
    int valid;     /* Whether this entry is valid */
    int level;     /* Cache level (1 or 2) */
    uint8_t descriptor; /* Original descriptor byte */
};

/* Function to execute CPUID instruction */
static void cpuid(uint32_t leaf, uint32_t subleaf, 
                  uint32_t* eax, uint32_t* ebx, 
                  uint32_t* ecx, uint32_t* edx) {
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

/* Check if CPU is Xeon MP (for case 0x49) */
static int is_xeon_mp(void) {
    uint32_t eax, ebx, ecx, edx;
    
    /* Get CPU signature from leaf 0x1 */
    cpuid(0x1, 0, &eax, &ebx, &ecx, &edx);
    
    /* Extract family, model, stepping */
    uint32_t family = ((eax >> 8) & 0xF) | ((eax >> 20) & 0xFF);
    uint32_t model = ((eax >> 4) & 0xF) | ((eax >> 12) & 0xF0);
    uint32_t stepping = eax & 0xF;
    
    /* Check extended model and family for Xeon MP */
    /* This is a simplified check - real detection would be more complex */
    if (family == 0xF && model >= 0x6) {
        /* Check if it's a Xeon by brand string (simplified) */
        char brand[49] = {0};
        uint32_t brand_regs[12];
        
        /* Get brand string */
        cpuid(0x80000002, 0, &brand_regs[0], &brand_regs[1], 
              &brand_regs[2], &brand_regs[3]);
        cpuid(0x80000003, 0, &brand_regs[4], &brand_regs[5], 
              &brand_regs[6], &brand_regs[7]);
        cpuid(0x80000004, 0, &brand_regs[8], &brand_regs[9], 
              &brand_regs[10], &brand_regs[11]);
        
        memcpy(brand, brand_regs, 48);
        
        /* Check for "Xeon" and "MP" in brand string */
        if (strstr(brand, "Xeon") && strstr(brand, "MP")) {
            return 1;
        }
    }
    
    return 0;
}

/* Process a single cache descriptor byte */
static void process_descriptor(uint8_t desc, struct cache_desc* level1, 
                               struct cache_desc* level2, int xeon_mp) {
    /* Only process valid cache descriptors (high bit clear) */
    if (desc & 0x80) {
        /* This is a TLB descriptor, skip */
        return;
    }
    
    switch (desc) {
        /* L1 Data Cache cases */
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            level1->valid = 1; level1->level = 1; level1->descriptor = desc;
            printf("  Found L1 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level1->sizekb, level1->assoc, level1->line);
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            level1->valid = 1; level1->level = 1; level1->descriptor = desc;
            printf("  Found L1 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level1->sizekb, level1->assoc, level1->line);
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            level1->valid = 1; level1->level = 1; level1->descriptor = desc;
            printf("  Found L1 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level1->sizekb, level1->assoc, level1->line);
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            level1->valid = 1; level1->level = 1; level1->descriptor = desc;
            printf("  Found L1 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level1->sizekb, level1->assoc, level1->line);
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            level1->valid = 1; level1->level = 1; level1->descriptor = desc;
            printf("  Found L1 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level1->sizekb, level1->assoc, level1->line);
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            level1->valid = 1; level1->level = 1; level1->descriptor = desc;
            printf("  Found L1 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level1->sizekb, level1->assoc, level1->line);
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            level1->valid = 1; level1->level = 1; level1->descriptor = desc;
            printf("  Found L1 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level1->sizekb, level1->assoc, level1->line);
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            level1->valid = 1; level1->level = 1; level1->descriptor = desc;
            printf("  Found L1 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level1->sizekb, level1->assoc, level1->line);
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            level1->valid = 1; level1->level = 1; level1->descriptor = desc;
            printf("  Found L1 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level1->sizekb, level1->assoc, level1->line);
            break;
            
        /* L2 Cache cases */
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x49:
            if (xeon_mp) {
                printf("  Descriptor 0x49: Xeon MP detected, skipping L2 cache entry\n");
                break;
            }
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            level2->valid = 1; level2->level = 2; level2->descriptor = desc;
            printf("  Found L2 cache: descriptor 0x%02x -> %dKB, %d-way, %d-byte line\n",
                   desc, level2->sizekb, level2->assoc, level2->line);
            break;
            
        default:
            /* Other descriptors not in our target list */
            if (desc != 0x00) {
                printf("  Skipping descriptor 0x%02x (not in target list)\n", desc);
            }
            break;
    }
}

/* Extract descriptor bytes from CPUID results */
static void extract_descriptors(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx,
                                uint8_t* descriptors, int* count) {
    int i = 0;
    
    /* EAX byte 0 is the number of times to call CPUID leaf 2 */
    /* Bytes 1-3 are descriptor bytes */
    if ((eax & 0xFF) != 0) {
        descriptors[i++] = (eax >> 8) & 0xFF;
        descriptors[i++] = (eax >> 16) & 0xFF;
        descriptors[i++] = (eax >> 24) & 0xFF;
    }
    
    /* EBX bytes 0-3 are descriptor bytes */
    descriptors[i++] = ebx & 0xFF;
    descriptors[i++] = (ebx >> 8) & 0xFF;
    descriptors[i++] = (ebx >> 16) & 0xFF;
    descriptors[i++] = (ebx >> 24) & 0xFF;
    
    /* ECX bytes 0-3 are descriptor bytes */
    descriptors[i++] = ecx & 0xFF;
    descriptors[i++] = (ecx >> 8) & 0xFF;
    descriptors[i++] = (ecx >> 16) & 0xFF;
    descriptors[i++] = (ecx >> 24) & 0xFF;
    
    /* EDX bytes 0-3 are descriptor bytes */
    descriptors[i++] = edx & 0xFF;
    descriptors[i++] = (edx >> 8) & 0xFF;
    descriptors[i++] = (edx >> 16) & 0xFF;
    descriptors[i++] = (edx >> 24) & 0xFF;
    
    *count = i;
}

/* Perform cache-aware computation using detected cache line size */
static void cache_aware_computation(struct cache_desc* level1, struct cache_desc* level2) {
    int line_size = 64; /* Default cache line size */
    
    /* Use detected cache line size if available */
    if (level1->valid && level1->line > 0) {
        line_size = level1->line;
    } else if (level2->valid && level2->line > 0) {
        line_size = level2->line;
    }
    
    printf("\nPerforming cache-aware computation with line size %d bytes:\n", line_size);
    
    /* Allocate aligned memory */
    size_t array_size = 1024 * 1024; /* 1MB */
    char* data = (char*)aligned_alloc(line_size, array_size);
    
    if (!data) {
        printf("  Memory allocation failed\n");
        return;
    }
    
    /* Initialize data */
    for (size_t i = 0; i < array_size; i++) {
        data[i] = (char)(i % 256);
    }
    
    /* Perform computation with cache line alignment */
    volatile int sum = 0;
    for (size_t i = 0; i < array_size; i += line_size) {
        sum += data[i];
    }
    
    /* Use sum to prevent optimization */
    printf("  Computed sum: %d (prevents optimization removal)\n", sum);
    
    free(data);
}

int main(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptors[64];
    int desc_count;
    int iterations;
    int i, j;
    
    /* Initialize cache descriptor structures */
    struct cache_desc level1 = {0, 0, 0, 0, 1, 0};
    struct cache_desc level2 = {0, 0, 0, 0, 2, 0};
    
    printf("Intel CPU Cache Detection Program\n");
    printf("=================================\n\n");
    
    /* Step 1: Check if CPU supports CPUID leaf 0x2 */
    cpuid(0x0, 0, &eax, &ebx, &ecx, &edx);
    uint32_t max_leaf = eax;
    
    if (max_leaf < 0x2) {
        printf("CPU does not support cache detection (CPUID leaf 0x2)\n");
        return 1;
    }
    
    printf("CPU supports CPUID leaf 0x2 (max leaf: 0x%x)\n", max_leaf);
    
    /* Step 2: Check Xeon MP status for case 0x49 */
    int xeon_mp = is_xeon_mp();
    printf("Xeon MP detection: %s\n\n", xeon_mp ? "YES" : "NO");
    
    /* Step 3: Call CPUID leaf 0x2 multiple times as needed */
    printf("Calling CPUID leaf 0x2 for cache descriptors:\n");
    
    cpuid(0x2, 0, &eax, &ebx, &ecx, &edx);
    
    /* Get number of iterations needed (EAX byte 0) */
    iterations = eax & 0xFF;
    printf("  Need to call CPUID leaf 0x2 %d time(s)\n", iterations);
    
    /* Process first call results */
    extract_descriptors(eax, ebx, ecx, edx, descriptors, &desc_count);
    
    /* Make additional calls if needed */
    for (i = 1; i < iterations; i++) {
        uint8_t more_descriptors[16];
        int more_count;
        
        cpuid(0x2, 0, &eax, &ebx, &ecx, &edx);
        extract_descriptors(eax, ebx, ecx, edx, more_descriptors, &more_count);
        
        /* Append to main descriptor array */
        for (j = 0; j < more_count && desc_count < 64; j++) {
            descriptors[desc_count++] = more_descriptors[j];
        }
    }
    
    /* Step 4: Process all descriptor bytes */
    printf("\nProcessing %d descriptor bytes:\n", desc_count);
    
    for (i = 0; i < desc_count; i++) {
        uint8_t desc = descriptors[i];
        
        /* Stop at terminator byte */
        if (desc == 0x00) {
            printf("  Found terminator byte 0x00, stopping\n");
            break;
        }
        
        /* Process the descriptor */
        process_descriptor(desc, &level1, &level2, xeon_mp);
    }
    
    /* Step 5: Print final cache information */
    printf("\nFinal Cache Configuration:\n");
    printf("--------------------------\n");
    
    if (level1.valid) {
        printf("L1 Cache: %d KB, %d-way associative, %d-byte line (descriptor 0x%02x)\n",
               level1.sizekb, level1.assoc, level1.line, level1.descriptor);
    } else {
        printf("L1 Cache: Not detected in target descriptors\n");
    }
    
    if (level2.valid) {
        printf("L2 Cache: %d KB, %d-way associative, %d-byte line (descriptor 0x%02x)\n",
               level2.sizekb, level2.assoc, level2.line, level2.descriptor);
    } else {
        printf("L2 Cache: Not detected in target descriptors\n");
    }
    
    /* Step 6: Perform cache-aware computation */
    cache_aware_computation(&level1, &level2);
    
    printf("\nProgram completed successfully\n");
    return 0;
}
