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

/* Cross-platform CPUID function */
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

/* Check if CPU is Xeon MP (simplified heuristic) */
static int is_xeon_mp() {
    uint32_t eax, ebx, ecx, edx;
    
    /* Get CPU family/model/stepping from leaf 0x1 */
    cpuid(0x1, 0, &eax, &ebx, &ecx, &edx);
    
    /* Extract family/model/stepping */
    uint32_t family = ((eax >> 8) & 0xF) | ((eax >> 20) & 0xFF);
    uint32_t model = ((eax >> 4) & 0xF) | ((eax >> 12) & 0xF0);
    uint32_t stepping = eax & 0xF;
    
    /* Simplified Xeon MP detection:
       - Family 0xF (Pentium 4/Xeon)
       - Model 0x3, 0x4 (some Xeon MPs)
       This is a simplified check for demonstration */
    if (family == 0xF && (model == 0x3 || model == 0x4)) {
        /* Check Xeon signature in brand string (simplified) */
        char brand[49] = {0};
        for (int i = 0; i < 3; i++) {
            cpuid(0x80000002 + i, 0, 
                  (uint32_t*)&brand[i*16], 
                  (uint32_t*)&brand[i*16 + 4],
                  (uint32_t*)&brand[i*16 + 8],
                  (uint32_t*)&brand[i*16 + 12]);
        }
        
        /* Check for "Xeon" or "MP" in brand string */
        if (strstr(brand, "Xeon") || strstr(brand, "MP")) {
            return 1;
        }
    }
    
    return 0;
}

/* Process cache descriptor byte */
static void process_descriptor(uint8_t desc, struct cache_desc* level1, 
                               struct cache_desc* level2, int xeon_mp) {
    switch (desc) {
        /* L1 cache descriptors */
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            printf("  L1 Cache: 8KB, 2-way, 32-byte line\n");
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            printf("  L1 Cache: 16KB, 4-way, 32-byte line\n");
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("  L1 Cache: 16KB, 4-way, 64-byte line\n");
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            printf("  L1 Cache: 24KB, 6-way, 64-byte line\n");
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            printf("  L1 Cache: 32KB, 8-way, 64-byte line\n");
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            printf("  L1 Cache: 16KB, 8-way, 64-byte line\n");
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            printf("  L1 Cache: 8KB, 4-way, 64-byte line\n");
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("  L1 Cache: 16KB, 4-way, 64-byte line\n");
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            printf("  L1 Cache: 32KB, 4-way, 64-byte line\n");
            break;
            
        /* L2 cache descriptors */
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("  L2 Cache: 256KB, 8-way, 64-byte line\n");
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            printf("  L2 Cache: 1024KB, 16-way, 64-byte line\n");
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            printf("  L2 Cache: 128KB, 4-way, 64-byte line\n");
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            printf("  L2 Cache: 192KB, 6-way, 64-byte line\n");
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            printf("  L2 Cache: 128KB, 2-way, 64-byte line\n");
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            printf("  L2 Cache: 256KB, 4-way, 64-byte line\n");
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            printf("  L2 Cache: 384KB, 6-way, 64-byte line\n");
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("  L2 Cache: 512KB, 4-way, 64-byte line\n");
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            printf("  L2 Cache: 128KB, 4-way, 32-byte line\n");
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            printf("  L2 Cache: 256KB, 4-way, 32-byte line\n");
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            printf("  L2 Cache: 512KB, 4-way, 32-byte line\n");
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            printf("  L2 Cache: 1024KB, 4-way, 32-byte line\n");
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            printf("  L2 Cache: 2048KB, 4-way, 32-byte line\n");
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            printf("  L2 Cache: 3072KB, 12-way, 64-byte line\n");
            break;
        case 0x49:
            if (xeon_mp) {
                printf("  L2 Cache descriptor 0x49: Xeon MP detected, skipping\n");
                break;
            }
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            printf("  L2 Cache: 4096KB, 16-way, 64-byte line (non-Xeon MP)\n");
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            printf("  L2 Cache: 6144KB, 24-way, 64-byte line\n");
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            printf("  L2 Cache: 1024KB, 4-way, 64-byte line\n");
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            printf("  L2 Cache: 128KB, 8-way, 64-byte line\n");
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("  L2 Cache: 256KB, 8-way, 64-byte line\n");
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("  L2 Cache: 512KB, 8-way, 64-byte line\n");
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("  L2 Cache: 1024KB, 8-way, 64-byte line\n");
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            printf("  L2 Cache: 2048KB, 8-way, 64-byte line\n");
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            printf("  L2 Cache: 512KB, 2-way, 64-byte line\n");
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("  L2 Cache: 512KB, 8-way, 64-byte line\n");
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            printf("  L2 Cache: 256KB, 8-way, 32-byte line\n");
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            printf("  L2 Cache: 512KB, 8-way, 32-byte line\n");
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            printf("  L2 Cache: 1024KB, 8-way, 32-byte line\n");
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            printf("  L2 Cache: 2048KB, 8-way, 32-byte line\n");
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("  L2 Cache: 512KB, 4-way, 64-byte line\n");
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("  L2 Cache: 1024KB, 8-way, 64-byte line\n");
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
            /* Valid descriptors but not in target lines - skip */
            break;
            
        default:
            /* Invalid or reserved descriptor */
            if (desc & 0x80) {
                /* This is a TLB descriptor, ignore */
            } else if (desc != 0x00) {
                printf("  Unknown cache descriptor: 0x%02x\n", desc);
            }
            break;
    }
}

