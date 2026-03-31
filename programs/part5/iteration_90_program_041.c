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
#ifdef _WIN32
#include <intrin.h>
#else
#include <cpuid.h>
#endif

/* Cache descriptor structure matching driver-i386.cc */
struct cache_desc {
    int sizekb;    /* Size in KB */
    int assoc;     /* Associativity */
    int line;      /* Line size in bytes */
};

/* CPUID wrapper function */
static void cpuid(uint32_t leaf, uint32_t subleaf,
                  uint32_t *eax, uint32_t *ebx,
                  uint32_t *ecx, uint32_t *edx) {
#ifdef _WIN32
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

/* Check if CPU is Xeon MP (simplified detection) */
static int is_xeon_mp(void) {
    uint32_t eax, ebx, ecx, edx;
    
    /* Get CPU family/model/stepping from leaf 0x1 */
    cpuid(0x1, 0, &eax, &ebx, &ecx, &edx);
    
    /* Extract family, model, stepping */
    uint32_t family = ((eax >> 8) & 0xF) | ((eax >> 16) & 0xFF0);
    uint32_t model = ((eax >> 4) & 0xF) | ((eax >> 12) & 0xF0);
    uint32_t stepping = eax & 0xF;
    
    /* Simplified Xeon MP detection:
     * - Family 0xF (Pentium 4/Xeon)
     * - Model 0x6 (Xeon MP Gallatin) or similar
     * This is a simplified check; real detection would be more complex
     */
    if (family == 0xF && model >= 0x6) {
        /* Check if it's a multi-processor capable Xeon */
        uint32_t brand = ebx;
        char brand_str[13];
        memcpy(brand_str, &brand, 4);
        memcpy(brand_str + 4, &edx, 4);
        memcpy(brand_str + 8, &ecx, 4);
        brand_str[12] = '\0';
        
        /* Look for "Xeon" in brand string */
        if (strstr(brand_str, "Xeon") != NULL) {
            /* Additional check for MP capability */
            cpuid(0x0, 0, &eax, &ebx, &ecx, &edx);
            if (eax >= 4) {
                cpuid(0x4, 0, &eax, &ebx, &ecx, &edx);
                /* Check if it's a multi-core processor */
                if (((eax >> 26) & 0x3F) > 1) {
                    return 1;
                }
            }
        }
    }
    
    return 0;
}

/* Process cache descriptor byte */
static void process_cache_descriptor(uint8_t desc, struct cache_desc *level1,
                                     struct cache_desc *level2, int xeon_mp) {
    switch (desc) {
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            printf("L1 Cache: 8KB, 2-way, 32B line\n");
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            printf("L1 Cache: 16KB, 4-way, 32B line\n");
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("L1 Cache: 16KB, 4-way, 64B line\n");
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            printf("L1 Cache: 24KB, 6-way, 64B line\n");
            break;
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 256KB, 8-way, 64B line\n");
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            printf("L2 Cache: 1024KB, 16-way, 64B line\n");
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            printf("L1 Cache: 32KB, 8-way, 64B line\n");
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 128KB, 4-way, 64B line\n");
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            printf("L2 Cache: 192KB, 6-way, 64B line\n");
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            printf("L2 Cache: 128KB, 2-way, 64B line\n");
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 256KB, 4-way, 64B line\n");
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            printf("L2 Cache: 384KB, 6-way, 64B line\n");
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 512KB, 4-way, 64B line\n");
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 128KB, 4-way, 32B line\n");
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 256KB, 4-way, 32B line\n");
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 512KB, 4-way, 32B line\n");
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 1024KB, 4-way, 32B line\n");
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 2048KB, 4-way, 32B line\n");
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            printf("L2 Cache: 3072KB, 12-way, 64B line\n");
            break;
        case 0x49:
            if (xeon_mp) {
                printf("Case 0x49: Xeon MP detected, skipping L2 cache setup\n");
                break;
            }
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            printf("L2 Cache: 4096KB, 16-way, 64B line (non-Xeon MP)\n");
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            printf("L2 Cache: 6144KB, 24-way, 64B line\n");
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            printf("L1 Cache: 16KB, 8-way, 64B line\n");
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            printf("L1 Cache: 8KB, 4-way, 64B line\n");
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("L1 Cache: 16KB, 4-way, 64B line\n");
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            printf("L1 Cache: 32KB, 4-way, 64B line\n");
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 1024KB, 4-way, 64B line\n");
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 128KB, 8-way, 64B line\n");
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 256KB, 8-way, 64B line\n");
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 512KB, 8-way, 64B line\n");
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 1024KB, 8-way, 64B line\n");
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 2048KB, 8-way, 64B line\n");
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            printf("L2 Cache: 512KB, 2-way, 64B line\n");
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 512KB, 8-way, 64B line\n");
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            printf("L2 Cache: 256KB, 8-way, 32B line\n");
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            printf("L2 Cache: 512KB, 8-way, 32B line\n");
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            printf("L2 Cache: 1024KB, 8-way, 32B line\n");
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            printf("L2 Cache: 2048KB, 8-way, 32B line\n");
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 512KB, 4-way, 64B line\n");
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 1024KB, 8-way, 64B line\n");
            break;
        default:
            /* Ignore other descriptor values */
            break;
    }
}

/* Extract and process cache descriptor bytes from CPUID leaf 0x2 */
static void process_cpuid_cache_descriptors(struct cache_desc *level1,
                                            struct cache_desc *level2,
                                            int xeon_mp) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptors[16];
    int iteration = 0;
    int max_iterations = 10; /* Safety limit */
    
    printf("Starting CPUID leaf 0x2 cache descriptor processing...\n");
    
    while (iteration < max_iterations) {
        /* Call CPUID leaf 0x2 */
        cpuid(0x2, iteration, &eax, &ebx, &ecx, &edx);
        
        /* Extract descriptor bytes from registers */
        descriptors[0] = (eax >> 0) & 0xFF;
        descriptors[1] = (eax >> 8) & 0xFF;
        descriptors[2] = (eax >> 16) & 0xFF;
        descriptors[3] = (eax >> 24) & 0xFF;
        
        descriptors[4] = (ebx >> 0) & 0xFF;
        descriptors[5] = (ebx >> 8) & 0xFF;
        descriptors[6] = (ebx >> 16) & 0xFF;
        descriptors[7] = (ebx >> 24) & 0xFF;
        
        descriptors[8] = (ecx >> 0) & 0xFF;
        descriptors[9] = (ecx >> 8) & 0xFF;
        descriptors[10] = (ecx >> 16) & 0xFF;
        descriptors[11] = (ecx >> 24) & 0xFF;
        
        descriptors[12] = (edx >> 0) & 0xFF;
        descriptors[13] = (edx >> 8) & 0xFF;
        descriptors[14] = (edx >> 16) & 0xFF;
        descriptors[15] = (edx >> 24) & 0xFF;
        
        /* Process each descriptor byte */
        for (int i = 0; i < 16; i++) {
            uint8_t desc = descriptors[i];
            
            /* Check for valid descriptor (non-zero) */
            if (desc != 0x00) {
                /* Check if it's one of the target values */
                if ((desc >= 0x0a && desc <= 0x0e) ||
                    desc == 0x21 || desc == 0x24 || desc == 0x2c ||
                    (desc >= 0x39 && desc <= 0x45) ||
                    desc == 0x48 || desc == 0x49 || desc == 0x4e ||
                    (desc >= 0x60 && desc <= 0x68) ||
                    (desc >= 0x78 && desc <= 0x80) ||
                    (desc >= 0x82 && desc <= 0x87)) {
                    
                    printf("Processing descriptor 0x%02x\n", desc);
                    process_cache_descriptor(desc, level1, level2, xeon_mp);
                }
            }
        }
        
        /* Check if we should continue (EAX[7:0] gives number of times to call) */
        if ((eax & 0xFF) == 0 || iteration >= (eax & 0xFF)) {
            break;
        }
        
        iteration++;
    }
    
    printf("Finished processing %d CPUID leaf 0x2 iterations\n", iteration + 1);
}

