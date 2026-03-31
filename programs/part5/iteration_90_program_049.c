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
    int sizekb;
    int assoc;
    int line;
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

/* Check if CPU is Xeon MP (simplified logic) */
static int is_xeon_mp() {
    uint32_t eax, ebx, ecx, edx;
    
    /* Get CPU signature from leaf 0x1 */
    cpuid(0x1, 0, &eax, &ebx, &ecx, &edx);
    
    /* Extract family, model, stepping */
    uint32_t family = ((eax >> 8) & 0xF) | ((eax >> 20) & 0xFF);
    uint32_t model = ((eax >> 4) & 0xF) | ((eax >> 12) & 0xF0);
    uint32_t stepping = eax & 0xF;
    
    /* Simplified Xeon MP detection:
       Family 0xF (Pentium 4/Xeon), Model 0x6 (Xeon MP) or similar */
    if (family == 0xF && model >= 0x6) {
        /* Check if it's a Xeon (from brand string would be better) */
        char brand[49] = {0};
        uint32_t brand_regs[12];
        
        /* Get brand string leaves 0x80000002-0x80000004 */
        for (int i = 0; i < 3; i++) {
            cpuid(0x80000002 + i, 0, 
                  &brand_regs[0 + i*4], 
                  &brand_regs[1 + i*4],
                  &brand_regs[2 + i*4], 
                  &brand_regs[3 + i*4]);
        }
        memcpy(brand, brand_regs, 48);
        
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
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            printf("L1 Cache: 8KB, 2-way, 32B line (0x0a)\n");
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            printf("L1 Cache: 16KB, 4-way, 32B line (0x0c)\n");
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("L1 Cache: 16KB, 4-way, 64B line (0x0d)\n");
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            printf("L1 Cache: 24KB, 6-way, 64B line (0x0e)\n");
            break;
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 256KB, 8-way, 64B line (0x21)\n");
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            printf("L2 Cache: 1024KB, 16-way, 64B line (0x24)\n");
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            printf("L1 Cache: 32KB, 8-way, 64B line (0x2c)\n");
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 128KB, 4-way, 64B line (0x39)\n");
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            printf("L2 Cache: 192KB, 6-way, 64B line (0x3a)\n");
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            printf("L2 Cache: 128KB, 2-way, 64B line (0x3b)\n");
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 256KB, 4-way, 64B line (0x3c)\n");
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            printf("L2 Cache: 384KB, 6-way, 64B line (0x3d)\n");
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 512KB, 4-way, 64B line (0x3e)\n");
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 128KB, 4-way, 32B line (0x41)\n");
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 256KB, 4-way, 32B line (0x42)\n");
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 512KB, 4-way, 32B line (0x43)\n");
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 1024KB, 4-way, 32B line (0x44)\n");
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            printf("L2 Cache: 2048KB, 4-way, 32B line (0x45)\n");
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            printf("L2 Cache: 3072KB, 12-way, 64B line (0x48)\n");
            break;
        case 0x49:
            if (xeon_mp) {
                printf("Descriptor 0x49: Xeon MP detected, skipping L2 cache\n");
                break;
            }
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            printf("L2 Cache: 4096KB, 16-way, 64B line (0x49)\n");
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            printf("L2 Cache: 6144KB, 24-way, 64B line (0x4e)\n");
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            printf("L1 Cache: 16KB, 8-way, 64B line (0x60)\n");
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            printf("L1 Cache: 8KB, 4-way, 64B line (0x66)\n");
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("L1 Cache: 16KB, 4-way, 64B line (0x67)\n");
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            printf("L1 Cache: 32KB, 4-way, 64B line (0x68)\n");
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 1024KB, 4-way, 64B line (0x78)\n");
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 128KB, 8-way, 64B line (0x79)\n");
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 256KB, 8-way, 64B line (0x7a)\n");
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 512KB, 8-way, 64B line (0x7b)\n");
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 1024KB, 8-way, 64B line (0x7c)\n");
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 2048KB, 8-way, 64B line (0x7d)\n");
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            printf("L2 Cache: 512KB, 2-way, 64B line (0x7f)\n");
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 512KB, 8-way, 64B line (0x80)\n");
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            printf("L2 Cache: 256KB, 8-way, 32B line (0x82)\n");
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            printf("L2 Cache: 512KB, 8-way, 32B line (0x83)\n");
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            printf("L2 Cache: 1024KB, 8-way, 32B line (0x84)\n");
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            printf("L2 Cache: 2048KB, 8-way, 32B line (0x85)\n");
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("L2 Cache: 512KB, 4-way, 64B line (0x86)\n");
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("L2 Cache: 1024KB, 8-way, 64B line (0x87)\n");
            break;
        default:
            /* Valid descriptor but not in our uncovered lines */
            if (desc != 0x00 && (desc & 0x80) == 0) {
                printf("Other valid cache descriptor: 0x%02x\n", desc);
            }
            break;
    }
}

