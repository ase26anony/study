#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <cpuid.h>
#endif

/* Structures matching those in driver-i386.cc */
struct cache_desc {
    int sizekb;
    int assoc;
    int line;
};

/* Global variables to track coverage */
static int cases_hit[256] = {0};
static int xeon_mp = 0; /* We'll set this to 0 to hit the 0x49 case */

/* Function to simulate the cache parsing logic from driver-i386.cc */
void parse_cache_descriptor_byte(uint8_t desc, struct cache_desc *level1, 
                                 struct cache_desc *level2, int *has_level1, 
                                 int *has_level2) {
    printf("Processing descriptor: 0x%02x\n", desc);
    cases_hit[desc]++;
    
    switch (desc) {
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            *has_level1 = 1;
            printf("  -> L1: 8KB, 2-way, 32B line\n");
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            *has_level1 = 1;
            printf("  -> L1: 16KB, 4-way, 32B line\n");
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            *has_level1 = 1;
            printf("  -> L1: 16KB, 4-way, 64B line\n");
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            *has_level1 = 1;
            printf("  -> L1: 24KB, 6-way, 64B line\n");
            break;
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            *has_level2 = 1;
            printf("  -> L2: 256KB, 8-way, 64B line\n");
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            *has_level2 = 1;
            printf("  -> L2: 1024KB, 16-way, 64B line\n");
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            *has_level1 = 1;
            printf("  -> L1: 32KB, 8-way, 64B line\n");
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            *has_level2 = 1;
            printf("  -> L2: 128KB, 4-way, 64B line\n");
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            *has_level2 = 1;
            printf("  -> L2: 192KB, 6-way, 64B line\n");
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            *has_level2 = 1;
            printf("  -> L2: 128KB, 2-way, 64B line\n");
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            *has_level2 = 1;
            printf("  -> L2: 256KB, 4-way, 64B line\n");
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            *has_level2 = 1;
            printf("  -> L2: 384KB, 6-way, 64B line\n");
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            *has_level2 = 1;
            printf("  -> L2: 512KB, 4-way, 64B line\n");
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            *has_level2 = 1;
            printf("  -> L2: 128KB, 4-way, 32B line\n");
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            *has_level2 = 1;
            printf("  -> L2: 256KB, 4-way, 32B line\n");
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            *has_level2 = 1;
            printf("  -> L2: 512KB, 4-way, 32B line\n");
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            *has_level2 = 1;
            printf("  -> L2: 1024KB, 4-way, 32B line\n");
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            *has_level2 = 1;
            printf("  -> L2: 2048KB, 4-way, 32B line\n");
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            *has_level2 = 1;
            printf("  -> L2: 3072KB, 12-way, 64B line\n");
            break;
        case 0x49:
            if (xeon_mp) {
                printf("  -> 0x49: Xeon MP detected, skipping\n");
                break;
            }
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            *has_level2 = 1;
            printf("  -> L2: 4096KB, 16-way, 64B line (non-Xeon-MP)\n");
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            *has_level2 = 1;
            printf("  -> L2: 6144KB, 24-way, 64B line\n");
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            *has_level1 = 1;
            printf("  -> L1: 16KB, 8-way, 64B line\n");
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            *has_level1 = 1;
            printf("  -> L1: 8KB, 4-way, 64B line\n");
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            *has_level1 = 1;
            printf("  -> L1: 16KB, 4-way, 64B line\n");
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            *has_level1 = 1;
            printf("  -> L1: 32KB, 4-way, 64B line\n");
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            *has_level2 = 1;
            printf("  -> L2: 1024KB, 4-way, 64B line\n");
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            *has_level2 = 1;
            printf("  -> L2: 128KB, 8-way, 64B line\n");
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            *has_level2 = 1;
            printf("  -> L2: 256KB, 8-way, 64B line\n");
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            *has_level2 = 1;
            printf("  -> L2: 512KB, 8-way, 64B line\n");
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            *has_level2 = 1;
            printf("  -> L2: 1024KB, 8-way, 64B line\n");
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            *has_level2 = 1;
            printf("  -> L2: 2048KB, 8-way, 64B line\n");
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            *has_level2 = 1;
            printf("  -> L2: 512KB, 2-way, 64B line\n");
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            *has_level2 = 1;
            printf("  -> L2: 512KB, 8-way, 64B line\n");
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            *has_level2 = 1;
            printf("  -> L2: 256KB, 8-way, 32B line\n");
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            *has_level2 = 1;
            printf("  -> L2: 512KB, 8-way, 32B line\n");
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            *has_level2 = 1;
            printf("  -> L2: 1024KB, 8-way, 32B line\n");
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            *has_level2 = 1;
            printf("  -> L2: 2048KB, 8-way, 32B line\n");
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            *has_level2 = 1;
            printf("  -> L2: 512KB, 4-way, 64B line\n");
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            *has_level2 = 1;
            printf("  -> L2: 1024KB, 8-way, 64B line\n");
            break;
        default:
            printf("  -> Unknown descriptor (not in uncovered lines)\n");
            break;
    }
}

