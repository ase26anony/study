/*
 * This program triggers Intel CPU cache detection logic by calling CPUID leaf 0x2
 * and processing cache descriptor bytes matching the uncovered cases from driver-i386.cc
 */

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
    int sizekb;     /* Size in KB */
    int assoc;      /* Associativity */
    int line;       /* Line size in bytes */
    int level;      /* Cache level (1 or 2) */
    int type;       /* Cache type */
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

/* Check if CPU is Xeon MP (for case 0x49) */
static int is_xeon_mp() {
    uint32_t eax, ebx, ecx, edx;
    
    /* Get CPU family/model/stepping from leaf 0x1 */
    cpuid(0x1, 0, &eax, &ebx, &ecx, &edx);
    
    /* Extract family, model, stepping */
    uint32_t family = ((eax >> 8) & 0xF) | ((eax >> 16) & 0xFF0);
    uint32_t model = ((eax >> 4) & 0xF) | ((eax >> 12) & 0xF0);
    uint32_t stepping = eax & 0xF;
    
    /* Simplified Xeon MP detection:
     * - Family 0xF (Pentium 4/Xeon)
     * - Model >= 0x4 (some Xeon MP models)
     * This is a simplified check for demonstration */
    if (family == 0xF && model >= 0x4) {
        /* Check extended model for Xeon MP characteristics */
        uint32_t extended_model = (eax >> 16) & 0xF;
        if (extended_model >= 0x3) {
            return 1; /* Likely Xeon MP */
        }
    }
    
    return 0;
}

/* Process cache descriptor byte */
static void process_descriptor(uint8_t desc, struct cache_desc *level1, 
                               struct cache_desc *level2, int xeon_mp) {
    switch (desc) {
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            level1->level = 1; level1->type = 1;
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            level1->level = 1; level1->type = 1;
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            level1->level = 1; level1->type = 1;
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            level1->level = 1; level1->type = 1;
            break;
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 2;
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            level2->level = 2; level2->type = 2;
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            level1->level = 1; level1->type = 1;
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = 2;
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            level2->level = 2; level2->type = 2;
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            level2->level = 2; level2->type = 2;
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = 2;
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            level2->level = 2; level2->type = 2;
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = 2;
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = 2;
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = 2;
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = 2;
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = 2;
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = 2;
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            level2->level = 2; level2->type = 2;
            break;
        case 0x49:
            if (xeon_mp) {
                /* Xeon MP case - break without setting values */
                printf("Case 0x49: Xeon MP detected, skipping L2 cache setup\n");
                break;
            }
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            level2->level = 2; level2->type = 2;
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            level2->level = 2; level2->type = 2;
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            level1->level = 1; level1->type = 1;
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            level1->level = 1; level1->type = 1;
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            level1->level = 1; level1->type = 1;
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            level1->level = 1; level1->type = 1;
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = 2;
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 2;
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 2;
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 2;
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 2;
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 2;
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            level2->level = 2; level2->type = 2;
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 2;
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            level2->level = 2; level2->type = 2;
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            level2->level = 2; level2->type = 2;
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            level2->level = 2; level2->type = 2;
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            level2->level = 2; level2->type = 2;
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = 2;
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 2;
            break;
        default:
            /* Not one of our target cases */
            break;
    }
}

/* Print cache information */
static void print_cache_info(const struct cache_desc *cache, const char *name) {
    if (cache->sizekb > 0) {
        printf("%s Cache: %d KB, %d-way associative, %d byte line size\n",
               name, cache->sizekb, cache->assoc, cache->line);
    }
}

/* Perform computation using cache line size to prevent optimization */
static void cache_aware_computation(int line_size) {
    volatile int result = 0;
    
    /* Allocate aligned memory based on cache line size */
    int array_size = 1024 * line_size;
    char *buffer = (char*)aligned_alloc(line_size, array_size);
    
    if (buffer) {
        /* Access memory with cache line alignment */
        for (int i = 0; i < array_size; i += line_size) {
            buffer[i] = (char)(i % 256);
            result += buffer[i];
        }
        
        /* Use result to prevent dead code elimination */
        printf("Cache-aware computation result: %d (using line size %d)\n", result, line_size);
        
        free(buffer);
    }
}

