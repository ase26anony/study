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

/* Cache descriptor structure matching driver-i386.cc */
struct cache_desc {
    int sizekb;
    int assoc;
    int line;
};

/* Cross-platform CPUID wrapper */
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

/* Check if CPU is Xeon MP (simplified logic) */
static int is_xeon_mp() {
    uint32_t eax, ebx, ecx, edx;
    
    /* CPUID leaf 0x1 for family/model/stepping */
    cpuid(0x1, 0, &eax, &ebx, &ecx, &edx);
    
    /* Extract family/model/stepping */
    uint32_t stepping = eax & 0xF;
    uint32_t model = (eax >> 4) & 0xF;
    uint32_t family = (eax >> 8) & 0xF;
    uint32_t extended_model = (eax >> 16) & 0xF;
    uint32_t extended_family = (eax >> 20) & 0xFF;
    
    /* Simplified Xeon MP detection:
       - Family 0xF (Pentium 4/Xeon)
       - Model checking for Xeon MP specific models
       This is a simplified version - real detection would be more complex */
    if (family == 0xF) {
        /* Check for Xeon MP models (simplified) */
        uint32_t full_model = (extended_model << 4) | model;
        /* Some Xeon MP models: 0x2, 0x3, 0x4 for certain families */
        if (full_model >= 0x2 && full_model <= 0x4) {
            return 1;
        }
    }
    
    return 0;
}

/* Process cache descriptor byte */
static void process_descriptor(uint8_t desc, struct cache_desc* level1, 
                               struct cache_desc* level2, int xeon_mp) {
    switch (desc) {
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            printf("L1 Cache: 8KB, 2-way, 32-byte line\n");
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            printf("L1 Cache: 16KB, 4-way, 32-byte line\n");
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("L1 Cache: 16KB, 4-way, 64-byte line\n");
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            printf("L1 Cache: 24KB, 6-way, 64-byte line\n");
            break;
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 256KB, 8-way, 64-byte line\n");
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            printf("L2 Cache: 1024KB, 16-way, 64-byte line\n");
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            printf("L1 Cache: 32KB, 8-way, 64-byte line\n");
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 128KB, 4-way, 64-byte line\n");
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            printf("L2 Cache: 192KB, 6-way, 64-byte line\n");
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            printf("L2 Cache: 128KB, 2-way, 64-byte line\n");
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 256KB, 4-way, 64-byte line\n");
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            printf("L2 Cache: 384KB, 6-way, 64-byte line\n");
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 512KB, 4-way, 64-byte line\n");
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 128KB, 4-way, 32-byte line\n");
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 256KB, 4-way, 32-byte line\n");
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 512KB, 4-way, 32-byte line\n");
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 1024KB, 4-way, 32-byte line\n");
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 2048KB, 4-way, 32-byte line\n");
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            printf("L2 Cache: 3072KB, 12-way, 64-byte line\n");
            break;
        case 0x49:
            if (xeon_mp) {
                printf("Case 0x49: Xeon MP detected, skipping L2 cache setup\n");
                break;
            }
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            printf("L2 Cache: 4096KB, 16-way, 64-byte line (non-Xeon MP)\n");
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            printf("L2 Cache: 6144KB, 24-way, 64-byte line\n");
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            printf("L1 Cache: 16KB, 8-way, 64-byte line\n");
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            printf("L1 Cache: 8KB, 4-way, 64-byte line\n");
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("L1 Cache: 16KB, 4-way, 64-byte line\n");
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            printf("L1 Cache: 32KB, 4-way, 64-byte line\n");
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 1024KB, 4-way, 64-byte line\n");
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 128KB, 8-way, 64-byte line\n");
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 256KB, 8-way, 64-byte line\n");
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 512KB, 8-way, 64-byte line\n");
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 1024KB, 8-way, 64-byte line\n");
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 2048KB, 8-way, 64-byte line\n");
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            printf("L2 Cache: 512KB, 2-way, 64-byte line\n");
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 512KB, 8-way, 64-byte line\n");
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            printf("L2 Cache: 256KB, 8-way, 32-byte line\n");
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            printf("L2 Cache: 512KB, 8-way, 32-byte line\n");
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            printf("L2 Cache: 1024KB, 8-way, 32-byte line\n");
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            printf("L2 Cache: 2048KB, 8-way, 32-byte line\n");
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 512KB, 4-way, 64-byte line\n");
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 1024KB, 8-way, 64-byte line\n");
            break;
        default:
            /* Valid descriptor but not in our uncovered lines */
            if (desc != 0x00 && (desc & 0x80) == 0) {
                printf("Unknown cache descriptor: 0x%02x\n", desc);
            }
            break;
    }
}

/* Extract and process descriptor bytes from CPUID results */
static void extract_descriptors(uint32_t reg, uint8_t* descriptors, int* count) {
    uint8_t* bytes = (uint8_t*)&reg;
    
    for (int i = 0; i < 4; i++) {
        if (bytes[i] != 0x00) {
            descriptors[(*count)++] = bytes[i];
        }
    }
}

/* Simple computation using cache line size to prevent optimization */
static void cache_line_computation(int cache_line_size) {
    volatile int* array;
    int size = cache_line_size * 16;
    
    /* Allocate aligned memory */
    array = (volatile int*)aligned_alloc(cache_line_size, size);
    if (!array) return;
    
    /* Access memory with cache line alignment */
    for (int i = 0; i < size / sizeof(int); i += cache_line_size / sizeof(int)) {
        array[i] = i;
    }
    
    /* Sum to create side effect */
    volatile int sum = 0;
    for (int i = 0; i < size / sizeof(int); i += cache_line_size / sizeof(int)) {
        sum += array[i];
    }
    
    free((void*)array);
    (void)sum; /* Use sum to prevent optimization */
}

int main() {
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
        
        cpuid(0x2, iterations, &eax, &ebx, &ecx, &edx);
        
        /* Extract descriptor bytes from all registers */
        extract_descriptors(eax, descriptors, &desc_count);
        extract_descriptors(ebx, descriptors, &desc_count);
        extract_descriptors(ecx, descriptors, &desc_count);
        extract_descriptors(edx, descriptors, &desc_count);
        
        /* Check if we should continue (bit 7 of AL indicates if we need to call again) */
        if ((eax & 0xFF) == 0x00 || iterations > 0) {
            /* Found terminator or already iterating */
            break;
        }
        
        iterations++;
    }
    
    printf("Found %d cache descriptors\n", desc_count);
    
    /* Step 3: Process all descriptor bytes */
    for (int i = 0; i < desc_count; i++) {
        printf("Processing descriptor 0x%02x\n", descriptors[i]);
        process_descriptor(descriptors[i], &level1, &level2, xeon_mp);
    }
    
    /* Step 4: Print final cache information */
    printf("\nFinal Cache Configuration:\n");
    if (level1.sizekb > 0) {
        printf("L1: %dKB, %d-way, %d-byte line\n", 
               level1.sizekb, level1.assoc, level1.line);
    }
    if (level2.sizekb > 0) {
        printf("L2: %dKB, %d-way, %d-byte line\n", 
               level2.sizekb, level2.assoc, level2.line);
    }
    
    /* Step 5: Perform computation using cache line size */
    int cache_line = level1.line > 0 ? level1.line : 
                    (level2.line > 0 ? level2.line : 64);
    printf("\nPerforming computation with cache line size: %d bytes\n", cache_line);
    cache_line_computation(cache_line);
    
    return 0;
}
