/*
 * This program triggers Intel CPU cache detection logic by calling CPUID leaf 0x2
 * and processing cache descriptor bytes matching the uncovered cases in driver-i386.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Cross-platform CPUID intrinsics */
#if defined(_WIN32) || defined(_WIN64)
#include <intrin.h>
#include <windows.h>
#elif defined(__GNUC__) || defined(__clang__)
#include <cpuid.h>
#endif

/* Cache descriptor structure matching driver-i386.cc */
struct cache_desc {
    int sizekb;     /* Size in KB */
    int assoc;      /* Associativity */
    int line;       /* Line size in bytes */
    int level;      /* Cache level (1 or 2) */
    uint8_t type;   /* Descriptor byte */
};

/* Function to execute CPUID */
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
#elif defined(__GNUC__) || defined(__clang__)
    __cpuid_count(leaf, subleaf, *eax, *ebx, *ecx, *edx);
#endif
}

/* Check if CPU is Xeon MP (for case 0x49) */
static int is_xeon_mp(void) {
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
    
    /* Get CPU signature from leaf 0x1 */
    cpuid(0x1, 0, &eax, &ebx, &ecx, &edx);
    
    /* Extract family, model, stepping */
    uint32_t family = ((eax >> 8) & 0xF) | ((eax >> 20) & 0xFF);
    uint32_t model = ((eax >> 4) & 0xF) | ((eax >> 12) & 0xF0);
    uint32_t stepping = eax & 0xF;
    
    /* Check for Xeon MP characteristics:
     * Family 0xF (Pentium 4/Xeon), Model 0x4 or 0x6, 
     * and brand string contains "Xeon" or "MP" */
    if (family == 0xF) {
        /* Additional check: get brand string */
        char brand[49] = {0};
        uint32_t brand_regs[12];
        
        /* CPUID leaf 0x80000002-0x80000004 for brand string */
        for (uint32_t i = 0; i < 3; i++) {
            uint32_t *ptr = &brand_regs[i * 4];
            cpuid(0x80000002 + i, 0, &ptr[0], &ptr[1], &ptr[2], &ptr[3]);
        }
        memcpy(brand, brand_regs, 48);
        
        /* Check for Xeon MP indicators */
        if (strstr(brand, "Xeon") || strstr(brand, "MP")) {
            return 1;
        }
    }
    
    return 0;
}

/* Process cache descriptor byte */
static void process_descriptor(uint8_t desc, struct cache_desc *level1, 
                               struct cache_desc *level2, int xeon_mp) {
    switch (desc) {
        /* L1 Data Cache cases */
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            level1->level = 1; level1->type = desc;
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            level1->level = 1; level1->type = desc;
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            level1->level = 1; level1->type = desc;
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            level1->level = 1; level1->type = desc;
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            level1->level = 1; level1->type = desc;
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            level1->level = 1; level1->type = desc;
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            level1->level = 1; level1->type = desc;
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            level1->level = 1; level1->type = desc;
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            level1->level = 1; level1->type = desc;
            break;
            
        /* L2 Cache cases */
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = desc;
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = desc;
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = desc;
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = desc;
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = desc;
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x49:
            if (!xeon_mp) {
                level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
                level2->level = 2; level2->type = desc;
            }
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            level2->level = 2; level2->type = desc;
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            level2->level = 2; level2->type = desc;
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            level2->level = 2; level2->type = desc;
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            level2->level = 2; level2->type = desc;
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = desc;
            break;
            
        default:
            /* Other descriptor values not in our target list */
            break;
    }
}

/* Print cache information */
static void print_cache_info(const struct cache_desc *cache, const char *name) {
    if (cache->sizekb > 0) {
        printf("%s Cache (Descriptor 0x%02x):\n", name, cache->type);
        printf("  Size: %d KB\n", cache->sizekb);
        printf("  Associativity: %d-way\n", cache->assoc);
        printf("  Line size: %d bytes\n", cache->line);
        if (cache->type == 0x49) {
            printf("  Xeon MP condition: %s\n", 
                   cache->sizekb == 0 ? "skipped (Xeon MP)" : "applied (non-Xeon MP)");
        }
        printf("\n");
    }
}

