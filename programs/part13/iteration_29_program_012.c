#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <cpuid.h>
#endif

/* Structure matching cache_desc in driver-i386.cc */
struct cache_desc {
    int sizekb;
    int assoc;
    int line;
};

/* Global flag to simulate xeon_mp condition */
static int xeon_mp = 0;

/* Function to process cache descriptors - similar to driver-i386.cc logic */
static void process_cache_descriptor(uint8_t desc, struct cache_desc *level1, 
                                     struct cache_desc *level2, int *has_level1, 
                                     int *has_level2) {
    switch (desc) {
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            *has_level1 = 1;
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            *has_level1 = 1;
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            *has_level1 = 1;
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            *has_level1 = 1;
            break;
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            *has_level1 = 1;
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            *has_level2 = 1;
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            *has_level2 = 1;
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            *has_level2 = 1;
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            *has_level2 = 1;
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            *has_level2 = 1;
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x49:
            if (!xeon_mp) {
                level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
                *has_level2 = 1;
            }
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            *has_level1 = 1;
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            *has_level1 = 1;
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            *has_level1 = 1;
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            *has_level1 = 1;
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            *has_level2 = 1;
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            *has_level2 = 1;
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            *has_level2 = 1;
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            *has_level2 = 1;
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            *has_level2 = 1;
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            *has_level2 = 1;
            break;
        default:
            /* Unknown descriptor */
            break;
    }
}

/* Real CPUID call for leaf 0x02 */
static void call_cpuid_leaf2(void) {
    uint32_t eax, ebx, ecx, edx;
    
    printf("=== Calling CPUID Leaf 0x02 ===\n");
    
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
    
    printf("EAX: 0x%08X, EBX: 0x%08X, ECX: 0x%08X, EDX: 0x%08X\n", 
           eax, ebx, ecx, edx);
    
    /* Check if AL > 1 (valid descriptor table) */
    uint8_t al = eax & 0xFF;
    if (al > 1) {
        printf("Valid descriptor table with %d bytes\n", al);
        
        struct cache_desc level1 = {0, 0, 0};
        struct cache_desc level2 = {0, 0, 0};
        int has_level1 = 0, has_level2 = 0;
        
        /* Process descriptor bytes from registers */
        uint8_t *regs = (uint8_t*)&eax;
        for (int i = 0; i < 4; i++) {
            if (regs[i] != 0 && (regs[i] & 0x80) == 0) {
                printf("Processing descriptor 0x%02X\n", regs[i]);
                process_cache_descriptor(regs[i], &level1, &level2, 
                                        &has_level1, &has_level2);
            }
        }
        
        regs = (uint8_t*)&ebx;
        for (int i = 0; i < 4; i++) {
            if (regs[i] != 0 && (regs[i] & 0x80) == 0) {
                printf("Processing descriptor 0x%02X\n", regs[i]);
                process_cache_descriptor(regs[i], &level1, &level2, 
                                        &has_level1, &has_level2);
            }
        }
        
        regs = (uint8_t*)&ecx;
        for (int i = 0; i < 4; i++) {
            if (regs[i] != 0 && (regs[i] & 0x80) == 0) {
                printf("Processing descriptor 0x%02X\n", regs[i]);
                process_cache_descriptor(regs[i], &level1, &level2, 
                                        &has_level1, &has_level2);
            }
        }
        
        regs = (uint8_t*)&edx;
        for (int i = 0; i < 4; i++) {
            if (regs[i] != 0 && (regs[i] & 0x80) == 0) {
                printf("Processing descriptor 0x%02X\n", regs[i]);
                process_cache_descriptor(regs[i], &level1, &level2, 
                                        &has_level1, &has_level2);
            }
        }
        
        if (has_level1) {
            printf("L1 Cache: %d KB, %d-way, %d byte line\n", 
                   level1.sizekb, level1.assoc, level1.line);
        }
        if (has_level2) {
            printf("L2 Cache: %d KB, %d-way, %d byte line\n", 
                   level2.sizekb, level2.assoc, level2.line);
        }
    } else {
        printf("Not using descriptor table method (AL = %d)\n", al);
    }
}

