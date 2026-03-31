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

/* Determine if CPU is Xeon MP (simplified check) */
static int is_xeon_mp() {
    uint32_t eax, ebx, ecx, edx;
    
    /* Get CPU signature from leaf 0x1 */
    cpuid(0x1, 0, &eax, &ebx, &ecx, &edx);
    
    /* Extract family, model, stepping */
    uint32_t family = ((eax >> 8) & 0xF) | ((eax >> 20) & 0xFF);
    uint32_t model = ((eax >> 4) & 0xF) | ((eax >> 12) & 0xF0);
    uint32_t stepping = eax & 0xF;
    
    /* Simplified Xeon MP detection:
       - Family 0xF (Pentium 4/Xeon)
       - Model 0x6 (Xeon MP 2.8GHz/3.0GHz) or similar
       This is a simplified check for demonstration */
    if (family == 0xF && model >= 0x6) {
        /* Check if it's MP capable (extended model bits) */
        uint32_t extended_model = (eax >> 16) & 0xF;
        if (extended_model >= 0x3) {
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
                printf("Case 0x49: Xeon MP detected, skipping L2 cache update\n");
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
            /* Valid descriptor but not in our target cases */
            if (desc != 0x00 && (desc & 0x80) == 0) {
                printf("Other valid cache descriptor: 0x%02x\n", desc);
            }
            break;
    }
}

/* Extract descriptor bytes from CPUID results */
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

/* Perform cache-aware computation using detected cache line size */
static void cache_aware_computation(int cache_line_size) {
    /* Allocate aligned memory for cache-friendly access */
    size_t array_size = 1024 * 1024; /* 1MB */
    char* buffer = (char*)aligned_alloc(cache_line_size, array_size);
    
    if (buffer) {
        /* Perform strided access based on cache line size */
        volatile char sum = 0;
        for (size_t i = 0; i < array_size; i += cache_line_size) {
            buffer[i] = (char)(i % 256);
            sum += buffer[i];
        }
        
        /* Use sum to prevent optimization */
        printf("Cache-aware computation result (dummy): %d\n", (int)sum);
        
        free(buffer);
    }
}

int main() {
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    uint8_t descriptors[64];
    int desc_count = 0;
    
    printf("=== CPU Cache Detection Program ===\n");
    
    /* Step 1: Check for Xeon MP */
    int xeon_mp = is_xeon_mp();
    printf("Xeon MP detection: %s\n", xeon_mp ? "YES" : "NO");
    
    /* Step 2: Call CPUID leaf 0x2 multiple times */
    printf("\nCalling CPUID leaf 0x2 (Cache Descriptors):\n");
    
    uint32_t eax, ebx, ecx, edx;
    int iteration = 0;
    int valid_descriptors_found = 0;
    
    do {
        cpuid(0x2, iteration, &eax, &ebx, &ecx, &edx);
        
        /* Check if EAX[7:0] indicates valid descriptors */
        if ((eax & 0xFF) == 0) {
            break; /* No more valid descriptors */
        }
        
        /* Extract descriptor bytes from all registers */
        extract_descriptors(eax, ebx, ecx, edx, descriptors, &desc_count);
        
        iteration++;
        
        /* Safety check to prevent infinite loop */
        if (iteration > 10) {
            printf("Warning: Exceeded maximum CPUID leaf 0x2 iterations\n");
            break;
        }
    } while (1);
    
    /* Step 3: Process all collected descriptors */
    printf("\nProcessing %d cache descriptor bytes:\n", desc_count);
    
    for (int i = 0; i < desc_count; i++) {
        uint8_t desc = descriptors[i];
        
        /* Skip invalid/reserved descriptors */
        if (desc == 0x00 || (desc & 0x80)) {
            continue;
        }
        
        valid_descriptors_found++;
        printf("Descriptor 0x%02x: ", desc);
        
        /* Process through the exact switch cases from uncovered lines */
        process_descriptor(desc, &level1, &level2, xeon_mp);
    }
    
    /* Step 4: Print final cache information */
    printf("\n=== Final Cache Configuration ===\n");
    if (level1.sizekb > 0) {
        printf("L1 Cache: %dKB, %d-way, %dB line\n", 
               level1.sizekb, level1.assoc, level1.line);
    }
    if (level2.sizekb > 0) {
        printf("L2 Cache: %dKB, %d-way, %dB line\n", 
               level2.sizekb, level2.assoc, level2.line);
    }
    
    if (valid_descriptors_found == 0) {
        printf("No valid cache descriptors found in CPUID leaf 0x2\n");
        printf("This CPU may use deterministic cache parameters (leaf 0x4)\n");
    }
    
    /* Step 5: Perform cache-aware computation */
    printf("\n=== Cache-Aware Computation ===\n");
    int cache_line_size = level1.line > 0 ? level1.line : 
                         (level2.line > 0 ? level2.line : 64);
    
    printf("Using cache line size: %d bytes\n", cache_line_size);
    cache_aware_computation(cache_line_size);
    
    return 0;
}
