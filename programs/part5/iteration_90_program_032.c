#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Cross-platform CPUID intrinsics */
#ifdef _WIN32
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

/* CPUID wrapper for different compilers */
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

/* Check if CPU is Xeon MP (simplified logic) */
static int is_xeon_mp() {
    uint32_t eax, ebx, ecx, edx;
    
    /* Get CPU family/model/stepping */
    cpuid(1, 0, &eax, &ebx, &ecx, &edx);
    
    int family = ((eax >> 8) & 0xF) | ((eax >> 16) & 0xFF0);
    int model = ((eax >> 4) & 0xF) | ((eax >> 12) & 0xF0);
    
    /* Simplified check: Xeon MP typically has family 0xF, model >= 0x4 */
    /* This is a simplified approximation for triggering the logic */
    return (family == 0xF && model >= 0x4);
}

/* Process cache descriptor byte */
static void process_descriptor(uint8_t desc, struct cache_desc *level1, 
                               struct cache_desc *level2, int xeon_mp) {
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
                printf("Case 0x49: Xeon MP detected, skipping L2 cache update\n");
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
            /* Ignore other descriptors */
            break;
    }
}

/* Extract descriptor bytes from CPUID results */
static void extract_descriptors(uint32_t eax, uint32_t ebx, 
                                uint32_t ecx, uint32_t edx,
                                uint8_t *descriptors, int *count) {
    uint8_t *regs = (uint8_t *)&eax;
    for (int i = 0; i < 4; i++) {
        if (regs[i] != 0x00 && regs[i] != 0xFF) {
            descriptors[(*count)++] = regs[i];
        }
    }
    
    regs = (uint8_t *)&ebx;
    for (int i = 0; i < 4; i++) {
        if (regs[i] != 0x00 && regs[i] != 0xFF) {
            descriptors[(*count)++] = regs[i];
        }
    }
    
    regs = (uint8_t *)&ecx;
    for (int i = 0; i < 4; i++) {
        if (regs[i] != 0x00 && regs[i] != 0xFF) {
            descriptors[(*count)++] = regs[i];
        }
    }
    
    regs = (uint8_t *)&edx;
    for (int i = 0; i < 4; i++) {
        if (regs[i] != 0x00 && regs[i] != 0xFF) {
            descriptors[(*count)++] = regs[i];
        }
    }
}

int main() {
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    uint8_t descriptors[64];
    int desc_count = 0;
    int iteration = 0;
    
    /* Check for Xeon MP */
    int xeon_mp = is_xeon_mp();
    printf("Xeon MP detection: %s\n", xeon_mp ? "YES" : "NO");
    
    /* Call CPUID leaf 0x2 multiple times as per Intel spec */
    while (iteration < 16) {  /* Safety limit */
        uint32_t eax, ebx, ecx, edx;
        cpuid(2, iteration, &eax, &ebx, &ecx, &edx);
        
        /* Check if valid data returned */
        if ((eax & 0xFF) == 0) {
            /* No more valid descriptors */
            break;
        }
        
        /* Extract descriptor bytes */
        extract_descriptors(eax, ebx, ecx, edx, descriptors, &desc_count);
        
        iteration++;
    }
    
    printf("Found %d cache descriptor bytes\n", desc_count);
    
    /* Process all collected descriptors */
    for (int i = 0; i < desc_count; i++) {
        printf("Processing descriptor 0x%02x\n", descriptors[i]);
        process_descriptor(descriptors[i], &level1, &level2, xeon_mp);
    }
    
    /* Print final cache configuration */
    printf("\nFinal Cache Configuration:\n");
    if (level1.sizekb > 0) {
        printf("L1: %dKB, %d-way, %d-byte line\n", 
               level1.sizekb, level1.assoc, level1.line);
    }
    if (level2.sizekb > 0) {
        printf("L2: %dKB, %d-way, %d-byte line\n", 
               level2.sizekb, level2.assoc, level2.line);
    }
    
    /* Perform computation using cache line size to prevent optimization */
    int cache_line = level1.line > 0 ? level1.line : 64;
    int array_size = cache_line * 1024;
    volatile char *buffer = (volatile char *)malloc(array_size);
    
    if (buffer) {
        /* Access memory with cache line alignment */
        for (int i = 0; i < array_size; i += cache_line) {
            buffer[i] = (char)(i % 256);
        }
        
        /* Compute checksum */
        volatile char checksum = 0;
        for (int i = 0; i < array_size; i += cache_line) {
            checksum ^= buffer[i];
        }
        
        free((void*)buffer);
        printf("Computation completed with cache line size: %d bytes\n", cache_line);
    }
    
    return 0;
}
