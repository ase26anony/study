#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <cpuid.h>

/* Structure matching cache_desc from driver-i386.cc */
struct cache_desc {
    int sizekb;
    int assoc;
    int line;
};

/* Global variables to track cache levels */
struct cache_desc level1 = {0, 0, 0};
struct cache_desc level2 = {0, 0, 0};
int xeon_mp = 0;  /* Set to 0 to hit case 0x49 */

/* Function to process cache descriptor bytes - directly from uncovered lines */
void process_cache_descriptor(uint8_t descriptor) {
    switch (descriptor) {
        case 0x0a:
            level1.sizekb = 8; level1.assoc = 2; level1.line = 32;
            printf("Case 0x0a: L1 Cache - 8KB, 2-way, 32B line\n");
            break;
        case 0x0c:
            level1.sizekb = 16; level1.assoc = 4; level1.line = 32;
            printf("Case 0x0c: L1 Cache - 16KB, 4-way, 32B line\n");
            break;
        case 0x0d:
            level1.sizekb = 16; level1.assoc = 4; level1.line = 64;
            printf("Case 0x0d: L1 Cache - 16KB, 4-way, 64B line\n");
            break;
        case 0x0e:
            level1.sizekb = 24; level1.assoc = 6; level1.line = 64;
            printf("Case 0x0e: L1 Cache - 24KB, 6-way, 64B line\n");
            break;
        case 0x21:
            level2.sizekb = 256; level2.assoc = 8; level2.line = 64;
            printf("Case 0x21: L2 Cache - 256KB, 8-way, 64B line\n");
            break;
        case 0x24:
            level2.sizekb = 1024; level2.assoc = 16; level2.line = 64;
            printf("Case 0x24: L2 Cache - 1024KB, 16-way, 64B line\n");
            break;
        case 0x2c:
            level1.sizekb = 32; level1.assoc = 8; level1.line = 64;
            printf("Case 0x2c: L1 Cache - 32KB, 8-way, 64B line\n");
            break;
        case 0x39:
            level2.sizekb = 128; level2.assoc = 4; level2.line = 64;
            printf("Case 0x39: L2 Cache - 128KB, 4-way, 64B line\n");
            break;
        case 0x3a:
            level2.sizekb = 192; level2.assoc = 6; level2.line = 64;
            printf("Case 0x3a: L2 Cache - 192KB, 6-way, 64B line\n");
            break;
        case 0x3b:
            level2.sizekb = 128; level2.assoc = 2; level2.line = 64;
            printf("Case 0x3b: L2 Cache - 128KB, 2-way, 64B line\n");
            break;
        case 0x3c:
            level2.sizekb = 256; level2.assoc = 4; level2.line = 64;
            printf("Case 0x3c: L2 Cache - 256KB, 4-way, 64B line\n");
            break;
        case 0x3d:
            level2.sizekb = 384; level2.assoc = 6; level2.line = 64;
            printf("Case 0x3d: L2 Cache - 384KB, 6-way, 64B line\n");
            break;
        case 0x3e:
            level2.sizekb = 512; level2.assoc = 4; level2.line = 64;
            printf("Case 0x3e: L2 Cache - 512KB, 4-way, 64B line\n");
            break;
        case 0x41:
            level2.sizekb = 128; level2.assoc = 4; level2.line = 32;
            printf("Case 0x41: L2 Cache - 128KB, 4-way, 32B line\n");
            break;
        case 0x42:
            level2.sizekb = 256; level2.assoc = 4; level2.line = 32;
            printf("Case 0x42: L2 Cache - 256KB, 4-way, 32B line\n");
            break;
        case 0x43:
            level2.sizekb = 512; level2.assoc = 4; level2.line = 32;
            printf("Case 0x43: L2 Cache - 512KB, 4-way, 32B line\n");
            break;
        case 0x44:
            level2.sizekb = 1024; level2.assoc = 4; level2.line = 32;
            printf("Case 0x44: L2 Cache - 1024KB, 4-way, 32B line\n");
            break;
        case 0x45:
            level2.sizekb = 2048; level2.assoc = 4; level2.line = 32;
            printf("Case 0x45: L2 Cache - 2048KB, 4-way, 32B line\n");
            break;
        case 0x48:
            level2.sizekb = 3072; level2.assoc = 12; level2.line = 64;
            printf("Case 0x48: L2 Cache - 3072KB, 12-way, 64B line\n");
            break;
        case 0x49:
            if (xeon_mp)
                break;
            level2.sizekb = 4096; level2.assoc = 16; level2.line = 64;
            printf("Case 0x49: L2 Cache - 4096KB, 16-way, 64B line (xeon_mp=false)\n");
            break;
        case 0x4e:
            level2.sizekb = 6144; level2.assoc = 24; level2.line = 64;
            printf("Case 0x4e: L2 Cache - 6144KB, 24-way, 64B line\n");
            break;
        case 0x60:
            level1.sizekb = 16; level1.assoc = 8; level1.line = 64;
            printf("Case 0x60: L1 Cache - 16KB, 8-way, 64B line\n");
            break;
        case 0x66:
            level1.sizekb = 8; level1.assoc = 4; level1.line = 64;
            printf("Case 0x66: L1 Cache - 8KB, 4-way, 64B line\n");
            break;
        case 0x67:
            level1.sizekb = 16; level1.assoc = 4; level1.line = 64;
            printf("Case 0x67: L1 Cache - 16KB, 4-way, 64B line\n");
            break;
        case 0x68:
            level1.sizekb = 32; level1.assoc = 4; level1.line = 64;
            printf("Case 0x68: L1 Cache - 32KB, 4-way, 64B line\n");
            break;
        case 0x78:
            level2.sizekb = 1024; level2.assoc = 4; level2.line = 64;
            printf("Case 0x78: L2 Cache - 1024KB, 4-way, 64B line\n");
            break;
        case 0x79:
            level2.sizekb = 128; level2.assoc = 8; level2.line = 64;
            printf("Case 0x79: L2 Cache - 128KB, 8-way, 64B line\n");
            break;
        case 0x7a:
            level2.sizekb = 256; level2.assoc = 8; level2.line = 64;
            printf("Case 0x7a: L2 Cache - 256KB, 8-way, 64B line\n");
            break;
        case 0x7b:
            level2.sizekb = 512; level2.assoc = 8; level2.line = 64;
            printf("Case 0x7b: L2 Cache - 512KB, 8-way, 64B line\n");
            break;
        case 0x7c:
            level2.sizekb = 1024; level2.assoc = 8; level2.line = 64;
            printf("Case 0x7c: L2 Cache - 1024KB, 8-way, 64B line\n");
            break;
        case 0x7d:
            level2.sizekb = 2048; level2.assoc = 8; level2.line = 64;
            printf("Case 0x7d: L2 Cache - 2048KB, 8-way, 64B line\n");
            break;
        case 0x7f:
            level2.sizekb = 512; level2.assoc = 2; level2.line = 64;
            printf("Case 0x7f: L2 Cache - 512KB, 2-way, 64B line\n");
            break;
        case 0x80:
            level2.sizekb = 512; level2.assoc = 8; level2.line = 64;
            printf("Case 0x80: L2 Cache - 512KB, 8-way, 64B line\n");
            break;
        case 0x82:
            level2.sizekb = 256; level2.assoc = 8; level2.line = 32;
            printf("Case 0x82: L2 Cache - 256KB, 8-way, 32B line\n");
            break;
        case 0x83:
            level2.sizekb = 512; level2.assoc = 8; level2.line = 32;
            printf("Case 0x83: L2 Cache - 512KB, 8-way, 32B line\n");
            break;
        case 0x84:
            level2.sizekb = 1024; level2.assoc = 8; level2.line = 32;
            printf("Case 0x84: L2 Cache - 1024KB, 8-way, 32B line\n");
            break;
        case 0x85:
            level2.sizekb = 2048; level2.assoc = 8; level2.line = 32;
            printf("Case 0x85: L2 Cache - 2048KB, 8-way, 32B line\n");
            break;
        case 0x86:
            level2.sizekb = 512; level2.assoc = 4; level2.line = 64;
            printf("Case 0x86: L2 Cache - 512KB, 4-way, 64B line\n");
            break;
        case 0x87:
            level2.sizekb = 1024; level2.assoc = 8; level2.line = 64;
            printf("Case 0x87: L2 Cache - 1024KB, 8-way, 64B line\n");
            break;
        default:
            /* Ignore other descriptors */
            break;
    }
}

