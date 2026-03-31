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
#include <windows.h>
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
    uint32_t family = ((eax >> 8) & 0xF) | ((eax >> 16) & 0xFF0);
    uint32_t model = ((eax >> 4) & 0xF) | ((eax >> 12) & 0xF0);
    uint32_t stepping = eax & 0xF;
    
    /* Check processor type bits (8-11) */
    uint32_t type = (eax >> 12) & 0x3;
    
    /* For this test, we'll simulate both conditions:
     * Return 1 for Xeon MP, 0 for non-Xeon MP
     * We'll base this on family/model heuristics
     */
    if (family == 0xF && model >= 0x4) {
        /* Intel Xeon MP processors (pre-Core microarchitecture) */
        return 1;
    }
    
    /* Also check brand string for "Xeon MP" */
    char brand[49] = {0};
    for (uint32_t i = 0; i < 3; i++) {
        uint32_t regs[4];
        cpuid(0x80000002 + i, 0, &regs[0], &regs[1], &regs[2], &regs[3]);
        memcpy(brand + i * 16, regs, 16);
    }
    
    if (strstr(brand, "Xeon") && strstr(brand, "MP")) {
        return 1;
    }
    
    return 0;
}

/* Process cache descriptor byte */
static void process_descriptor(uint8_t desc, struct cache_desc *level1,
                               struct cache_desc *level2, int xeon_mp) {
    switch (desc) {
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            printf("  Found L1 descriptor 0x0a: 8KB, 2-way, 32B line\n");
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            printf("  Found L1 descriptor 0x0c: 16KB, 4-way, 32B line\n");
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("  Found L1 descriptor 0x0d: 16KB, 4-way, 64B line\n");
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            printf("  Found L1 descriptor 0x0e: 24KB, 6-way, 64B line\n");
            break;
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("  Found L2 descriptor 0x21: 256KB, 8-way, 64B line\n");
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            printf("  Found L2 descriptor 0x24: 1024KB, 16-way, 64B line\n");
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            printf("  Found L1 descriptor 0x2c: 32KB, 8-way, 64B line\n");
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            printf("  Found L2 descriptor 0x39: 128KB, 4-way, 64B line\n");
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            printf("  Found L2 descriptor 0x3a: 192KB, 6-way, 64B line\n");
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            printf("  Found L2 descriptor 0x3b: 128KB, 2-way, 64B line\n");
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            printf("  Found L2 descriptor 0x3c: 256KB, 4-way, 64B line\n");
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            printf("  Found L2 descriptor 0x3d: 384KB, 6-way, 64B line\n");
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("  Found L2 descriptor 0x3e: 512KB, 4-way, 64B line\n");
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            printf("  Found L2 descriptor 0x41: 128KB, 4-way, 32B line\n");
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            printf("  Found L2 descriptor 0x42: 256KB, 4-way, 32B line\n");
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            printf("  Found L2 descriptor 0x43: 512KB, 4-way, 32B line\n");
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            printf("  Found L2 descriptor 0x44: 1024KB, 4-way, 32B line\n");
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            printf("  Found L2 descriptor 0x45: 2048KB, 4-way, 32B line\n");
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            printf("  Found L2 descriptor 0x48: 3072KB, 12-way, 64B line\n");
            break;
        case 0x49:
            if (xeon_mp) {
                printf("  Found descriptor 0x49 but Xeon MP detected - skipping\n");
                break;
            }
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            printf("  Found L2 descriptor 0x49: 4096KB, 16-way, 64B line\n");
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            printf("  Found L2 descriptor 0x4e: 6144KB, 24-way, 64B line\n");
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            printf("  Found L1 descriptor 0x60: 16KB, 8-way, 64B line\n");
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            printf("  Found L1 descriptor 0x66: 8KB, 4-way, 64B line\n");
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("  Found L1 descriptor 0x67: 16KB, 4-way, 64B line\n");
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            printf("  Found L1 descriptor 0x68: 32KB, 4-way, 64B line\n");
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            printf("  Found L2 descriptor 0x78: 1024KB, 4-way, 64B line\n");
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            printf("  Found L2 descriptor 0x79: 128KB, 8-way, 64B line\n");
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("  Found L2 descriptor 0x7a: 256KB, 8-way, 64B line\n");
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("  Found L2 descriptor 0x7b: 512KB, 8-way, 64B line\n");
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("  Found L2 descriptor 0x7c: 1024KB, 8-way, 64B line\n");
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            printf("  Found L2 descriptor 0x7d: 2048KB, 8-way, 64B line\n");
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            printf("  Found L2 descriptor 0x7f: 512KB, 2-way, 64B line\n");
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("  Found L2 descriptor 0x80: 512KB, 8-way, 64B line\n");
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            printf("  Found L2 descriptor 0x82: 256KB, 8-way, 32B line\n");
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            printf("  Found L2 descriptor 0x83: 512KB, 8-way, 32B line\n");
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            printf("  Found L2 descriptor 0x84: 1024KB, 8-way, 32B line\n");
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            printf("  Found L2 descriptor 0x85: 2048KB, 8-way, 32B line\n");
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("  Found L2 descriptor 0x86: 512KB, 4-way, 64B line\n");
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("  Found L2 descriptor 0x87: 1024KB, 8-way, 64B line\n");
            break;
        case 0x00:
            /* Valid terminator - do nothing */
            break;
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
        case 0x06:
            /* TLB descriptors - skip */
            break;
        default:
            /* Unknown descriptor */
            printf("  Unknown descriptor: 0x%02x\n", desc);
            break;
    }
}