/* Real CPUID call for leaf 0x04 */
static void call_cpuid_leaf4(void) {
    printf("\n=== Calling CPUID Leaf 0x04 ===\n");
    
    int cache_index = 0;
    while (1) {
        uint32_t eax, ebx, ecx, edx;
        
#ifdef _MSC_VER
        int cpuInfo[4];
        __cpuidex(cpuInfo, 0x04, cache_index);
        eax = cpuInfo[0];
        ebx = cpuInfo[1];
        ecx = cpuInfo[2];
        edx = cpuInfo[3];
#else
        __cpuid_count(0x04, cache_index, eax, ebx, ecx, edx);
#endif
        
        printf("Cache %d: EAX=0x%08X, EBX=0x%08X, ECX=0x%08X, EDX=0x%08X\n",
               cache_index, eax, ebx, ecx, edx);
        
        /* Check cache type field (bits 4:0) */
        uint32_t cache_type = eax & 0x1F;
        if (cache_type == 0) {
            printf("No more caches\n");
            break;
        }
        
        /* Extract cache information */
        uint32_t cache_level = (eax >> 5) & 0x7;
        uint32_t self_initializing = (eax >> 8) & 0x1;
        uint32_t fully_associative = (eax >> 9) & 0x1;
        uint32_t max_threads = ((eax >> 14) & 0xFFF) + 1;
        uint32_t max_cores = ((eax >> 26) & 0x3F) + 1;
        
        /* Ways of associativity */
        uint32_t ways = ((ebx >> 22) & 0x3FF) + 1;
        
        /* Physical line partitions */
        uint32_t partitions = (ebx >> 12) & 0x3FF;
        
        /* System coherency line size */
        uint32_t line_size = (ebx & 0xFFF) + 1;
        
        /* Number of sets */
        uint32_t sets = ecx + 1;
        
        /* Calculate cache size */
        uint32_t cache_size = ways * partitions * line_size * sets;
        
        printf("  Level: %d, Type: %d, Size: %u bytes\n", 
               cache_level, cache_type, cache_size);
        printf("  Ways: %u, Line Size: %u, Sets: %u\n", 
               ways, line_size, sets);
        
        cache_index++;
    }
}

/* Simulate all target descriptor cases */
static void simulate_all_descriptors(void) {
    printf("\n=== Simulating All Target Descriptors ===\n");
    
    /* All target descriptor values from uncovered lines */
    uint8_t target_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    int num_descriptors = sizeof(target_descriptors) / sizeof(target_descriptors[0]);
    
    /* Test with xeon_mp = 0 to hit case 0x49 */
    xeon_mp = 0;
    printf("Testing with xeon_mp = %d\n", xeon_mp);
    
    for (int i = 0; i < num_descriptors; i++) {
        struct cache_desc level1 = {0, 0, 0};
        struct cache_desc level2 = {0, 0, 0};
        int has_level1 = 0, has_level2 = 0;
        
        printf("\nDescriptor 0x%02X: ", target_descriptors[i]);
        process_cache_descriptor(target_descriptors[i], &level1, &level2, 
                                &has_level1, &has_level2);
        
        if (has_level1) {
            printf("L1: %dKB %d-way %dB line", 
                   level1.sizekb, level1.assoc, level1.line);
        }
        if (has_level2) {
            if (has_level1) printf(", ");
            printf("L2: %dKB %d-way %dB line", 
                   level2.sizekb, level2.assoc, level2.line);
        }
        if (!has_level1 && !has_level2) {
            printf("No cache info set");
        }
    }
    
    /* Test case 0x49 with xeon_mp = 1 to show it's skipped */
    printf("\n\n=== Testing case 0x49 with xeon_mp = 1 ===\n");
    xeon_mp = 1;
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    int has_level1 = 0, has_level2 = 0;
    
    process_cache_descriptor(0x49, &level1, &level2, &has_level1, &has_level2);
    if (has_level2) {
        printf("L2 Cache set: %d KB, %d-way, %d byte line\n", 
               level2.sizekb, level2.assoc, level2.line);
    } else {
        printf("Case 0x49 skipped due to xeon_mp = 1\n");
    }
}

