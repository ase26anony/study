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

/* Extract cache descriptor bytes from CPUID results */
static void extract_descriptor_bytes(uint32_t reg, uint8_t *bytes) {
    bytes[0] = (reg >> 0) & 0xFF;
    bytes[1] = (reg >> 8) & 0xFF;
    bytes[2] = (reg >> 16) & 0xFF;
    bytes[3] = (reg >> 24) & 0xFF;
}

/* Check if processor is Xeon MP (simplified check) */
static int is_xeon_mp(uint32_t family, uint32_t model, uint32_t stepping) {
    /* Simplified: Family 0xF (Pentium 4/Xeon), Model 0x6 (Xeon MP) */
    if (family == 0xF && model == 0x6) {
        return 1;
    }
    /* Additional check for older Xeon MP (Family 6, Model 1Eh) */
    if (family == 6 && model == 0x1E) {
        return 1;
    }
    return 0;
}

/* Process a single cache descriptor byte */
static void process_descriptor_byte(uint8_t desc, struct cache_desc *level1,
                                    struct cache_desc *level2, int xeon_mp) {
    switch (desc) {
        /* L1 Data Cache cases */
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            printf("  Cache descriptor 0x%02x: L1 Data 8KB, 2-way, 32B line\n", desc);
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            printf("  Cache descriptor 0x%02x: L1 Data 16KB, 4-way, 32B line\n", desc);
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("  Cache descriptor 0x%02x: L1 Data 16KB, 4-way, 64B line\n", desc);
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            printf("  Cache descriptor 0x%02x: L1 Data 24KB, 6-way, 64B line\n", desc);
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            printf("  Cache descriptor 0x%02x: L1 Data 32KB, 8-way, 64B line\n", desc);
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            printf("  Cache descriptor 0x%02x: L1 Data 16KB, 8-way, 64B line\n", desc);
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            printf("  Cache descriptor 0x%02x: L1 Data 8KB, 4-way, 64B line\n", desc);
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("  Cache descriptor 0x%02x: L1 Data 16KB, 4-way, 64B line\n", desc);
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            printf("  Cache descriptor 0x%02x: L1 Data 32KB, 4-way, 64B line\n", desc);
            break;
            
        /* L2 Cache cases */
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 256KB, 8-way, 64B line\n", desc);
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 1MB, 16-way, 64B line\n", desc);
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 128KB, 4-way, 64B line\n", desc);
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 192KB, 6-way, 64B line\n", desc);
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 128KB, 2-way, 64B line\n", desc);
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 256KB, 4-way, 64B line\n", desc);
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 384KB, 6-way, 64B line\n", desc);
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 512KB, 4-way, 64B line\n", desc);
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            printf("  Cache descriptor 0x%02x: L2 128KB, 4-way, 32B line\n", desc);
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            printf("  Cache descriptor 0x%02x: L2 256KB, 4-way, 32B line\n", desc);
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            printf("  Cache descriptor 0x%02x: L2 512KB, 4-way, 32B line\n", desc);
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            printf("  Cache descriptor 0x%02x: L2 1MB, 4-way, 32B line\n", desc);
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            printf("  Cache descriptor 0x%02x: L2 2MB, 4-way, 32B line\n", desc);
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 3MB, 12-way, 64B line\n", desc);
            break;
        case 0x49:
            if (xeon_mp) {
                printf("  Cache descriptor 0x%02x: Skipped (Xeon MP detected)\n", desc);
                break;
            }
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 4MB, 16-way, 64B line\n", desc);
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 6MB, 24-way, 64B line\n", desc);
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 1MB, 4-way, 64B line\n", desc);
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 128KB, 8-way, 64B line\n", desc);
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 256KB, 8-way, 64B line\n", desc);
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 512KB, 8-way, 64B line\n", desc);
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 1MB, 8-way, 64B line\n", desc);
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 2MB, 8-way, 64B line\n", desc);
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 512KB, 2-way, 64B line\n", desc);
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 512KB, 8-way, 64B line\n", desc);
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            printf("  Cache descriptor 0x%02x: L2 256KB, 8-way, 32B line\n", desc);
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            printf("  Cache descriptor 0x%02x: L2 512KB, 8-way, 32B line\n", desc);
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            printf("  Cache descriptor 0x%02x: L2 1MB, 8-way, 32B line\n", desc);
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            printf("  Cache descriptor 0x%02x: L2 2MB, 8-way, 32B line\n", desc);
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 512KB, 4-way, 64B line\n", desc);
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("  Cache descriptor 0x%02x: L2 1MB, 8-way, 64B line\n", desc);
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
            printf("  Cache descriptor 0x%02x: Valid but not in target lines\n", desc);
            break;
            
        /* Invalid descriptor (bit 0 = 0) */
        default:
            if ((desc & 0x1F) != desc) {
                printf("  Cache descriptor 0x%02x: Invalid (reserved bits set)\n", desc);
            }
            break;
    }
}

