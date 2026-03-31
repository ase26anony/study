#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <cpuid.h>

/* Mock structures matching the original driver-i386.cc */
struct cache_desc {
    int sizekb;
    int assoc;
    int line;
};

/* Global variables to track cache levels */
struct cache_desc level1_cache = {0, 0, 0};
struct cache_desc level2_cache = {0, 0, 0};
int xeon_mp = 0;  /* Set to 0 to trigger the 0x49 case */

/* Function to process cache descriptor bytes - mimics the uncovered logic */
void process_cache_descriptor(uint8_t desc) {
    switch (desc) {
        case 0x0a:
            level1_cache.sizekb = 8; level1_cache.assoc = 2; level1_cache.line = 32;
            printf("Case 0x0a: L1 Cache - 8KB, 2-way, 32-byte line\n");
            break;
        case 0x0c:
            level1_cache.sizekb = 16; level1_cache.assoc = 4; level1_cache.line = 32;
            printf("Case 0x0c: L1 Cache - 16KB, 4-way, 32-byte line\n");
            break;
        case 0x0d:
            level1_cache.sizekb = 16; level1_cache.assoc = 4; level1_cache.line = 64;
            printf("Case 0x0d: L1 Cache - 16KB, 4-way, 64-byte line\n");
            break;
        case 0x0e:
            level1_cache.sizekb = 24; level1_cache.assoc = 6; level1_cache.line = 64;
            printf("Case 0x0e: L1 Cache - 24KB, 6-way, 64-byte line\n");
            break;
        case 0x21:
            level2_cache.sizekb = 256; level2_cache.assoc = 8; level2_cache.line = 64;
            printf("Case 0x21: L2 Cache - 256KB, 8-way, 64-byte line\n");
            break;
        case 0x24:
            level2_cache.sizekb = 1024; level2_cache.assoc = 16; level2_cache.line = 64;
            printf("Case 0x24: L2 Cache - 1024KB, 16-way, 64-byte line\n");
            break;
        case 0x2c:
            level1_cache.sizekb = 32; level1_cache.assoc = 8; level1_cache.line = 64;
            printf("Case 0x2c: L1 Cache - 32KB, 8-way, 64-byte line\n");
            break;
        case 0x39:
            level2_cache.sizekb = 128; level2_cache.assoc = 4; level2_cache.line = 64;
            printf("Case 0x39: L2 Cache - 128KB, 4-way, 64-byte line\n");
            break;
        case 0x3a:
            level2_cache.sizekb = 192; level2_cache.assoc = 6; level2_cache.line = 64;
            printf("Case 0x3a: L2 Cache - 192KB, 6-way, 64-byte line\n");
            break;
        case 0x3b:
            level2_cache.sizekb = 128; level2_cache.assoc = 2; level2_cache.line = 64;
            printf("Case 0x3b: L2 Cache - 128KB, 2-way, 64-byte line\n");
            break;
        case 0x3c:
            level2_cache.sizekb = 256; level2_cache.assoc = 4; level2_cache.line = 64;
            printf("Case 0x3c: L2 Cache - 256KB, 4-way, 64-byte line\n");
            break;
        case 0x3d:
            level2_cache.sizekb = 384; level2_cache.assoc = 6; level2_cache.line = 64;
            printf("Case 0x3d: L2 Cache - 384KB, 6-way, 64-byte line\n");
            break;
        case 0x3e:
            level2_cache.sizekb = 512; level2_cache.assoc = 4; level2_cache.line = 64;
            printf("Case 0x3e: L2 Cache - 512KB, 4-way, 64-byte line\n");
            break;
        case 0x41:
            level2_cache.sizekb = 128; level2_cache.assoc = 4; level2_cache.line = 32;
            printf("Case 0x41: L2 Cache - 128KB, 4-way, 32-byte line\n");
            break;
        case 0x42:
            level2_cache.sizekb = 256; level2_cache.assoc = 4; level2_cache.line = 32;
            printf("Case 0x42: L2 Cache - 256KB, 4-way, 32-byte line\n");
            break;
        case 0x43:
            level2_cache.sizekb = 512; level2_cache.assoc = 4; level2_cache.line = 32;
            printf("Case 0x43: L2 Cache - 512KB, 4-way, 32-byte line\n");
            break;
        case 0x44:
            level2_cache.sizekb = 1024; level2_cache.assoc = 4; level2_cache.line = 32;
            printf("Case 0x44: L2 Cache - 1024KB, 4-way, 32-byte line\n");
            break;
        case 0x45:
            level2_cache.sizekb = 2048; level2_cache.assoc = 4; level2_cache.line = 32;
            printf("Case 0x45: L2 Cache - 2048KB, 4-way, 32-byte line\n");
            break;
        case 0x48:
            level2_cache.sizekb = 3072; level2_cache.assoc = 12; level2_cache.line = 64;
            printf("Case 0x48: L2 Cache - 3072KB, 12-way, 64-byte line\n");
            break;
        case 0x49:
            if (xeon_mp)
                break;
            level2_cache.sizekb = 4096; level2_cache.assoc = 16; level2_cache.line = 64;
            printf("Case 0x49: L2 Cache - 4096KB, 16-way, 64-byte line (xeon_mp = false)\n");
            break;
        case 0x4e:
            level2_cache.sizekb = 6144; level2_cache.assoc = 24; level2_cache.line = 64;
            printf("Case 0x4e: L2 Cache - 6144KB, 24-way, 64-byte line\n");
            break;
        case 0x60:
            level1_cache.sizekb = 16; level1_cache.assoc = 8; level1_cache.line = 64;
            printf("Case 0x60: L1 Cache - 16KB, 8-way, 64-byte line\n");
            break;
        case 0x66:
            level1_cache.sizekb = 8; level1_cache.assoc = 4; level1_cache.line = 64;
            printf("Case 0x66: L1 Cache - 8KB, 4-way, 64-byte line\n");
            break;
        case 0x67:
            level1_cache.sizekb = 16; level1_cache.assoc = 4; level1_cache.line = 64;
            printf("Case 0x67: L1 Cache - 16KB, 4-way, 64-byte line\n");
            break;
        case 0x68:
            level1_cache.sizekb = 32; level1_cache.assoc = 4; level1_cache.line = 64;
            printf("Case 0x68: L1 Cache - 32KB, 4-way, 64-byte line\n");
            break;
        case 0x78:
            level2_cache.sizekb = 1024; level2_cache.assoc = 4; level2_cache.line = 64;
            printf("Case 0x78: L2 Cache - 1024KB, 4-way, 64-byte line\n");
            break;
        case 0x79:
            level2_cache.sizekb = 128; level2_cache.assoc = 8; level2_cache.line = 64;
            printf("Case 0x79: L2 Cache - 128KB, 8-way, 64-byte line\n");
            break;
        case 0x7a:
            level2_cache.sizekb = 256; level2_cache.assoc = 8; level2_cache.line = 64;
            printf("Case 0x7a: L2 Cache - 256KB, 8-way, 64-byte line\n");
            break;
        case 0x7b:
            level2_cache.sizekb = 512; level2_cache.assoc = 8; level2_cache.line = 64;
            printf("Case 0x7b: L2 Cache - 512KB, 8-way, 64-byte line\n");
            break;
        case 0x7c:
            level2_cache.sizekb = 1024; level2_cache.assoc = 8; level2_cache.line = 64;
            printf("Case 0x7c: L2 Cache - 1024KB, 8-way, 64-byte line\n");
            break;
        case 0x7d:
            level2_cache.sizekb = 2048; level2_cache.assoc = 8; level2_cache.line = 64;
            printf("Case 0x7d: L2 Cache - 2048KB, 8-way, 64-byte line\n");
            break;
        case 0x7f:
            level2_cache.sizekb = 512; level2_cache.assoc = 2; level2_cache.line = 64;
            printf("Case 0x7f: L2 Cache - 512KB, 2-way, 64-byte line\n");
            break;
        case 0x80:
            level2_cache.sizekb = 512; level2_cache.assoc = 8; level2_cache.line = 64;
            printf("Case 0x80: L2 Cache - 512KB, 8-way, 64-byte line\n");
            break;
        case 0x82:
            level2_cache.sizekb = 256; level2_cache.assoc = 8; level2_cache.line = 32;
            printf("Case 0x82: L2 Cache - 256KB, 8-way, 32-byte line\n");
            break;
        case 0x83:
            level2_cache.sizekb = 512; level2_cache.assoc = 8; level2_cache.line = 32;
            printf("Case 0x83: L2 Cache - 512KB, 8-way, 32-byte line\n");
            break;
        case 0x84:
            level2_cache.sizekb = 1024; level2_cache.assoc = 8; level2_cache.line = 32;
            printf("Case 0x84: L2 Cache - 1024KB, 8-way, 32-byte line\n");
            break;
        case 0x85:
            level2_cache.sizekb = 2048; level2_cache.assoc = 8; level2_cache.line = 32;
            printf("Case 0x85: L2 Cache - 2048KB, 8-way, 32-byte line\n");
            break;
        case 0x86:
            level2_cache.sizekb = 512; level2_cache.assoc = 4; level2_cache.line = 64;
            printf("Case 0x86: L2 Cache - 512KB, 4-way, 64-byte line\n");
            break;
        case 0x87:
            level2_cache.sizekb = 1024; level2_cache.assoc = 8; level2_cache.line = 64;
            printf("Case 0x87: L2 Cache - 1024KB, 8-way, 64-byte line\n");
            break;
        default:
            /* Ignore other descriptor values */
            break;
    }
}