/* Extract descriptor bytes from CPUID results */
static void extract_descriptors(uint32_t eax, uint32_t ebx, 
                                uint32_t ecx, uint32_t edx,
                                uint8_t* descriptors, int* count) {
    /* According to Intel manual, bytes are returned in AL, BL, CL, DL */
    /* Each register contains up to 4 descriptor bytes */
    
    /* Process EAX (AL contains valid bit, bits 8-31 contain descriptors) */
    if (!(eax & 0x80000000)) {  /* Valid bit not set */
        descriptors[(*count)++] = (eax >> 0) & 0xFF;
        descriptors[(*count)++] = (eax >> 8) & 0xFF;
        descriptors[(*count)++] = (eax >> 16) & 0xFF;
        descriptors[(*count)++] = (eax >> 24) & 0xFF;
    }
    
    /* Process EBX */
    descriptors[(*count)++] = (ebx >> 0) & 0xFF;
    descriptors[(*count)++] = (ebx >> 8) & 0xFF;
    descriptors[(*count)++] = (ebx >> 16) & 0xFF;
    descriptors[(*count)++] = (ebx >> 24) & 0xFF;
    
    /* Process ECX */
    descriptors[(*count)++] = (ecx >> 0) & 0xFF;
    descriptors[(*count)++] = (ecx >> 8) & 0xFF;
    descriptors[(*count)++] = (ecx >> 16) & 0xFF;
    descriptors[(*count)++] = (ecx >> 24) & 0xFF;
    
    /* Process EDX */
    descriptors[(*count)++] = (edx >> 0) & 0xFF;
    descriptors[(*count)++] = (edx >> 8) & 0xFF;
    descriptors[(*count)++] = (edx >> 16) & 0xFF;
    descriptors[(*count)++] = (edx >> 24) & 0xFF;
}

int main() {
    struct cache_desc l1_cache = {0, 0, 0};
    struct cache_desc l2_cache = {0, 0, 0};
    uint8_t descriptors[64];
    int desc_count = 0;
    int iterations = 0;
    int xeon_mp = 0;
    
    printf("Starting CPU cache detection...\n");
    
    /* Step 1: Check if CPU is Xeon MP */
    xeon_mp = is_xeon_mp();
    printf("Xeon MP detection: %s\n", xeon_mp ? "YES" : "NO");
    
    /* Step 2: Call CPUID leaf 0x2 multiple times */
    /* According to Intel manual, CPUID leaf 0x2 may need to be called multiple times */
    for (iterations = 0; iterations < 16; iterations++) {
        uint32_t eax, ebx, ecx, edx;
        
        cpuid(0x2, iterations, &eax, &ebx, &ecx, &edx);
        
        /* Extract descriptor bytes */
        int start_count = desc_count;
        extract_descriptors(eax, ebx, ecx, edx, descriptors, &desc_count);
        
        /* Check if we got any new descriptors */
        if (desc_count == start_count) {
            /* No new descriptors, break */
            break;
        }
        
        /* Check if AL indicates no more iterations needed */
        if ((eax & 0xFF) == 1 && iterations > 0) {
            /* Only one iteration needed */
            break;
        }
    }
    
    printf("Collected %d cache descriptors in %d iterations\n", desc_count, iterations);
    
    /* Step 3: Process all descriptor bytes */
    for (int i = 0; i < desc_count; i++) {
        if (descriptors[i] == 0x00) {
            /* Terminator byte found */
            printf("Found terminator byte at position %d\n", i);
            break;
        }
        
        printf("Processing descriptor 0x%02x at position %d\n", descriptors[i], i);
        process_descriptor(descriptors[i], &l1_cache, &l2_cache, xeon_mp);
    }
    
    /* Step 4: Print final cache information */
    printf("\nFinal Cache Configuration:\n");
    if (l1_cache.sizekb > 0) {
        printf("L1 Cache: %d KB, %d-way, %d-byte line\n", 
               l1_cache.sizekb, l1_cache.assoc, l1_cache.line);
    } else {
        printf("L1 Cache: Not detected in target descriptors\n");
    }
    
    if (l2_cache.sizekb > 0) {
        printf("L2 Cache: %d KB, %d-way, %d-byte line\n", 
               l2_cache.sizekb, l2_cache.assoc, l2_cache.line);
    } else {
        printf("L2 Cache: Not detected in target descriptors\n");
    }
    
    /* Step 5: Perform computation using cache line size to prevent optimization */
    int line_size = l1_cache.line > 0 ? l1_cache.line : 
                   (l2_cache.line > 0 ? l2_cache.line : 64);
    
    /* Align array access to cache line boundary */
    const int ARRAY_SIZE = 1024;
    volatile int* data = (volatile int*)aligned_alloc(line_size, ARRAY_SIZE * sizeof(int));
    
    if (data) {
        /* Access data with stride equal to cache line size */
        for (int i = 0; i < ARRAY_SIZE; i += line_size / sizeof(int)) {
            data[i] = i;
        }
        
        /* Compute sum to ensure compiler doesn't optimize away */
        volatile int sum = 0;
        for (int i = 0; i < ARRAY_SIZE; i += line_size / sizeof(int)) {
            sum += data[i];
        }
        
        printf("Computation completed with cache line size %d, sum = %d\n", line_size, sum);
        free((void*)data);
    }
    
    return 0;
}
