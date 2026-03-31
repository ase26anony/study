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
#define cpuid(info, x) __cpuid(info, x)
#define cpuidex(info, x, ecx_val) __cpuidex(info, x, ecx_val)
#else
#include <cpuid.h>
#define cpuid(info, x) __cpuid(x, info[0], info[1], info[2], info[3])
#define cpuidex(info, x, ecx_val) __cpuid_count(x, ecx_val, info[0], info[1], info[2], info[3])
#endif

/* Cache descriptor structure matching driver-i386.cc */
struct cache_desc {
    int sizekb;    /* Size in KB */
    int assoc;     /* Associativity */
    int line;      /* Line size in bytes */
    int level;     /* Cache level (1 or 2) */
    uint8_t desc;  /* Original descriptor byte */
};

/* Function to check if CPU is Xeon MP (simulating xeon_mp variable) */
static int is_xeon_mp(void) {
    uint32_t info[4];
    cpuid(info, 0x00000001);
    
    /* Extract family, model, stepping */
    uint32_t stepping = info[0] & 0xF;
    uint32_t model = (info[0] >> 4) & 0xF;
    uint32_t family = (info[0] >> 8) & 0xF;
    uint32_t extended_model = (info[0] >> 16) & 0xF;
    uint32_t extended_family = (info[0] >> 20) & 0xFF;
    
    /* For demonstration, simulate both true and false conditions */
    /* In real code, this would check specific Xeon MP models */
    static int toggle = 0;
    toggle = !toggle;
    
    /* Print CPU info for debugging */
    printf("CPU Info: Family %u, Model %u, Stepping %u\n", 
           family + extended_family, model + (extended_model << 4), stepping);
    
    /* Return simulated Xeon MP status */
    return toggle;
}