int main() {
    struct cache_desc l1_cache = {0};
    struct cache_desc l2_cache = {0};
    int xeon_mp = 0;
    
    printf("Starting Intel CPU cache detection...\n");
    
    /* Step 1: Check if CPU is Xeon MP (for case 0x49) */
    xeon_mp = is_xeon_mp();
    printf("Xeon MP detection: %s\n", xeon_mp ? "YES" : "NO");
    
    /* Step 2: Call CPUID leaf 0x2 multiple times to get cache descriptors */
    uint32_t eax, ebx, ecx, edx;
    int iteration = 0;
    int valid_descriptors = 0;
    
    do {
        cpuid(0x2, iteration, &eax, &ebx, &ecx, &edx);
        
        /* Check if EAX[7:0] indicates valid descriptors */
        uint8_t al = eax & 0xFF;
        
        /* Process descriptor bytes from all registers */
        uint8_t descriptors[16];
        
        /* EAX bytes (skip the first byte which is the count) */
        descriptors[0] = (eax >> 8) & 0xFF;
        descriptors[1] = (eax >> 16) & 0xFF;
        descriptors[2] = (eax >> 24) & 0xFF;
        
        /* EBX bytes */
        descriptors[3] = ebx & 0xFF;
        descriptors[4] = (ebx >> 8) & 0xFF;
        descriptors[5] = (ebx >> 16) & 0xFF;
        descriptors[6] = (ebx >> 24) & 0xFF;
        
        /* ECX bytes */
        descriptors[7] = ecx & 0xFF;
        descriptors[8] = (ecx >> 8) & 0xFF;
        descriptors[9] = (ecx >> 16) & 0xFF;
        descriptors[10] = (ecx >> 24) & 0xFF;
        
        /* EDX bytes */
        descriptors[11] = edx & 0xFF;
        descriptors[12] = (edx >> 8) & 0xFF;
        descriptors[13] = (edx >> 16) & 0xFF;
        descriptors[14] = (edx >> 24) & 0xFF;
        
        /* Process each descriptor byte */
        for (int i = 0; i < 15; i++) {
            uint8_t desc = descriptors[i];
            
            /* Skip invalid descriptors (0x00) */
            if (desc == 0x00) {
                continue;
            }
            
            /* Check if this is a valid cache descriptor (not a TLB descriptor) */
            if ((desc & 0x80) == 0) {
                printf("Processing cache descriptor 0x%02x (iteration %d, byte %d)\n", 
                       desc, iteration, i);
                
                process_descriptor(desc, &l1_cache, &l2_cache, xeon_mp);
                valid_descriptors++;
            }
        }
        
        iteration++;
        
        /* According to Intel spec, we should call CPUID(2) EAX[7:0] times */
        if (iteration > (eax & 0xFF)) {
            break;
        }
        
    } while (iteration < 10); /* Safety limit */
    
    printf("\nCache Detection Results:\n");
    printf("========================\n");
    printf("Valid descriptors processed: %d\n", valid_descriptors);
    
    /* Print detected cache information */
    print_cache_info(&l1_cache, "L1");
    print_cache_info(&l2_cache, "L2");
    
    /* Step 3: Perform cache-aware computation */
    printf("\nPerforming cache-aware computation...\n");
    int line_size = l1_cache.line > 0 ? l1_cache.line : 
                   (l2_cache.line > 0 ? l2_cache.line : 64);
    cache_aware_computation(line_size);
    
    /* Additional check for case 0x49 logic */
    if (xeon_mp) {
        printf("\nNote: Xeon MP processor detected - case 0x49 would skip L2 cache setup\n");
    }
    
    return 0;
}