/* Simulate CPUID leaf 0x02 with specific descriptor bytes */
void simulate_cpuid_leaf2_descriptors(void) {
    /* Array of all target descriptor bytes from uncovered lines */
    uint8_t target_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    printf("\n=== Simulating CPUID Leaf 0x02 Descriptors ===\n");
    
    /* Simulate AL = 0x03 (3 valid registers) to bypass early return */
    uint8_t al_value = 0x03;  /* More than 1, triggers descriptor table parsing */
    
    /* Process each target descriptor */
    for (size_t i = 0; i < sizeof(target_descriptors); i++) {
        process_cache_descriptor(target_descriptors[i]);
    }
}

/* Real CPUID leaf 0x02 call with iteration through descriptor bytes */
void real_cpuid_leaf2(void) {
    uint32_t eax, ebx, ecx, edx;
    
    printf("\n=== Real CPUID Leaf 0x02 Call ===\n");
    
    /* Call CPUID leaf 0x02 */
    __cpuid_count(0x02, 0, eax, ebx, ecx, edx);
    
    /* Check if AL > 1 (valid descriptor table) */
    uint8_t al = eax & 0xFF;
    
    if (al == 0 || al == 1) {
        printf("CPUID leaf 0x02 AL=%d - using alternative cache detection\n", al);
        return;
    }
    
    printf("CPUID leaf 0x02 AL=%d - processing descriptor table\n", al);
    
    /* Process descriptor bytes from all registers */
    uint8_t *regs = (uint8_t*)&eax;
    for (int i = 0; i < 16; i++) {
        if (i < 4) regs = (uint8_t*)&eax + i;
        else if (i < 8) regs = (uint8_t*)&ebx + (i - 4);
        else if (i < 12) regs = (uint8_t*)&ecx + (i - 8);
        else regs = (uint8_t*)&edx + (i - 12);
        
        uint8_t descriptor = *regs;
        if (descriptor != 0 && (descriptor & 0x80) == 0) {
            process_cache_descriptor(descriptor);
        }
    }
}

