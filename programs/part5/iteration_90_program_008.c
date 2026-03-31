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
    
    /* Extract family, model, stepping */
    uint32_t family = ((eax >> 8) & 0xF) | ((eax >> 20) & 0xFF);
    uint32_t model = ((eax >> 4) & 0xF) | ((eax >> 12) & 0xF0);
    uint32_t stepping = eax & 0xF;
    
    /* Simplified Xeon MP detection:
       - Family 0xF (Pentium 4/Xeon)
       - Model >= 0x4 (some Xeon MP models)
       This is a simplified check for demonstration */
    if (family == 0xF && model >= 0x4) {
        /* Check extended model for Xeon MP characteristics */
        uint32_t extended_model = (eax >> 16) & 0xF;
        if (extended_model >= 0x3) {
            return 1;
        }
    }
    
    return 0;
}

/* Parse cache descriptor byte and populate cache_desc */
static void parse_cache_descriptor(uint8_t desc, struct cache_desc* level1, 
                                   struct cache_desc* level2, int xeon_mp) {
    switch (desc) {
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
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("  L2 Cache: 256KB, 8-way, 64-byte line\n");
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            printf("  L2 Cache: 1024KB, 16-way, 64-byte line\n");
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            printf("  L1 Cache: 32KB, 8-way, 64-byte line\n");
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
                printf("  Case 0x49: Xeon MP detected, skipping L2 cache update\n");
                break;
            }
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            printf("  L2 Cache: 4096KB, 16-way, 64-byte line (non-Xeon MP)\n");
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            printf("  L2 Cache: 6144KB, 24-way, 64-byte line\n");
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
        default:
            /* Ignore other descriptor values */
            break;
    }
}

/* Process CPUID leaf 0x2 cache descriptors */
static void process_cache_descriptors(struct cache_desc* level1, 
                                      struct cache_desc* level2, 
                                      int xeon_mp) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptors[16];
    int iteration = 0;
    int max_iterations = 10; /* Safety limit */
    
    printf("Starting cache descriptor parsing...\n");
    
    /* According to Intel spec, CPUID leaf 0x2 may need multiple calls */
    while (iteration < max_iterations) {
        cpuid(0x2, 0, &eax, &ebx, &ecx, &edx);
        
        /* Extract descriptor bytes from registers */
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
            
            /* Check for valid descriptor (0x00 is terminator) */
            if (desc == 0x00) {
                printf("Found terminator byte 0x00 at position %d\n", i);
                return;
            }
            
            /* Check if high bit is set (register is valid, not a descriptor) */
            if (desc & 0x80) {
                continue;
            }
            
            /* Parse the descriptor */
            parse_cache_descriptor(desc, level1, level2, xeon_mp);
        }
        
        iteration++;
    }
    
    printf("Reached maximum iterations without finding terminator\n");
}

/* Use cache line size for computation to prevent optimization */
static void cache_line_optimized_computation(int cache_line_size) {
    /* Allocate aligned memory based on cache line size */
    size_t alignment = (cache_line_size > 0) ? cache_line_size : 64;
    size_t array_size = 1024 * 1024; /* 1MB */
    
    /* Use volatile to prevent optimization */
    volatile char* buffer = (volatile char*)aligned_alloc(alignment, array_size);
    if (!buffer) {
        return;
    }
    
    /* Perform memory access with cache line alignment */
    for (size_t i = 0; i < array_size; i += alignment) {
        buffer[i] = (char)(i & 0xFF);
    }
    
    /* Sum to ensure computation isn't optimized away */
    volatile char sum = 0;
    for (size_t i = 0; i < array_size; i += alignment) {
        sum += buffer[i];
    }
    
    free((void*)buffer);
    
    /* Use sum to prevent dead code elimination */
    if (sum == 0) {
        printf("Cache line computation completed (sum=%d)\n", sum);
    }
}

int main() {
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    int xeon_mp;
    
    printf("=== CPU Cache Detection Program ===\n");
    
    /* Step 1: Check if CPU is Xeon MP */
    xeon_mp = is_xeon_mp();
    printf("Xeon MP detection: %s\n", xeon_mp ? "YES" : "NO");
    
    /* Step 2: Process cache descriptors from CPUID leaf 0x2 */
    process_cache_descriptors(&level1, &level2, xeon_mp);
    
    /* Step 3: Print final cache information */
    printf("\n=== Final Cache Configuration ===\n");
    if (level1.sizekb > 0) {
        printf("L1 Cache: %dKB, %d-way, %d-byte line\n", 
               level1.sizekb, level1.assoc, level1.line);
    }
    if (level2.sizekb > 0) {
        printf("L2 Cache: %dKB, %d-way, %d-byte line\n", 
               level2.sizekb, level2.assoc, level2.line);
    }
    
    /* Step 4: Perform computation using cache line size */
    int cache_line_size = (level1.line > 0) ? level1.line : 
                         (level2.line > 0) ? level2.line : 64;
    printf("\nPerforming cache-optimized computation with line size: %d bytes\n", 
           cache_line_size);
    cache_line_optimized_computation(cache_line_size);
    
    printf("\nProgram completed successfully.\n");
    return 0;
}
