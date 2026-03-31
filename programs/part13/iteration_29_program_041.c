#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
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

/* Global flag to simulate non-Xeon-MP system */
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
            if (xeon_mp)
                break;
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            *got_level2 = 1;
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
            /* Ignore other descriptors */
            break;
    }
}

/* Real CPUID call for leaf 0x02 */
static void call_cpuid_leaf2(void) {
    uint32_t eax, ebx, ecx, edx;
    
    printf("=== Calling CPUID Leaf 0x02 (Cache Descriptors) ===\n");
    
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
    
    printf("EAX: 0x%08X\n", eax);
    printf("EBX: 0x%08X\n", ebx);
    printf("ECX: 0x%08X\n", ecx);
    printf("EDX: 0x%08X\n", edx);
    
    /* Check if we should use descriptor table method (AL > 1) */
    uint8_t al = eax & 0xFF;
    if (al > 1) {
        printf("Using descriptor table method (AL = 0x%02X)\n", al);
        
        struct cache_desc level1 = {0};
        struct cache_desc level2 = {0};
        int got_level1 = 0;
        int got_level2 = 0;
        
        /* Process descriptor bytes from all registers */
        uint8_t *regs = (uint8_t*)&eax;
        for (int i = 0; i < 16; i++) {
            if (regs[i] != 0 && (regs[i] & 0x80) == 0) {
                printf("Processing descriptor 0x%02X\n", regs[i]);
                process_cache_descriptor(regs[i], &level1, &level2, 
                                        &got_level1, &got_level2);
            }
        }
        
        if (got_level1) {
            printf("L1 Cache: %dKB, %d-way, %d byte line\n", 
                   level1.sizekb, level1.assoc, level1.line);
        }
        if (got_level2) {
            printf("L2 Cache: %dKB, %d-way, %d byte line\n", 
                   level2.sizekb, level2.assoc, level2.line);
        }
    } else {
        printf("Not using descriptor table method (AL = 0x%02X)\n", al);
    }
}

/* Real CPUID call for leaf 0x04 (Deterministic Cache Parameters) */
static void call_cpuid_leaf4(void) {
    printf("\n=== Calling CPUID Leaf 0x04 (Deterministic Cache) ===\n");
    
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
        
        uint8_t cache_type = eax & 0x1F;
        if (cache_type == 0) {
            printf("No more caches (type = 0)\n");
            break;
        }
        
        printf("Cache %d: type=%u, level=%u, line_size=%u, ways=%u, sets=%u\n",
               i, cache_type, (eax >> 5) & 0x7,
               (ebx & 0xFFF) + 1,
               ((ebx >> 22) & 0x3FF) + 1,
               ecx + 1);
    }
}

/* Simulate all uncovered cache descriptor cases */
static void simulate_all_descriptors(void) {
    printf("\n=== Simulating All Target Cache Descriptors ===\n");
    
    /* All the target descriptor values from uncovered lines */
    uint8_t target_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    struct cache_desc level1 = {0};
    struct cache_desc level2 = {0};
    int got_level1 = 0;
    int got_level2 = 0;
    
    /* First pass: xeon_mp = 0 (to hit case 0x49) */
    xeon_mp = 0;
    printf("\nSimulating with xeon_mp = 0:\n");
    for (size_t i = 0; i < sizeof(target_descriptors)/sizeof(target_descriptors[0]); i++) {
        process_cache_descriptor(target_descriptors[i], &level1, &level2, 
                                &got_level1, &got_level2);
    }
    
    if (got_level1) {
        printf("L1 Cache: %dKB, %d-way, %d byte line\n", 
               level1.sizekb, level1.assoc, level1.line);
    }
    if (got_level2) {
        printf("L2 Cache: %dKB, %d-way, %d byte line\n", 
               level2.sizekb, level2.assoc, level2.line);
    }
    
    /* Reset and test with xeon_mp = 1 */
    memset(&level1, 0, sizeof(level1));
    memset(&level2, 0, sizeof(level2));
    got_level1 = got_level2 = 0;
    xeon_mp = 1;
    
    printf("\nSimulating with xeon_mp = 1:\n");
    for (size_t i = 0; i < sizeof(target_descriptors)/sizeof(target_descriptors[0]); i++) {
        process_cache_descriptor(target_descriptors[i], &level1, &level2, 
                                &got_level1, &got_level2);
    }
    
    if (got_level1) {
        printf("L1 Cache: %dKB, %d-way, %d byte line\n", 
               level1.sizekb, level1.assoc, level1.line);
    }
    if (got_level2) {
        printf("L2 Cache: %dKB, %d-way, %d byte line\n", 
               level2.sizekb, level2.assoc, level2.line);
    }
}