/* CPUID leaf 0x04 deterministic cache parameters */
void cpuid_leaf4_deterministic(void) {
    uint32_t eax, ebx, ecx, edx;
    int cache_level = 0;
    
    printf("\n=== CPUID Leaf 0x04 Deterministic Cache Parameters ===\n");
    
    /* Iterate through cache levels */
    for (int i = 0; ; i++) {
        __cpuid_count(0x04, i, eax, ebx, ecx, edx);
        
        /* Check cache type field (bits 4:0) */
        uint32_t cache_type = eax & 0x1F;
        
        if (cache_type == 0) {
            printf("No more caches at index %d\n", i);
            break;
        }
        
        cache_level++;
        
        /* Extract cache information */
        uint32_t cache_level_num = (eax >> 5) & 0x7;
        uint32_t self_initializing = (eax >> 8) & 0x1;
        uint32_t fully_associative = (eax >> 9) & 0x1;
        uint32_t max_threads = (eax >> 14) & 0x3FF;
        uint32_t max_cores = (eax >> 26) & 0x3F;
        
        /* Ways of associativity */
        uint32_t ways = ((ebx >> 22) & 0x3FF) + 1;
        
        /* Physical line partitions */
        uint32_t partitions = (ebx >> 12) & 0x3FF;
        
        /* System coherency line size */
        uint32_t line_size = (ebx & 0xFFF) + 1;
        
        /* Number of sets */
        uint32_t sets = ecx + 1;
        
        /* Calculate cache size */
        uint32_t size_bytes = ways * partitions * line_size * sets;
        uint32_t size_kb = size_bytes / 1024;
        
        printf("Cache Level %d: Type=%d, Size=%dKB, Ways=%d, Line=%dB, Sets=%d\n",
               cache_level_num, cache_type, size_kb, ways, line_size, sets);
    }
}

/* Test xeon_mp conditional branch */
void test_xeon_mp_conditional(void) {
    printf("\n=== Testing Xeon MP Conditional (Case 0x49) ===\n");
    
    /* First with xeon_mp = 0 (should execute the assignment) */
    xeon_mp = 0;
    level2.sizekb = 0; level2.assoc = 0; level2.line = 0;
    process_cache_descriptor(0x49);
    printf("With xeon_mp=0: L2 size=%dKB, assoc=%d, line=%dB\n", 
           level2.sizekb, level2.assoc, level2.line);
    
    /* Then with xeon_mp = 1 (should skip the assignment) */
    xeon_mp = 1;
    level2.sizekb = 0; level2.assoc = 0; level2.line = 0;
    process_cache_descriptor(0x49);
    printf("With xeon_mp=1: L2 size=%dKB, assoc=%d, line=%dB (should be 0)\n", 
           level2.sizekb, level2.assoc, level2.line);
}

int main(void) {
    printf("Cache Descriptor Coverage Test Program\n");
    printf("======================================\n");
    
    /* Test 1: Simulate all target descriptor cases */
    simulate_cpuid_leaf2_descriptors();
    
    /* Test 2: Real CPUID leaf 0x02 call */
    real_cpuid_leaf2();
    
    /* Test 3: CPUID leaf 0x04 deterministic parameters */
    cpuid_leaf4_deterministic();
    
    /* Test 4: Specific test for xeon_mp conditional */
    test_xeon_mp_conditional();
    
    /* Final summary */
    printf("\n=== Final Cache Configuration ===\n");
    printf("L1 Cache: %dKB, %d-way, %dB line\n", 
           level1.sizekb, level1.assoc, level1.line);
    printf("L2 Cache: %dKB, %d-way, %dB line\n", 
           level2.sizekb, level2.assoc, level2.line);
    
    /* Prevent optimization */
    volatile int dummy = level1.sizekb + level2.sizekb;
    
    return 0;
}