/* Extract descriptor bytes from CPUID results */
static void extract_descriptors(uint32_t eax, uint32_t ebx,
                                uint32_t ecx, uint32_t edx,
                                uint8_t *descriptors, int *count) {
    /* AL register contains number of times CPUID(2) should be called */
    uint8_t times = eax & 0xFF;
    
    /* Extract descriptor bytes from registers */
    uint8_t *regs = (uint8_t*)&eax;
    for (int i = 1; i < 4; i++) {  /* Skip AL byte */
        if (regs[i] != 0x00) {
            descriptors[(*count)++] = regs[i];
        }
    }
    
    regs = (uint8_t*)&ebx;
    for (int i = 0; i < 4; i++) {
        if (regs[i] != 0x00) {
            descriptors[(*count)++] = regs[i];
        }
    }
    
    regs = (uint8_t*)&ecx;
    for (int i = 0; i < 4; i++) {
        if (regs[i] != 0x00) {
            descriptors[(*count)++] = regs[i];
        }
    }
    
    regs = (uint8_t*)&edx;
    for (int i = 0; i < 4; i++) {
        if (regs[i] != 0x00) {
            descriptors[(*count)++] = regs[i];
        }
    }
}

int main(void) {
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    uint8_t descriptors[256];
    int desc_count = 0;
    int xeon_mp = 0;
    
    printf("=== CPU Cache Descriptor Detection ===\n");
    
    /* Step 1: Check Xeon MP status */
    xeon_mp = is_xeon_mp();
    printf("Xeon MP status: %s\n", xeon_mp ? "Yes" : "No");
    
    /* Step 2: Call CPUID leaf 0x2 multiple times */
    printf("\nCalling CPUID leaf 0x2...\n");
    
    uint32_t eax, ebx, ecx, edx;
    cpuid(0x2, 0, &eax, &ebx, &ecx, &edx);
    
    /* Get number of times to call CPUID(2) */
    uint8_t times = eax & 0xFF;
    printf("CPUID(2) should be called %d time(s)\n", times);
    
    /* Extract descriptors from first call */
    extract_descriptors(eax, ebx, ecx, edx, descriptors, &desc_count);
    
    /* Call additional times if needed */
    for (int i = 1; i < times; i++) {
        cpuid(0x2, i, &eax, &ebx, &ecx, &edx);
        extract_descriptors(eax, ebx, ecx, edx, descriptors, &desc_count);
    }
    
    printf("Found %d descriptor bytes\n", desc_count);
    
    /* Step 3: Process all descriptor bytes */
    printf("\nProcessing descriptors:\n");
    for (int i = 0; i < desc_count; i++) {
        process_descriptor(descriptors[i], &level1, &level2, xeon_mp);
    }
    
    /* Step 4: Print collected cache information */
    printf("\n=== Cache Information Summary ===\n");
    if (level1.sizekb > 0) {
        printf("L1 Cache: %d KB, %d-way, %d byte line\n",
               level1.sizekb, level1.assoc, level1.line);
    } else {
        printf("L1 Cache: Not detected\n");
    }
    
    if (level2.sizekb > 0) {
        printf("L2 Cache: %d KB, %d-way, %d byte line\n",
               level2.sizekb, level2.assoc, level2.line);
    } else {
        printf("L2 Cache: Not detected\n");
    }
    
    /* Step 5: Perform computation using cache line size */
    printf("\n=== Cache-Aware Computation ===\n");
    int line_size = level1.line > 0 ? level1.line : 64; /* Default to 64 if not detected */
    printf("Using cache line size: %d bytes\n", line_size);
    
    /* Allocate aligned memory for cache-aware access */
    size_t array_size = 1024 * 1024; /* 1MB */
    char *buffer = (char*)aligned_alloc(line_size, array_size);
    
    if (buffer) {
        /* Fill buffer with pattern */
        for (size_t i = 0; i < array_size; i++) {
            buffer[i] = (char)(i % 256);
        }
        
        /* Access memory with cache line alignment */
        volatile char sum = 0;
        for (size_t i = 0; i < array_size; i += line_size) {
            sum += buffer[i];
        }
        
        /* Use sum to prevent optimization */
        printf("Cache-aligned access completed. Checksum: %d\n", (int)sum);
        
        free(buffer);
    } else {
        printf("Failed to allocate aligned memory\n");
    }
    
    /* Additional check for case 0x49 */
    printf("\n=== Special Case Check (0x49) ===\n");
    printf("For descriptor 0x49, Xeon MP condition is %s\n",
           xeon_mp ? "TRUE (cache not configured)" : "FALSE (cache configured)");
    
    return 0;
}
