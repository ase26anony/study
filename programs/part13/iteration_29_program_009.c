#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <cpuid.h>
#endif

/* Structure matching cache_desc from driver-i386.cc */
struct cache_desc {
    int sizekb;
    int assoc;
    int line;
};

/* Global flag to simulate xeon_mp condition */
static int xeon_mp = 0;

/* Function to process cache descriptor bytes - mimics the uncovered logic */
static void process_cache_descriptor(uint8_t desc, struct cache_desc *level1, 
                                     struct cache_desc *level2, int *got_level1, 
                                     int *got_level2) {
    switch (desc) {
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            *got_level1 = 1;
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            *got_level1 = 1;
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            *got_level1 = 1;
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            *got_level1 = 1;
            break;
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            *got_level1 = 1;
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            *got_level2 = 1;
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            *got_level2 = 1;
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            *got_level2 = 1;
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            *got_level2 = 1;
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            *got_level2 = 1;
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x49:
            if (!xeon_mp) {
                level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
                *got_level2 = 1;
            }
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            *got_level1 = 1;
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            *got_level1 = 1;
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            *got_level1 = 1;
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            *got_level1 = 1;
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            *got_level2 = 1;
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            *got_level2 = 1;
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            *got_level2 = 1;
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            *got_level2 = 1;
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            *got_level2 = 1;
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            *got_level2 = 1;
            break;
        default:
            /* Other descriptors not in our uncovered lines */
            break;
    }
}