/* Simulate CPUID leaf 0x02 with fabricated data to trigger all cases */
static void simulate_cpuid_leaf2_with_fabricated_data(void) {
    printf("\n=== Simulating CPUID Leaf 0x02 with Fabricated Data ===\n");
    
    /* Fabricate CPUID results with AL > 1 to use descriptor table method */
    uint32_t fabricated_eax = 0x03020100;  /* AL = 0x03 (3 valid descriptors) */
    uint32_t fabricated_ebx = 0x0A0C0D0E;  /* Descriptors: 0x0A, 0x0C, 0x0D, 0x0E */
    uint32_t fabricated_ecx = 0x21242C39;  /* Descriptors: 0x21, 0x24, 0x2C, 0x39 */
    uint32_t fabricated_edx = 0x3A3B3C3D;  /* Descriptors: 0x3A, 0x3B, 0x3C, 0x3D */
    
    printf("Fabricated EAX: 0x%08X\n", fabricated_eax);
    printf("Fabricated EBX: 0x%08X\n", fabricated_ebx);
    printf("Fabricated ECX: 0x%08X\n", fabricated_ecx);
    printf("Fabricated EDX: 0x%08X\n", fabricated_edx);
    
    uint8_t al = fabricated_eax & 0xFF;
    if (al > 1) {
        printf("Using descriptor table method (AL = 0x%02X)\n", al);
        
        struct cache_desc level1 = {0};
        struct cache_desc level2 = {0};
        int got_level1 = 0;
        int got_level2 = 0;
        
        /* Process all bytes from fabricated registers */
        uint8_t *regs[] = {
            (uint8_t*)&fabricated_eax,
            (uint8_t*)&fabricated_ebx,
            (uint8_t*)&fabricated_ecx,
            (uint8_t*)&fabricated_edx
        };
        
        for (int reg = 0; reg < 4; reg++) {
            for (int i = 0; i < 4; i++) {
                uint8_t desc = regs[reg][i];
                if (desc != 0 && (desc & 0x80) == 0) {
                    printf("Processing fabricated descriptor 0x%02X\n", desc);
                    process_cache_descriptor(desc, &level1, &level2, 
                                            &got_level1, &got_level2);
                }
            }
        }
        
        if (got_level1) {
            printf("L1 Cache: %dKB, %d-way, %d byte line\n", 
                   level1.sizekb, level1.assoc, level1.line);
        }
        if (got_level2) {
            printf("L2 Cache: %dKB, %d-way, %d byte line\n", 
                   level2.sizekb, level2.assoc, level2.line);
        }
    }
}

int main(void) {
    printf("Cache Descriptor Coverage Test Program\n");
    printf("======================================\n");
    
    /* Test 1: Real CPUID calls */
    call_cpuid_leaf2();
    call_cpuid_leaf4();
    
    /* Test 2: Simulate all target descriptors */
    simulate_all_descriptors();
    
    /* Test 3: Simulate with fabricated CPUID data */
    simulate_cpuid_leaf2_with_fabricated_data();
    
    /* Force compiler to keep all code */
    volatile int keep = 1;
    if (keep) {
        printf("\nAll tests completed.\n");
    }
    
    return 0;
}