/* Simulate CPUID leaf 0x02 with specific descriptor bytes */
void simulate_cpuid_leaf2(uint8_t* descriptors, int count) {
    printf("\n=== Simulating CPUID Leaf 0x02 ===\n");
    printf("Number of valid descriptor bytes: %d\n", count);
    
    /* Process each descriptor byte */
    for (int i = 0; i < count; i++) {
        process_cache_descriptor(descriptors[i]);
    }
}

/* Test CPUID leaf 0x04 (deterministic cache parameters) */
void test_cpuid_leaf4() {
    printf("\n=== Testing CPUID Leaf 0x04 ===\n");
    
    unsigned int eax, ebx, ecx, edx;
    int cache_index = 0;
    
    /* Loop through cache levels until cache type is 0 */
    do {
        __cpuid_count(0x04, cache_index, eax, ebx, ecx, edx);
        
        int cache_type = eax & 0x1F;
        if (cache_type == 0) {
            break;
        }
        
        printf("Cache Level %d: Type=%d, Level=%d\n", 
               cache_index, cache_type, (eax >> 5) & 0x7);
        
        cache_index++;
    } while (1);
    
    printf("Found %d deterministic cache(s)\n", cache_index);
}

int main() {
    printf("Cache Descriptor Test Program\n");
    printf("=============================\n");
    
    /* First, test real CPUID leaf 0x02 if available */
    unsigned int eax, ebx, ecx, edx;
    __cpuid(0x02, eax, ebx, ecx, edx);
    
    /* Check if leaf 0x02 is supported (AL != 0) */
    uint8_t al_byte = eax & 0xFF;
    
    if (al_byte != 0) {
        printf("Real CPUID Leaf 0x02 returned: AL=0x%02X\n", al_byte);
        printf("EAX=0x%08X, EBX=0x%08X, ECX=0x%08X, EDX=0x%08X\n", 
               eax, ebx, ecx, edx);
        
        /* Process real descriptor bytes if AL > 1 */
        if (al_byte > 1) {
            uint8_t* reg_bytes = (uint8_t*)&eax;
            for (int i = 1; i < 4; i++) {  /* Skip AL byte */
                if (reg_bytes[i] != 0) {
                    process_cache_descriptor(reg_bytes[i]);
                }
            }
            
            reg_bytes = (uint8_t*)&ebx;
            for (int i = 0; i < 4; i++) {
                if (reg_bytes[i] != 0) {
                    process_cache_descriptor(reg_bytes[i]);
                }
            }
            
            reg_bytes = (uint8_t*)&ecx;
            for (int i = 0; i < 4; i++) {
                if (reg_bytes[i] != 0) {
                    process_cache_descriptor(reg_bytes[i]);
                }
            }
            
            reg_bytes = (uint8_t*)&edx;
            for (int i = 0; i < 4; i++) {
                if (reg_bytes[i] != 0) {
                    process_cache_descriptor(reg_bytes[i]);
                }
            }
        }
    } else {
        printf("CPUID Leaf 0x02 not supported or AL=0\n");
    }
    
    /* Now simulate all the uncovered cases */
    uint8_t test_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    /* Simulate CPUID leaf 0x02 with AL = number of valid bytes */
    simulate_cpuid_leaf2(test_descriptors, sizeof(test_descriptors));
    
    /* Test CPUID leaf 0x04 */
    test_cpuid_leaf4();
    
    /* Print final cache configuration */
    printf("\n=== Final Cache Configuration ===\n");
    printf("L1 Cache: %d KB, %d-way, %d-byte line\n", 
           level1_cache.sizekb, level1_cache.assoc, level1_cache.line);
    printf("L2 Cache: %d KB, %d-way, %d-byte line\n", 
           level2_cache.sizekb, level2_cache.assoc, level2_cache.line);
    
    return 0;
}