/* Simulate CPUID leaf 0x02 with specific descriptor bytes */
static void simulate_cpuid_leaf2(uint8_t *descriptors, int count) {
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    int got_level1 = 0;
    int got_level2 = 0;
    
    printf("Processing %d cache descriptors:\n", count);
    
    for (int i = 0; i < count; i++) {
        uint8_t desc = descriptors[i];
        
        /* Skip null descriptors and TLB descriptors (0x00, 0xff) */
        if (desc == 0x00 || desc == 0xff) {
            continue;
        }
        
        printf("  Descriptor 0x%02x: ", desc);
        process_cache_descriptor(desc, &level1, &level2, &got_level1, &got_level2);
        
        if (got_level1 && level1.sizekb > 0) {
            printf("L1 Cache: %dKB, %d-way, %d-byte line\n", 
                   level1.sizekb, level1.assoc, level1.line);
            got_level1 = 0; /* Reset for next descriptor */
        } else if (got_level2 && level2.sizekb > 0) {
            printf("L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   level2.sizekb, level2.assoc, level2.line);
            got_level2 = 0; /* Reset for next descriptor */
        } else {
            printf("(not in uncovered cases)\n");
        }
    }
}

/* Real CPUID call for leaf 0x02 */
static void call_real_cpuid_leaf2(void) {
    uint32_t eax, ebx, ecx, edx;
    
    printf("\n=== Real CPUID Leaf 0x02 ===\n");
    
#ifdef _MSC_VER
    int cpuInfo[4];
    __cpuid(cpuInfo, 0x02);
    eax = cpuInfo[0];
    ebx = cpuInfo[1];
    ecx = cpuInfo[2];
    edx = cpuInfo[3];
#else
    __cpuid(0x02, eax, ebx, ecx, edx);
#endif
    
    printf("EAX: 0x%08x\n", eax);
    printf("EBX: 0x%08x\n", ebx);
    printf("ECX: 0x%08x\n", ecx);
    printf("EDX: 0x%08x\n", edx);
    
    /* Check if AL > 1 (valid descriptor table) */
    uint8_t al = eax & 0xFF;
    if (al > 1) {
        printf("Valid descriptor count in AL: %d\n", al);
        
        /* Extract descriptor bytes */
        uint8_t descriptors[16];
        descriptors[0] = (eax >> 8) & 0xFF;
        descriptors[1] = (eax >> 16) & 0xFF;
        descriptors[2] = (eax >> 24) & 0xFF;
        descriptors[3] = ebx & 0xFF;
        descriptors[4] = (ebx >> 8) & 0xFF;
        descriptors[5] = (ebx >> 16) & 0xFF;
        descriptors[6] = (ebx >> 24) & 0xFF;
        descriptors[7] = ecx & 0xFF;
        descriptors[8] = (ecx >> 8) & 0xFF;
        descriptors[9] = (ecx >> 16) & 0xFF;
        descriptors[10] = (ecx >> 24) & 0xFF;
        descriptors[11] = edx & 0xFF;
        descriptors[12] = (edx >> 8) & 0xFF;
        descriptors[13] = (edx >> 16) & 0xFF;
        descriptors[14] = (edx >> 24) & 0xFF;
        
        simulate_cpuid_leaf2(descriptors, al);
    } else {
        printf("AL = %d, not using descriptor table method\n", al);
    }
}

/* Real CPUID call for leaf 0x04 */
static void call_real_cpuid_leaf4(void) {
    printf("\n=== Real CPUID Leaf 0x04 ===\n");
    
    for (int i = 0; ; i++) {
        uint32_t eax, ebx, ecx, edx;
        
#ifdef _MSC_VER
        int cpuInfo[4];
        __cpuidex(cpuInfo, 0x04, i);
        eax = cpuInfo[0];
        ebx = cpuInfo[1];
        ecx = cpuInfo[2];
        edx = cpuInfo[3];
#else
        __cpuid_count(0x04, i, eax, ebx, ecx, edx);
#endif
        
        int cache_type = eax & 0x1F;
        if (cache_type == 0) {
            printf("No more caches at index %d\n", i);
            break;
        }
        
        printf("Cache %d: type=%d, level=%d, line_size=%d, ways=%d, sets=%d, size=%dKB\n",
               i, cache_type, (eax >> 5) & 0x7,
               (ebx & 0xFFF) + 1,
               ((ebx >> 22) & 0x3FF) + 1,
               ecx + 1,
               (((ebx >> 22) & 0x3FF) + 1) * 
               ((ebx & 0xFFF) + 1) * 
               (ecx + 1) / 1024);
    }
}

/* Test all uncovered descriptor cases with simulation */
static void test_all_uncovered_cases(void) {
    /* All the descriptor values from uncovered lines */
    uint8_t all_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    printf("\n=== Testing All Uncovered Descriptor Cases ===\n");
    
    /* First test with xeon_mp = 0 (to hit case 0x49) */
    xeon_mp = 0;
    printf("\nWith xeon_mp = 0 (should process case 0x49):\n");
    simulate_cpuid_leaf2(all_descriptors, sizeof(all_descriptors));
    
    /* Then test with xeon_mp = 1 (to skip case 0x49) */
    xeon_mp = 1;
    printf("\nWith xeon_mp = 1 (should skip case 0x49):\n");
    simulate_cpuid_leaf2(all_descriptors, sizeof(all_descriptors));
}

/* Create a simulated CPUID result that forces execution into descriptor table parsing */
static void test_specific_cpuid_scenario(void) {
    printf("\n=== Testing Specific CPUID Scenario ===\n");
    
    /* Create a simulated CPUID leaf 0x02 result where:
       - AL = 0x06 (6 valid descriptor bytes, >1 to avoid early return)
       - Descriptor bytes include multiple uncovered cases
    */
    uint8_t simulated_descriptors[] = {
        0x0a,  /* L1: 8KB, 2-way, 32B */
        0x0c,  /* L1: 16KB, 4-way, 32B */
        0x21,  /* L2: 256KB, 8-way, 64B */
        0x49,  /* L2: 4096KB, 16-way, 64B (if !xeon_mp) */
        0x78,  /* L2: 1024KB, 4-way, 64B */
        0x87   /* L2: 1024KB, 8-way, 64B */
    };
    
    xeon_mp = 0;  /* Ensure case 0x49 is processed */
    simulate_cpuid_leaf2(simulated_descriptors, 6);
}

int main(void) {
    printf("Cache Descriptor Coverage Test Program\n");
    printf("======================================\n");
    
    /* Test 1: Simulate all uncovered cases */
    test_all_uncovered_cases();
    
    /* Test 2: Specific scenario that would trigger the parsing logic */
    test_specific_cpuid_scenario();
    
    /* Test 3: Try real CPUID calls (if supported) */
    printf("\n=== Attempting Real CPUID Calls ===\n");
    
    /* Check if CPUID is supported */
    uint32_t max_leaf;
#ifdef _MSC_VER
    int cpuInfo[4];
    __cpuid(cpuInfo, 0);
    max_leaf = cpuInfo[0];
#else
    uint32_t ebx, ecx, edx;
    __cpuid(0, max_leaf, ebx, ecx, edx);
#endif
    
    printf("Maximum CPUID leaf: 0x%08x\n", max_leaf);
    
    if (max_leaf >= 0x02) {
        call_real_cpuid_leaf2();
    } else {
        printf("CPUID leaf 0x02 not supported\n");
    }
    
    if (max_leaf >= 0x04) {
        call_real_cpuid_leaf4();
    } else {
        printf("CPUID leaf 0x04 not supported\n");
    }
    
    return 0;
}