/* Simulate CPUID leaf 0x02 with specific descriptor bytes */
static void simulate_cpuid_leaf2_with_descriptors(void) {
    printf("\n=== Simulating CPUID Leaf 0x02 with Target Descriptors ===\n");
    
    /* Create a mock CPUID result with AL > 1 and our target descriptors */
    /* AL = 0x06 (6 valid descriptor bytes) */
    uint32_t mock_eax = 0x060A0C0D;  /* AL=0x06, then descriptors 0x0A, 0x0C, 0x0D */
    uint32_t mock_ebx = 0x0E21242C;  /* Descriptors 0x0E, 0x21, 0x24, 0x2C */
    uint32_t mock_ecx = 0x393A3B3C;  /* Descriptors 0x39, 0x3A, 0x3B, 0x3C */
    uint32_t mock_edx = 0x3D3E4142;  /* Descriptors 0x3D, 0x3E, 0x41, 0x42 */
    
    printf("Mock CPUID results: EAX=0x%08X, EBX=0x%08X, ECX=0x%08X, EDX=0x%08X\n",
           mock_eax, mock_ebx, mock_ecx, mock_edx);
    
    uint8_t al = mock_eax & 0xFF;
    if (al > 1) {
        printf("Processing %d descriptor bytes\n", al);
        
        struct cache_desc level1 = {0, 0, 0};
        struct cache_desc level2 = {0, 0, 0};
        int has_level1 = 0, has_level2 = 0;
        
        /* Process all bytes from mock registers */
        uint8_t *regs = (uint8_t*)&mock_eax;
        for (int i = 0; i < 4; i++) {
            if (regs[i] != 0 && (regs[i] & 0x80) == 0) {
                process_cache_descriptor(regs[i], &level1, &level2, 
                                        &has_level1, &has_level2);
            }
        }
        
        regs = (uint8_t*)&mock_ebx;
        for (int i = 0; i < 4; i++) {
            if (regs[i] != 0 && (regs[i] & 0x80) == 0) {
                process_cache_descriptor(regs[i], &level1, &level2, 
                                        &has_level1, &has_level2);
            }
        }
        
        regs = (uint8_t*)&mock_ecx;
        for (int i = 0; i < 4; i++) {
            if (regs[i] != 0 && (regs[i] & 0x80) == 0) {
                process_cache_descriptor(regs[i], &level1, &level2, 
                                        &has_level1, &has_level2);
            }
        }
        
        regs = (uint8_t*)&mock_edx;
        for (int i = 0; i < 4; i++) {
            if (regs[i] != 0 && (regs[i] & 0x80) == 0) {
                process_cache_descriptor(regs[i], &level1, &level2, 
                                        &has_level1, &has_level2);
            }
        }
        
        printf("\nFinal cache configuration:\n");
        if (has_level1) {
            printf("L1 Cache: %d KB, %d-way, %d byte line\n", 
                   level1.sizekb, level1.assoc, level1.line);
        }
        if (has_level2) {
            printf("L2 Cache: %d KB, %d-way, %d byte line\n", 
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
    
    /* Test 3: Simulate CPUID leaf 0x02 with specific descriptors */
    simulate_cpuid_leaf2_with_descriptors();
    
    /* Force compiler to keep all code */
    volatile int keep = 0;
    if (keep) {
        /* This ensures all switch cases are compiled in */
        struct cache_desc l1, l2;
        int h1, h2;
        process_cache_descriptor(0x00, &l1, &l2, &h1, &h2);
    }
    
    return 0;
}
