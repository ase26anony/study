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

/* Cache descriptor structure matching driver-i386.cc logic */
struct cache_desc {
    int sizekb;    /* Size in KB */
    int assoc;     /* Associativity */
    int line;      /* Line size in bytes */
    int level;     /* Cache level (1 or 2) */
    uint8_t descriptor; /* Original descriptor byte */
};

/* CPUID wrapper function */
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
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
    
    /* Get CPU signature from leaf 0x1 */
    cpuid(0x1, 0, &eax, &ebx, &ecx, &edx);
    
    /* Extract family, model, stepping */
    uint32_t family = ((eax >> 8) & 0x0F) | ((eax >> 20) & 0xFF);
    uint32_t model = ((eax >> 4) & 0x0F) | ((eax >> 12) & 0xF0);
    uint32_t stepping = eax & 0x0F;
    
    /* Check for Xeon MP characteristics:
     * Family 0xF (Pentium 4/Xeon), Model 0x3, 0x4, or specific Xeon MP models
     * This is a simplified check - real detection would be more complex */
    if (family == 0xF) {
        /* Check for Xeon MP models (simplified) */
        if (model == 0x3 || model == 0x4) {
            /* Additional check: Xeon MP typically has specific feature bits */
            if ((edx & (1 << 28)) && (edx & (1 << 25))) {
                return 1;
            }
        }
    }
    
    return 0;
}

/* Process a single cache descriptor byte */
static void process_descriptor(uint8_t desc, struct cache_desc* level1, 
                               struct cache_desc* level2, int xeon_mp) {
    switch (desc) {
        /* L1 Data Cache cases */
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            level1->level = 1; level1->descriptor = desc;
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            level1->level = 1; level1->descriptor = desc;
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            level1->level = 1; level1->descriptor = desc;
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            level1->level = 1; level1->descriptor = desc;
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            level1->level = 1; level1->descriptor = desc;
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            level1->level = 1; level1->descriptor = desc;
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            level1->level = 1; level1->descriptor = desc;
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            level1->level = 1; level1->descriptor = desc;
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            level1->level = 1; level1->descriptor = desc;
            break;
            
        /* L2 Cache cases */
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->descriptor = desc;
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            level2->level = 2; level2->descriptor = desc;
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->descriptor = desc;
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            level2->level = 2; level2->descriptor = desc;
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            level2->level = 2; level2->descriptor = desc;
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->descriptor = desc;
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            level2->level = 2; level2->descriptor = desc;
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->descriptor = desc;
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->descriptor = desc;
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->descriptor = desc;
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->descriptor = desc;
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->descriptor = desc;
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->descriptor = desc;
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            level2->level = 2; level2->descriptor = desc;
            break;
        case 0x49:
            /* Special case with Xeon MP check */
            if (!xeon_mp) {
                level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
                level2->level = 2; level2->descriptor = desc;
            }
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            level2->level = 2; level2->descriptor = desc;
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->descriptor = desc;
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->descriptor = desc;
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->descriptor = desc;
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->descriptor = desc;
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->descriptor = desc;
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->descriptor = desc;
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            level2->level = 2; level2->descriptor = desc;
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->descriptor = desc;
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            level2->level = 2; level2->descriptor = desc;
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            level2->level = 2; level2->descriptor = desc;
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            level2->level = 2; level2->descriptor = desc;
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            level2->level = 2; level2->descriptor = desc;
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->descriptor = desc;
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->descriptor = desc;
            break;
            
        default:
            /* Ignore other descriptors */
            break;
    }
}

/* Print cache information */
static void print_cache_info(const struct cache_desc* cache, const char* name) {
    if (cache->sizekb > 0) {
        printf("%s Cache (descriptor 0x%02x):\n", name, cache->descriptor);
        printf("  Size: %d KB\n", cache->sizekb);
        printf("  Associativity: %d-way\n", cache->assoc);
        printf("  Line size: %d bytes\n", cache->line);
        printf("\n");
    }
}

/* Perform computation using cache line size to prevent optimization */
static void perform_cache_aware_computation(int cache_line_size) {
    volatile int result = 0;
    const int array_size = 1024 * 1024; /* 1MB */
    char* buffer = (char*)malloc(array_size);
    
    if (buffer) {
        /* Align access to cache line boundary */
        char* aligned_ptr = (char*)(((uintptr_t)buffer + cache_line_size - 1) & ~(cache_line_size - 1));
        
        /* Perform computation that uses cache line size */
        for (int i = 0; i < array_size; i += cache_line_size) {
            aligned_ptr[i] = (char)(i & 0xFF);
            result += aligned_ptr[i];
        }
        
        /* Use result to prevent optimization */
        printf("Cache-aware computation result: %d\n", result);
        
        free(buffer);
    }
}

int main(void) {
    struct cache_desc level1 = {0};
    struct cache_desc level2 = {0};
    int xeon_mp = 0;
    int iterations = 0;
    
    printf("=== CPU Cache Descriptor Detection ===\n\n");
    
    /* Step 1: Check for Xeon MP */
    xeon_mp = is_xeon_mp();
    printf("Xeon MP detection: %s\n\n", xeon_mp ? "YES" : "NO");
    
    /* Step 2: Call CPUID leaf 0x2 multiple times */
    printf("Processing CPUID leaf 0x2 cache descriptors:\n");
    
    do {
        uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
        
        /* Call CPUID with leaf 0x2 */
        cpuid(0x2, iterations, &eax, &ebx, &ecx, &edx);
        
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
        for (int i = 0; i < 16; i++) {
            uint8_t desc = descriptors[i];
            
            /* Check for valid descriptor (not 0x00) */
            if (desc != 0x00) {
                printf("  Descriptor 0x%02x\n", desc);
                process_descriptor(desc, &level1, &level2, xeon_mp);
            }
        }
        
        iterations++;
        
        /* Check if we should continue (bit 7 of AL indicates if we need to call again) */
        if ((eax & 0x80) == 0) {
            break;
        }
        
    } while (iterations < 10); /* Safety limit */
    
    /* Step 3: Print collected cache information */
    printf("\n=== Detected Cache Information ===\n\n");
    print_cache_info(&level1, "L1");
    print_cache_info(&level2, "L2");
    
    /* Special handling for case 0x49 */
    if (level2.descriptor == 0x49) {
        printf("Note: Case 0x49 processed with Xeon MP = %s\n", 
               xeon_mp ? "true (cache ignored)" : "false (cache configured)");
    }
    
    /* Step 4: Perform cache-aware computation */
    printf("=== Cache-Aware Computation ===\n");
    int cache_line_size = level1.line > 0 ? level1.line : 
                         (level2.line > 0 ? level2.line : 64);
    perform_cache_aware_computation(cache_line_size);
    
    return 0;
}