int main(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t bytes[4];
    int i, j;
    int xeon_mp = 0;
    
    /* Initialize cache descriptor structures */
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    
    printf("=== CPU Cache Detection Program ===\n\n");
    
    /* Step 1: Get basic processor info (CPUID leaf 0x1) */
    cpuid(0x1, 0, &eax, &ebx, &ecx, &edx);
    
    /* Extract family, model, stepping */
    uint32_t stepping = eax & 0xF;
    uint32_t model = (eax >> 4) & 0xF;
    uint32_t family = (eax >> 8) & 0xF;
    if (family == 0xF) {
        family += (eax >> 20) & 0xFF;
        model |= ((eax >> 16) & 0xF) << 4;
    }
    
    printf("CPU Info: Family 0x%x, Model 0x%x, Stepping 0x%x\n",
           family, model, stepping);
    
    /* Determine if this is Xeon MP */
    xeon_mp = is_xeon_mp(family, model, stepping);
    printf("Xeon MP detection: %s\n\n", xeon_mp ? "YES" : "NO");
    
    /* Step 2: Get cache descriptors (CPUID leaf 0x2) */
    printf("Reading cache descriptors from CPUID leaf 0x2:\n");
    
    /* According to Intel spec, CPUID leaf 0x2 may need to be called multiple times */
    for (i = 0; i < 16; i++) {  /* Safety limit */
        cpuid(0x2, i, &eax, &ebx, &ecx, &edx);
        
        /* Check if this is the first call - AL register contains number of calls needed */
        if (i == 0) {
            int calls_needed = eax & 0xFF;
            printf("CPUID leaf 0x2 requires %d iteration(s)\n", calls_needed);
        }
        
        /* Extract descriptor bytes from all four registers */
        extract_descriptor_bytes(eax, bytes);
        for (j = 0; j < 4; j++) {
            if (bytes[j] == 0x00) {
                printf("  Found terminator byte 0x00\n");
                goto done_parsing;
            }
            process_descriptor_byte(bytes[j], &level1, &level2, xeon_mp);
        }
        
        extract_descriptor_bytes(ebx, bytes);
        for (j = 0; j < 4; j++) {
            if (bytes[j] == 0x00) {
                printf("  Found terminator byte 0x00\n");
                goto done_parsing;
            }
            process_descriptor_byte(bytes[j], &level1, &level2, xeon_mp);
        }
        
        extract_descriptor_bytes(ecx, bytes);
        for (j = 0; j < 4; j++) {
            if (bytes[j] == 0x00) {
                printf("  Found terminator byte 0x00\n");
                goto done_parsing;
            }
            process_descriptor_byte(bytes[j], &level1, &level2, xeon_mp);
        }
        
        extract_descriptor_bytes(edx, bytes);
        for (j = 0; j < 4; j++) {
            if (bytes[j] == 0x00) {
                printf("  Found terminator byte 0x00\n");
                goto done_parsing;
            }
            process_descriptor_byte(bytes[j], &level1, &level2, xeon_mp);
        }
    }
    
done_parsing:
    
    /* Step 3: Print collected cache information */
    printf("\n=== Detected Cache Information ===\n");
    if (level1.sizekb > 0) {
        printf("L1 Data Cache: %d KB, %d-way, %d byte line\n",
               level1.sizekb, level1.assoc, level1.line);
    } else {
        printf("L1 Data Cache: Not detected by target descriptors\n");
    }
    
    if (level2.sizekb > 0) {
        printf("L2 Cache: %d KB, %d-way, %d byte line\n",
               level2.sizekb, level2.assoc, level2.line);
    } else {
        printf("L2 Cache: Not detected by target descriptors\n");
    }
    
    /* Step 4: Perform computation using cache line size to prevent optimization */
    printf("\n=== Cache-Aware Computation ===\n");
    
    /* Use the largest detected cache line size */
    int cache_line = level1.line > level2.line ? level1.line : level2.line;
    if (cache_line == 0) {
        cache_line = 64;  /* Default assumption */
        printf("No cache line size detected, using default %d bytes\n", cache_line);
    } else {
        printf("Using detected cache line size: %d bytes\n", cache_line);
    }
    
    /* Allocate aligned memory and perform computation */
    const int array_size = 1024 * cache_line;
    char *buffer = (char*)aligned_alloc(cache_line, array_size);
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize and access memory with cache line alignment */
    memset(buffer, 0, array_size);
    volatile int sum = 0;
    
    /* Access every cache line to ensure cache effects */
    for (i = 0; i < array_size; i += cache_line) {
        buffer[i] = (char)(i % 256);
        sum += buffer[i];  /* Volatile access prevents optimization */
    }
    
    printf("Performed cache-aligned memory access (sum = %d)\n", sum);
    printf("Array size: %d bytes, Cache line: %d bytes\n", array_size, cache_line);
    
    free(buffer);
    
    /* Special check for case 0x49 */
    printf("\n=== Special Case 0x49 Check ===\n");
    if (xeon_mp) {
        printf("Xeon MP processor detected - case 0x49 would be skipped\n");
    } else {
        printf("Non-Xeon MP processor - case 0x49 would set L2 to 4MB, 16-way, 64B line\n");
    }
    
    return 0;
}