/* Perform cache-aware computation using detected cache line size */
static void cache_aware_computation(struct cache_desc *level1, 
                                    struct cache_desc *level2) {
    int line_size = level1->line > 0 ? level1->line : 
                   (level2->line > 0 ? level2->line : 64);
    
    /* Align array to cache line boundary */
    #define ARRAY_SIZE 1024
    alignas(64) int array[ARRAY_SIZE];
    volatile int result = 0;
    
    printf("\nPerforming cache-aware computation with line size %d bytes\n", line_size);
    
    /* Access array with cache line awareness */
    for (int i = 0; i < ARRAY_SIZE; i += line_size / sizeof(int)) {
        array[i] = i;
        result += array[i];
    }
    
    /* Prevent optimization */
    printf("Computation result (volatile): %d\n", result);
}

int main(void) {
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    int xeon_mp;
    
    printf("=== Intel CPU Cache Detection Program ===\n");
    
    /* Check if CPU is Xeon MP */
    xeon_mp = is_xeon_mp();
    printf("Xeon MP detection: %s\n", xeon_mp ? "YES" : "NO");
    
    /* Process CPUID cache descriptors */
    process_cpuid_cache_descriptors(&level1, &level2, xeon_mp);
    
    /* Print final cache configuration */
    printf("\n=== Final Cache Configuration ===\n");
    if (level1.sizekb > 0) {
        printf("L1 Cache: %dKB, %d-way, %dB line\n", 
               level1.sizekb, level1.assoc, level1.line);
    }
    if (level2.sizekb > 0) {
        printf("L2 Cache: %dKB, %d-way, %dB line\n", 
               level2.sizekb, level2.assoc, level2.line);
    }
    
    /* Perform cache-aware computation */
    cache_aware_computation(&level1, &level2);
    
    return 0;
}
