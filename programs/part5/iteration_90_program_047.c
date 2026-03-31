/*
 * This program is designed to trigger the specific cache descriptor parsing logic
 * in driver-i386.cc lines 127-244 by executing CPUID leaf 0x2 and processing
 * the returned cache descriptor bytes according to Intel's specification.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Cross-platform CPUID intrinsic wrappers */
#if defined(_WIN32) || defined(_WIN64)
#include <intrin.h>
#else
#include <cpuid.h>
#endif

/* Cache descriptor structure matching driver-i386.cc */
struct cache_desc {
    int sizekb;     /* Size in KB */
    int assoc;      /* Associativity */
    int line;       /* Line size in bytes */
};

/* Function to execute CPUID */
static void cpuid(uint32_t leaf, uint32_t subleaf,
                  uint32_t *eax, uint32_t *ebx,
                  uint32_t *ecx, uint32_t *edx)
{
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

/* Check if CPU is Xeon MP (simplified detection) */
static int is_xeon_mp(void)
{
    uint32_t eax, ebx, ecx, edx;
    char vendor[13];
    
    /* Get vendor string */
    cpuid(0, 0, &eax, &ebx, &ecx, &edx);
    memcpy(vendor, &ebx, 4);
    memcpy(vendor + 4, &edx, 4);
    memcpy(vendor + 8, &ecx, 4);
    vendor[12] = '\0';
    
    /* Get family/model/stepping */
    cpuid(1, 0, &eax, &ebx, &ecx, &edx);
    
    int family = ((eax >> 8) & 0xF) | ((eax >> 16) & 0xFF0);
    int model = ((eax >> 4) & 0xF) | ((eax >> 12) & 0xF0);
    
    /* Simplified Xeon MP detection:
       - Vendor must be "GenuineIntel"
       - Family 6, model 0x0F (Pentium 4 Xeon MP)
       - Or other Xeon MP models */
    if (strcmp(vendor, "GenuineIntel") == 0) {
        if (family == 6 && model == 0x0F) {
            return 1;
        }
        /* Add more Xeon MP models as needed */
    }
    
    return 0;
}

/* Parse cache descriptor byte and populate cache_desc */
static void parse_cache_descriptor(uint8_t desc, struct cache_desc *level1,
                                   struct cache_desc *level2, int xeon_mp)
{
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
                printf("Descriptor 0x49: Xeon MP detected, skipping L2 cache\n");
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
            /* Ignore other descriptors */
            break;
    }
}

/* Process CPUID leaf 0x2 results */
static void process_cpuid_leaf2(int xeon_mp)
{
    uint32_t eax, ebx, ecx, edx;
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    
    printf("\n=== Processing CPUID Leaf 0x2 ===\n");
    
    /* Call CPUID leaf 0x2 multiple times as per Intel spec */
    for (int i = 0; i < 16; i++) {  /* Safety limit */
        cpuid(2, i, &eax, &ebx, &ecx, &edx);
        
        /* Extract descriptor bytes from registers */
        uint8_t descriptors[16];
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
        for (int j = 0; j < 16; j++) {
            uint8_t desc = descriptors[j];
            
            /* Check for terminator */
            if (desc == 0x00) {
                printf("Found terminator byte 0x00\n");
                return;
            }
            
            /* Check if it's a valid cache descriptor (high bit clear) */
            if ((desc & 0x80) == 0) {
                printf("Processing descriptor 0x%02x\n", desc);
                parse_cache_descriptor(desc, &level1, &level2, xeon_mp);
            }
        }
        
        /* Check if AL register indicates more calls needed */
        if ((eax & 0xFF) == 1) {
            continue;  /* Need to call again */
        } else {
            break;
        }
    }
    
    printf("\n=== Final Cache Configuration ===\n");
    if (level1.sizekb > 0) {
        printf("L1: %dKB, %d-way, %dB line\n", 
               level1.sizekb, level1.assoc, level1.line);
    }
    if (level2.sizekb > 0) {
        printf("L2: %dKB, %d-way, %dB line\n", 
               level2.sizekb, level2.assoc, level2.line);
    }
}

/* Demonstrate cache line alignment usage */
static void cache_aligned_computation(int cache_line_size)
{
    /* Allocate aligned memory based on detected cache line size */
    int alignment = cache_line_size > 0 ? cache_line_size : 64;
    int array_size = 1024;
    
    printf("\n=== Cache-Aligned Computation ===\n");
    printf("Using alignment: %d bytes\n", alignment);
    
    /* Use aligned allocation if available */
    #ifdef _ISOC11_SOURCE
    int *array = aligned_alloc(alignment, array_size * sizeof(int));
    #else
    int *array = malloc(array_size * sizeof(int) + alignment - 1);
    array = (int*)(((uintptr_t)array + alignment - 1) & ~(alignment - 1));
    #endif
    
    if (!array) {
        printf("Memory allocation failed\n");
        return;
    }
    
    /* Perform computation that uses cache line size */
    volatile int sum = 0;
    for (int i = 0; i < array_size; i += alignment / sizeof(int)) {
        array[i] = i;
        sum += array[i];
    }
    
    printf("Computation result: %d (volatile to prevent optimization)\n", sum);
    
    free(array);
}

int main(void)
{
    printf("=== CPU Cache Descriptor Parser ===\n");
    
    /* Step 1: Check Xeon MP status */
    int xeon_mp = is_xeon_mp();
    printf("Xeon MP detection: %s\n", xeon_mp ? "YES" : "NO");
    
    /* Step 2: Process CPUID leaf 0x2 */
    process_cpuid_leaf2(xeon_mp);
    
    /* Step 3: Perform cache-aligned computation */
    cache_aligned_computation(64);  /* Use typical 64-byte line */
    
    return 0;
}