/* Parse cache descriptor byte and populate cache_desc structure */
static void parse_cache_descriptor(uint8_t desc, struct cache_desc *cache, int xeon_mp) {
    /* Initialize cache structure */
    cache->desc = desc;
    cache->sizekb = 0;
    cache->assoc = 0;
    cache->line = 0;
    cache->level = (desc >= 0x40) ? 2 : 1; /* Simple heuristic for demo */
    
    /* Exact switch cases from driver-i386.cc lines 127-244 */
    switch (desc) {
        case 0x0a:
            cache->sizekb = 8; cache->assoc = 2; cache->line = 32;
            cache->level = 1;
            break;
        case 0x0c:
            cache->sizekb = 16; cache->assoc = 4; cache->line = 32;
            cache->level = 1;
            break;
        case 0x0d:
            cache->sizekb = 16; cache->assoc = 4; cache->line = 64;
            cache->level = 1;
            break;
        case 0x0e:
            cache->sizekb = 24; cache->assoc = 6; cache->line = 64;
            cache->level = 1;
            break;
        case 0x21:
            cache->sizekb = 256; cache->assoc = 8; cache->line = 64;
            cache->level = 2;
            break;
        case 0x24:
            cache->sizekb = 1024; cache->assoc = 16; cache->line = 64;
            cache->level = 2;
            break;
        case 0x2c:
            cache->sizekb = 32; cache->assoc = 8; cache->line = 64;
            cache->level = 1;
            break;
        case 0x39:
            cache->sizekb = 128; cache->assoc = 4; cache->line = 64;
            cache->level = 2;
            break;
        case 0x3a:
            cache->sizekb = 192; cache->assoc = 6; cache->line = 64;
            cache->level = 2;
            break;
        case 0x3b:
            cache->sizekb = 128; cache->assoc = 2; cache->line = 64;
            cache->level = 2;
            break;
        case 0x3c:
            cache->sizekb = 256; cache->assoc = 4; cache->line = 64;
            cache->level = 2;
            break;
        case 0x3d:
            cache->sizekb = 384; cache->assoc = 6; cache->line = 64;
            cache->level = 2;
            break;
        case 0x3e:
            cache->sizekb = 512; cache->assoc = 4; cache->line = 64;
            cache->level = 2;
            break;
        case 0x41:
            cache->sizekb = 128; cache->assoc = 4; cache->line = 32;
            cache->level = 2;
            break;
        case 0x42:
            cache->sizekb = 256; cache->assoc = 4; cache->line = 32;
            cache->level = 2;
            break;
        case 0x43:
            cache->sizekb = 512; cache->assoc = 4; cache->line = 32;
            cache->level = 2;
            break;
        case 0x44:
            cache->sizekb = 1024; cache->assoc = 4; cache->line = 32;
            cache->level = 2;
            break;
        case 0x45:
            cache->sizekb = 2048; cache->assoc = 4; cache->line = 32;
            cache->level = 2;
            break;
        case 0x48:
            cache->sizekb = 3072; cache->assoc = 12; cache->line = 64;
            cache->level = 2;
            break;
        case 0x49:
            if (xeon_mp) {
                /* Break as in original code - don't populate */
                printf("Descriptor 0x49: Xeon MP detected, skipping\n");
                return;
            }
            cache->sizekb = 4096; cache->assoc = 16; cache->line = 64;
            cache->level = 2;
            break;
        case 0x4e:
            cache->sizekb = 6144; cache->assoc = 24; cache->line = 64;
            cache->level = 2;
            break;
        case 0x60:
            cache->sizekb = 16; cache->assoc = 8; cache->line = 64;
            cache->level = 1;
            break;
        case 0x66:
            cache->sizekb = 8; cache->assoc = 4; cache->line = 64;
            cache->level = 1;
            break;
        case 0x67:
            cache->sizekb = 16; cache->assoc = 4; cache->line = 64;
            cache->level = 1;
            break;
        case 0x68:
            cache->sizekb = 32; cache->assoc = 4; cache->line = 64;
            cache->level = 1;
            break;
        case 0x78:
            cache->sizekb = 1024; cache->assoc = 4; cache->line = 64;
            cache->level = 2;
            break;
        case 0x79:
            cache->sizekb = 128; cache->assoc = 8; cache->line = 64;
            cache->level = 2;
            break;
        case 0x7a:
            cache->sizekb = 256; cache->assoc = 8; cache->line = 64;
            cache->level = 2;
            break;
        case 0x7b:
            cache->sizekb = 512; cache->assoc = 8; cache->line = 64;
            cache->level = 2;
            break;
        case 0x7c:
            cache->sizekb = 1024; cache->assoc = 8; cache->line = 64;
            cache->level = 2;
            break;
        case 0x7d:
            cache->sizekb = 2048; cache->assoc = 8; cache->line = 64;
            cache->level = 2;
            break;
        case 0x7f:
            cache->sizekb = 512; cache->assoc = 2; cache->line = 64;
            cache->level = 2;
            break;
        case 0x80:
            cache->sizekb = 512; cache->assoc = 8; cache->line = 64;
            cache->level = 2;
            break;
        case 0x82:
            cache->sizekb = 256; cache->assoc = 8; cache->line = 32;
            cache->level = 2;
            break;
        case 0x83:
            cache->sizekb = 512; cache->assoc = 8; cache->line = 32;
            cache->level = 2;
            break;
        case 0x84:
            cache->sizekb = 1024; cache->assoc = 8; cache->line = 32;
            cache->level = 2;
            break;
        case 0x85:
            cache->sizekb = 2048; cache->assoc = 8; cache->line = 32;
            cache->level = 2;
            break;
        case 0x86:
            cache->sizekb = 512; cache->assoc = 4; cache->line = 64;
            cache->level = 2;
            break;
        case 0x87:
            cache->sizekb = 1024; cache->assoc = 8; cache->line = 64;
            cache->level = 2;
            break;
        default:
            /* Not one of our target descriptors */
            return;
    }
}