/* Simple computation using cache line size */
static void cache_line_computation(int line_size) {
    /* Allocate aligned memory based on cache line size */
    const int array_size = 1024;
    char *buffer = (char*)aligned_alloc(line_size, array_size);
    
    if (buffer) {
        /* Perform simple memory access pattern */
        for (int i = 0; i < array_size; i += line_size) {
            buffer[i] = (char)(i % 256);
        }
        
        /* Sum to prevent optimization */
        volatile char sum = 0;
        for (int i = 0; i < array_size; i += line_size) {
            sum += buffer[i];
        }
        
        free(buffer);
    }
}

int main(void) {
    struct cache_desc level1 = {0};
    struct cache_desc level2 = {0};
    int xeon_mp = 0;
    
    printf("Intel CPU Cache Detection Program\n");
    printf("==================================\n\n");
    
    /* Step 1: Check for Xeon MP */
    xeon_mp = is_xeon_mp();
    printf("Xeon MP detection: %s\n\n", xeon_mp ? "YES" : "NO");
    
    /* Step 2: Call CPUID leaf 0x2 multiple times */
    printf("Reading CPUID leaf 0x2 (Cache Descriptors):\n");
    
    int iterations = 0;
    int max_iterations = 10; /* Safety limit */
    uint8_t descriptors[256];
    int desc_count = 0;
    
    while (iterations < max_iterations) {
        uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
        
        cpuid(0x2, iterations, &eax, &ebx, &ecx, &edx);
        
        /* Extract descriptor bytes from registers */
        uint8_t *reg_bytes = (uint8_t*)&eax;
        for (int i = 0; i < 4; i++) {
            if (reg_bytes[i] != 0x00) {
                descriptors[desc_count++] = reg_bytes[i];
            }
        }
        
        reg_bytes = (uint8_t*)&ebx;
        for (int i = 0; i < 4; i++) {
            if (reg_bytes[i] != 0x00) {
                descriptors[desc_count++] = reg_bytes[i];
            }
        }
        
        reg_bytes = (uint8_t*)&ecx;
        for (int i = 0; i < 4; i++) {
            if (reg_bytes[i] != 0x00) {
                descriptors[desc_count++] = reg_bytes[i];
            }
        }
        
        reg_bytes = (uint8_t*)&edx;
        for (int i = 0; i < 4; i++) {
            if (reg_bytes[i] != 0x00) {
                descriptors[desc_count++] = reg_bytes[i];
            }
        }
        
        /* Check if EAX[7:0] indicates valid descriptors */
        if ((eax & 0xFF) == 0x01) {
            /* No more valid descriptors */
            break;
        }
        
        iterations++;
    }
    
    /* Step 3: Process all collected descriptors */
    printf("Found %d cache descriptor bytes:\n", desc_count);
    for (int i = 0; i < desc_count; i++) {
        printf("  0x%02x", descriptors[i]);
        if ((i + 1) % 8 == 0) printf("\n");
    }
    printf("\n\n");
    
    /* Process each descriptor */
    for (int i = 0; i < desc_count; i++) {
        process_descriptor(descriptors[i], &level1, &level2, xeon_mp);
    }
    
    /* Step 4: Print cache information */
    print_cache_info(&level1, "L1");
    print_cache_info(&level2, "L2");
    
    /* Step 5: Perform computation using cache line size */
    printf("Performing cache-aware computation:\n");
    if (level1.line > 0) {
        printf("Using L1 cache line size: %d bytes\n", level1.line);
        cache_line_computation(level1.line);
    }
    if (level2.line > 0) {
        printf("Using L2 cache line size: %d bytes\n", level2.line);
        cache_line_computation(level2.line);
    }
    
    printf("\nProgram completed successfully.\n");
    
    return 0;
}