/* Extract bytes from CPUID result */
static void extract_descriptors(uint32_t eax, uint32_t ebx, 
                                uint32_t ecx, uint32_t edx,
                                uint8_t* descriptors, int* count) {
    uint8_t* regs = (uint8_t*)&eax;
    for (int i = 0; i < 4; i++) {
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

int main() {
    struct cache_desc l1_cache = {0, 0, 0};
    struct cache_desc l2_cache = {0, 0, 0};
    uint8_t descriptors[256];
    int desc_count = 0;
    int iterations = 0;
    
    printf("=== CPU Cache Detection Program ===\n");
    
    /* Step 1: Check for Xeon MP */
    int xeon_mp = is_xeon_mp();
    printf("Xeon MP detection: %s\n", xeon_mp ? "YES" : "NO");
    
    /* Step 2: Call CPUID leaf 0x2 multiple times */
    printf("\nCalling CPUID leaf 0x2 (Cache/TLB descriptors):\n");
    
    while (iterations < 16) { /* Safety limit */
        uint32_t eax, ebx, ecx, edx;
        cpuid(0x2, iterations, &eax, &ebx, &ecx, &edx);
        
        /* Check if EAX[7:0] indicates number of iterations */
        uint8_t iterations_needed = eax & 0xFF;
        
        /* Extract descriptor bytes */
        extract_descriptors(eax, ebx, ecx, edx, descriptors, &desc_count);
        
        /* Check for terminator in first byte */
        if ((eax & 0xFF) == 0x00) {
            break;
        }
        
        iterations++;
        
        /* If iterations_needed is 1, we're done */
        if (iterations_needed == 1) {
            break;
        }
    }
    
    /* Step 3: Process all collected descriptors */
    printf("\nProcessing %d cache descriptor bytes:\n", desc_count);
    for (int i = 0; i < desc_count; i++) {
        printf("  Byte %d: 0x%02x\n", i, descriptors[i]);
        process_descriptor(descriptors[i], &l1_cache, &l2_cache, xeon_mp);
    }
    
    /* Step 4: Print final cache information */
    printf("\n=== Final Cache Configuration ===\n");
    if (l1_cache.sizekb > 0) {
        printf("L1 Cache: %dKB, %d-way, %dB line\n", 
               l1_cache.sizekb, l1_cache.assoc, l1_cache.line);
    }
    if (l2_cache.sizekb > 0) {
        printf("L2 Cache: %dKB, %d-way, %dB line\n", 
               l2_cache.sizekb, l2_cache.assoc, l2_cache.line);
    }
    
    /* Step 5: Perform computation using cache line size */
    printf("\n=== Cache-Aware Computation ===\n");
    int line_size = l1_cache.line > 0 ? l1_cache.line : 64;
    printf("Using cache line size: %d bytes\n", line_size);
    
    /* Allocate aligned memory */
    size_t array_size = 1024 * 1024; /* 1MB */
    char* buffer = (char*)aligned_alloc(line_size, array_size);
    
    if (buffer) {
        /* Access memory with cache line alignment */
        volatile int sum = 0;
        for (size_t i = 0; i < array_size; i += line_size) {
            buffer[i] = (char)(i % 256);
            sum += buffer[i];
        }
        
        printf("Cache-aligned access completed. Checksum: %d\n", sum);
        free(buffer);
    }
    
    printf("\nProgram completed successfully.\n");
    return 0;
}
