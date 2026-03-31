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

/* Cross-platform CPUID wrapper */
static void cpuid(uint32_t leaf, uint32_t subleaf, 
                  uint32_t* eax, uint32_t* ebx, 
                  uint32_t* ecx, uint32_t* edx) {
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
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
    cpuid(1, 0, &eax, &ebx, &ecx, &edx);
    
    /* Extract family/model/stepping */
    uint32_t family = (eax >> 8) & 0xF;
    uint32_t model = (eax >> 4) & 0xF;
    uint32_t extended_family = (eax >> 20) & 0xFF;
    uint32_t extended_model = (eax >> 16) & 0xF;
    
    if (family == 0xF) {
        family = extended_family + family;
        model = (extended_model << 4) + model;
    }
    
    /* Simplified Xeon MP detection: 
       Family 6, Model 44/46/47 are some Xeon MP variants */
    if (family == 6) {
        if (model == 0x2C || model == 0x2E || model == 0x2F) {
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

/* Extract bytes from CPUID result */
static void extract_descriptors(uint32_t reg, uint8_t* descriptors, int* count) {
    uint8_t* bytes = (uint8_t*)&reg;
    for (int i = 0; i < 4; i++) {
        if (bytes[i] != 0x00) {
            descriptors[(*count)++] = bytes[i];
        }
    }
}

int main() {
    struct cache_desc l1_cache = {0, 0, 0};
    struct cache_desc l2_cache = {0, 0, 0};
    uint8_t descriptors[256];
    int desc_count = 0;
    
    /* Check Xeon MP status */
    int xeon_mp = is_xeon_mp();
    printf("Xeon MP status: %s\n\n", xeon_mp ? "YES" : "NO");
    
    /* Call CPUID leaf 0x2 multiple times as per Intel spec */
    int iterations = 0;
    int max_iterations = 16; /* Safety limit */
    
    while (iterations < max_iterations) {
        uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
        cpuid(2, 0, &eax, &ebx, &ecx, &edx);
        
        /* Check if AL contains number of iterations needed */
        uint8_t al = eax & 0xFF;
        
        /* Extract descriptor bytes from all registers */
        extract_descriptors(eax, descriptors, &desc_count);
        extract_descriptors(ebx, descriptors, &desc_count);
        extract_descriptors(ecx, descriptors, &desc_count);
        extract_descriptors(edx, descriptors, &desc_count);
        
        iterations++;
        
        /* Check for terminator in AL */
        if (al == 1) {
            /* Only one iteration needed */
            break;
        }
        
        /* Check if we have a valid terminator (0x00) in descriptors */
        int has_terminator = 0;
        for (int i = 0; i < desc_count; i++) {
            if (descriptors[i] == 0x00) {
                has_terminator = 1;
                break;
            }
        }
        if (has_terminator) {
            break;
        }
    }
    
    /* Process all collected descriptors */
    printf("Processing %d cache descriptors:\n", desc_count);
    for (int i = 0; i < desc_count; i++) {
        if (descriptors[i] == 0x00) {
            break;
        }
        printf("Descriptor 0x%02x: ", descriptors[i]);
        process_descriptor(descriptors[i], &l1_cache, &l2_cache, xeon_mp);
    }
    
    /* Print final cache information */
    printf("\nFinal Cache Configuration:\n");
    if (l1_cache.sizekb > 0) {
        printf("L1: %dKB, %d-way, %d-byte line\n", 
               l1_cache.sizekb, l1_cache.assoc, l1_cache.line);
    }
    if (l2_cache.sizekb > 0) {
        printf("L2: %dKB, %d-way, %d-byte line\n", 
               l2_cache.sizekb, l2_cache.assoc, l2_cache.line);
    }
    
    /* Use cache line size for computation to prevent optimization */
    int line_size = l1_cache.line > 0 ? l1_cache.line : 64;
    volatile int* array = (volatile int*)aligned_alloc(line_size, line_size * 4);
    
    if (array) {
        /* Aligned access matching cache line */
        for (int i = 0; i < line_size / sizeof(int); i++) {
            array[i] = i * 2;
        }
        
        /* Compute sum to ensure runtime effect */
        int sum = 0;
        for (int i = 0; i < line_size / sizeof(int); i++) {
            sum += array[i];
        }
        
        printf("Computation result using %d-byte alignment: %d\n", line_size, sum);
        free((void*)array);
    }
    
    return 0;
}