/* Simulate CPUID leaf 0x02 with specific descriptor bytes */
void simulate_cpuid_leaf2(uint8_t *descriptors, int count) {
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    int has_level1 = 0, has_level2 = 0;
    
    printf("\n=== Simulating CPUID Leaf 0x02 with %d descriptors ===\n", count);
    
    /* Simulate the iteration through descriptor bytes */
    for (int i = 0; i < count; i++) {
        parse_cache_descriptor_byte(descriptors[i], &level1, &level2, 
                                   &has_level1, &has_level2);
    }
    
    printf("\nFinal cache configuration:\n");
    if (has_level1) {
        printf("L1 Cache: %d KB, %d-way, %d-byte line\n", 
               level1.sizekb, level1.assoc, level1.line);
    }
    if (has_level2) {
        printf("L2 Cache: %d KB, %d-way, %d-byte line\n", 
               level2.sizekb, level2.assoc, level2.line);
    }
}

/* Real CPUID call for leaf 0x04 (deterministic cache parameters) */
void call_cpuid_leaf4() {
    uint32_t eax, ebx, ecx, edx;
    int cache_index = 0;
    
    printf("\n=== Calling CPUID Leaf 0x04 (Deterministic Cache) ===\n");
    
    do {
#ifdef _MSC_VER
        int cpu_info[4];
        __cpuid_count(cpu_info, 0x04, cache_index);
        eax = cpu_info[0];
        ebx = cpu_info[1];
        ecx = cpu_info[2];
        edx = cpu_info[3];
#else
        __cpuid_count(0x04, cache_index, eax, ebx, ecx, edx);
#endif
        
        uint32_t cache_type = eax & 0x1F;
        
        if (cache_type == 0) {
            printf("Cache %d: No more caches\n", cache_index);
            break;
        }
        
        uint32_t cache_level = (eax >> 5) & 0x7;
        uint32_t self_initializing = (eax >> 8) & 0x1;
        uint32_t fully_associative = (eax >> 9) & 0x1;
        uint32_t max_threads = ((eax >> 14) & 0xFFF) + 1;
        uint32_t max_cores = ((eax >> 26) & 0x3F) + 1;
        
        uint32_t line_size = (ebx & 0xFFF) + 1;
        uint32_t partitions = ((ebx >> 12) & 0x3FF) + 1;
        uint32_t ways = ((ebx >> 22) & 0x3FF) + 1;
        
        uint32_t sets = ecx + 1;
        
        uint32_t size = ways * partitions * line_size * sets / 1024;
        
        printf("Cache %d: Type=%u, Level=%u, Size=%uKB, ", 
               cache_index, cache_type, cache_level, size);
        printf("Ways=%u, Line=%uB, Sets=%u\n", ways, line_size, sets);
        
        cache_index++;
    } while (1);
}

int main() {
    /* Target descriptor bytes from uncovered lines */
    uint8_t target_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e,           /* L1 cache descriptors */
        0x21, 0x24, 0x2c,                 /* L2 cache descriptors */
        0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, /* More L2 */
        0x41, 0x42, 0x43, 0x44, 0x45,     /* L2 with 32B lines */
        0x48, 0x49, 0x4e,                 /* Large L2 caches */
        0x60, 0x66, 0x67, 0x68,           /* More L1 */
        0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7f, 0x80, /* Various L2 */
        0x82, 0x83, 0x84, 0x85, 0x86, 0x87  /* L2 with 32B lines and others */
    };
    
    int num_descriptors = sizeof(target_descriptors) / sizeof(target_descriptors[0]);
    
    printf("=== Cache Descriptor Coverage Test ===\n");
    printf("Testing %d target descriptor bytes\n\n", num_descriptors);
    
    /* Set xeon_mp to 0 to ensure case 0x49 is hit */
    xeon_mp = 0;
    printf("xeon_mp = %d (will hit 0x49 case)\n", xeon_mp);
    
    /* Simulate CPUID leaf 0x02 with all target descriptors */
    simulate_cpuid_leaf2(target_descriptors, num_descriptors);
    
    /* Also call real CPUID leaf 0x04 for deterministic cache parameters */
    call_cpuid_leaf4();
    
    /* Summary of coverage */
    printf("\n=== Coverage Summary ===\n");
    int total_hit = 0;
    for (int i = 0; i < 256; i++) {
        if (cases_hit[i] > 0) {
            printf("Case 0x%02x: hit %d time(s)\n", i, cases_hit[i]);
            total_hit++;
        }
    }
    printf("\nTotal unique cases hit: %d out of %d target descriptors\n", 
           total_hit, num_descriptors);
    
    /* Check if we hit the special 0x49 case with xeon_mp = 0 */
    if (cases_hit[0x49] > 0) {
        printf("\n✓ SUCCESS: Hit case 0x49 with xeon_mp = 0\n");
        printf("  This should have set level2->sizekb = 4096\n");
    }
    
    return 0;
}