/* Extract descriptor bytes from CPUID leaf 0x2 results */
static void process_cpuid_leaf2(struct cache_desc *caches, int *cache_count, int xeon_mp) {
    uint32_t info[4];
    uint8_t descriptors[16];
    int iteration = 0;
    int max_iterations = 10; /* Safety limit */
    
    /* According to Intel spec, CPUID leaf 0x2 may need to be called multiple times */
    while (iteration < max_iterations) {
        cpuid(info, 0x00000002);
        
        /* Extract descriptor bytes from EAX, EBX, ECX, EDX */
        descriptors[0] = (info[0] >> 0) & 0xFF;
        descriptors[1] = (info[0] >> 8) & 0xFF;
        descriptors[2] = (info[0] >> 16) & 0xFF;
        descriptors[3] = (info[0] >> 24) & 0xFF;
        
        descriptors[4] = (info[1] >> 0) & 0xFF;
        descriptors[5] = (info[1] >> 8) & 0xFF;
        descriptors[6] = (info[1] >> 16) & 0xFF;
        descriptors[7] = (info[1] >> 24) & 0xFF;
        
        descriptors[8] = (info[2] >> 0) & 0xFF;
        descriptors[9] = (info[2] >> 8) & 0xFF;
        descriptors[10] = (info[2] >> 16) & 0xFF;
        descriptors[11] = (info[2] >> 24) & 0xFF;
        
        descriptors[12] = (info[3] >> 0) & 0xFF;
        descriptors[13] = (info[3] >> 8) & 0xFF;
        descriptors[14] = (info[3] >> 16) & 0xFF;
        descriptors[15] = (info[3] >> 24) & 0xFF;
        
        /* Process each descriptor byte */
        for (int i = 0; i < 16; i++) {
            uint8_t desc = descriptors[i];
            
            /* Skip null descriptors and invalid ones */
            if (desc == 0x00) {
                continue;
            }
            
            /* Check if this is a valid cache descriptor */
            if ((desc & 0x80) == 0) { /* Bit 7 = 0 indicates valid cache descriptor */
                /* Parse and store if it matches our target cases */
                parse_cache_descriptor(desc, &caches[*cache_count], xeon_mp);
                if (caches[*cache_count].sizekb > 0) {
                    (*cache_count)++;
                }
            }
            
            /* Safety check to avoid buffer overflow */
            if (*cache_count >= 32) {
                return;
            }
        }
        
        /* Check if we should continue (AL register bit 7 indicates if leaf 2 should be called again) */
        if ((info[0] & 0x80) == 0) {
            break;
        }
        
        iteration++;
    }
}

/* Demonstrate cache-aware computation using detected cache line size */
static void cache_aware_computation(struct cache_desc *caches, int cache_count) {
    if (cache_count == 0) {
        return;
    }
    
    /* Find the smallest cache line size */
    int min_line_size = 64; /* Default */
    for (int i = 0; i < cache_count; i++) {
        if (caches[i].line > 0 && caches[i].line < min_line_size) {
            min_line_size = caches[i].line;
        }
    }
    
    /* Allocate aligned memory based on cache line size */
    size_t array_size = 1024 * 1024; /* 1MB */
    char *buffer = (char*)aligned_alloc(min_line_size, array_size);
    if (!buffer) {
        return;
    }
    
    /* Perform computation that benefits from cache alignment */
    volatile int sum = 0;
    for (size_t i = 0; i < array_size; i += min_line_size) {
        buffer[i] = (char)(i % 256);
        sum += buffer[i];
    }
    
    /* Use sum to prevent optimization */
    printf("Cache-aware computation result: %d (using line size %d)\n", sum, min_line_size);
    
    free(buffer);
}

int main(void) {
    struct cache_desc caches[32];
    int cache_count = 0;
    
    printf("Starting CPUID-based cache detection...\n");
    
    /* Step 1: Get CPU info and determine Xeon MP status */
    int xeon_mp = is_xeon_mp();
    printf("Xeon MP simulation: %s\n", xeon_mp ? "true" : "false");
    
    /* Step 2: Call CPUID leaf 0x2 and parse cache descriptors */
    process_cpuid_leaf2(caches, &cache_count, xeon_mp);
    
    /* Step 3: Print detected cache information */
    printf("\nDetected %d cache configurations:\n", cache_count);
    for (int i = 0; i < cache_count; i++) {
        printf("Cache %d: Descriptor 0x%02x, Level %d, Size %d KB, "
               "Assoc %d-way, Line %d bytes\n",
               i, caches[i].desc, caches[i].level, caches[i].sizekb,
               caches[i].assoc, caches[i].line);
    }
    
    /* Step 4: Perform cache-aware computation */
    cache_aware_computation(caches, cache_count);
    
    /* Step 5: Additional test to ensure all target descriptors are considered */
    printf("\nTesting all target descriptor values:\n");
    uint8_t target_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    struct cache_desc test_cache;
    for (size_t i = 0; i < sizeof(target_descriptors)/sizeof(target_descriptors[0]); i++) {
        parse_cache_descriptor(target_descriptors[i], &test_cache, xeon_mp);
        if (test_cache.sizekb > 0) {
            printf("  Descriptor 0x%02x maps to: L%d %dKB %d-way %dB line\n",
                   target_descriptors[i], test_cache.level, test_cache.sizekb,
                   test_cache.assoc, test_cache.line);
        }
    }
    
    return 0;
}
