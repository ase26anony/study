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

/* CPUID wrapper for cross-platform compatibility */
static void cpuid(uint32_t leaf, uint32_t subleaf, 
                  uint32_t* eax, uint32_t* ebx, 
                  uint32_t* ecx, uint32_t* edx) {
#ifdef _WIN32
    int cpu_info[4];
    __cpuidex(cpu_info, leaf, subleaf);
    *eax = cpu_info[0];
    *ebx = cpu_info[1];
    *ecx = cpu_info[2];
    *edx = cpu_info[3];
#else
    __cpuid_count(leaf, subleaf, *eax, *ebx, *ecx, *edx);
#endif
}

/* Check if CPU is Xeon MP (simplified detection) */
static int is_xeon_mp(uint32_t family, uint32_t model, uint32_t stepping) {
    /* Simplified Xeon MP detection based on family/model */
    /* Family 0xF (Pentium 4/Xeon), Model 0x6 (Xeon MP) or similar */
    if (family == 0xF && model >= 0x6) {
        /* Additional checks could be added here */
        return 1;
    }
    return 0;
}

/* Process cache descriptor byte */
static void process_cache_descriptor(uint8_t desc, struct cache_desc* level1, 
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
            /* Other descriptor values are ignored */
            break;
    }
}

/* Simple computation using cache line size to prevent optimization */
static void cache_aware_computation(int cache_line_size) {
    /* Align array to cache line boundary */
    char* buffer = aligned_alloc(cache_line_size, 1024 * cache_line_size);
    if (!buffer) return;
    
    /* Access memory in cache-line sized chunks */
    for (int i = 0; i < 1024; i++) {
        buffer[i * cache_line_size] = (char)(i & 0xFF);
    }
    
    /* Simple computation to use the buffer */
    volatile char sum = 0;
    for (int i = 0; i < 1024; i++) {
        sum += buffer[i * cache_line_size];
    }
    
    free(buffer);
}

int main() {
    uint32_t eax, ebx, ecx, edx;
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    int xeon_mp = 0;
    
    printf("=== CPU Cache Detection Program ===\n");
    
    /* Step 1: Get CPU family/model/stepping from CPUID leaf 0x1 */
    cpuid(0x1, 0, &eax, &ebx, &ecx, &edx);
    
    /* Extract family/model/stepping */
    uint32_t stepping = eax & 0xF;
    uint32_t model = (eax >> 4) & 0xF;
    uint32_t family = (eax >> 8) & 0xF;
    if (family == 0xF) {
        model |= ((eax >> 16) & 0xF) << 4;
        family += (eax >> 20) & 0xFF;
    }
    
    printf("CPU Family: %u, Model: %u, Stepping: %u\n", family, model, stepping);
    
    /* Determine Xeon MP status */
    xeon_mp = is_xeon_mp(family, model, stepping);
    printf("Xeon MP: %s\n", xeon_mp ? "Yes" : "No");
    
    /* Step 2: Get cache descriptors from CPUID leaf 0x2 */
    printf("\nProcessing CPUID Leaf 0x2 Cache Descriptors:\n");
    
    /* According to Intel spec, CPUID leaf 0x2 may need to be called multiple times */
    for (int iteration = 0; iteration < 3; iteration++) {
        cpuid(0x2, iteration, &eax, &ebx, &ecx, &edx);
        
        /* Check if valid data returned (EAX bit 31 = 0) */
        if (eax & 0x80000000) {
            /* Number of times CPUID with EAX=2 should be executed */
            int times = eax & 0xFF;
            printf("CPUID leaf 0x2 should be called %d times\n", times);
        }
        
        /* Process descriptor bytes from all registers */
        uint8_t descriptors[16];
        memcpy(descriptors, &eax, 4);
        memcpy(descriptors + 4, &ebx, 4);
        memcpy(descriptors + 8, &ecx, 4);
        memcpy(descriptors + 12, &edx, 4);
        
        /* Process each descriptor byte */
        for (int i = 0; i < 16; i++) {
            uint8_t desc = descriptors[i];
            
            /* Skip invalid/reserved bytes */
            if (desc == 0x00) {
                /* Terminator byte found */
                printf("Terminator byte (0x00) found at position %d\n", i);
                break;
            }
            
            /* Skip if high bit set (register is valid, not a descriptor) */
            if (desc & 0x80) {
                continue;
            }
            
            /* Process the cache descriptor */
            printf("Processing descriptor 0x%02x\n", desc);
            process_cache_descriptor(desc, &level1, &level2, xeon_mp);
        }
    }
    
    /* Step 3: Print final cache information */
    printf("\n=== Final Cache Information ===\n");
    if (level1.sizekb > 0) {
        printf("L1 Cache: %dKB, %d-way, %d-byte line\n", 
               level1.sizekb, level1.assoc, level1.line);
    }
    if (level2.sizekb > 0) {
        printf("L2 Cache: %dKB, %d-way, %d-byte line\n", 
               level2.sizekb, level2.assoc, level2.line);
    }
    
    /* Step 4: Perform cache-aware computation */
    printf("\nPerforming cache-aware computation...\n");
    int cache_line_size = level1.line > 0 ? level1.line : 
                         (level2.line > 0 ? level2.line : 64);
    cache_aware_computation(cache_line_size);
    
    printf("\nProgram completed successfully.\n");
    return 0;
}
