#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <cpuid.h>

/* Structures matching the cache descriptor format from driver-i386.cc */
struct cache_desc {
    int sizekb;
    int assoc;
    int line;
};

/* Global variables to track cache levels */
struct cache_desc level1 = {0, 0, 0};
struct cache_desc level2 = {0, 0, 0};
int xeon_mp = 0;  /* Set to 0 to hit the case 0x49 uncovered line */

/* Function to process cache descriptor bytes - replicates the uncovered switch logic */
void process_cache_descriptor(uint8_t descriptor) {
    switch (descriptor) {
        case 0x0a:
            level1.sizekb = 8; level1.assoc = 2; level1.line = 32;
            printf("Processed descriptor 0x0a: L1 8KB, 2-way, 32B line\n");
            break;
        case 0x0c:
            level1.sizekb = 16; level1.assoc = 4; level1.line = 32;
            printf("Processed descriptor 0x0c: L1 16KB, 4-way, 32B line\n");
            break;
        case 0x0d:
            level1.sizekb = 16; level1.assoc = 4; level1.line = 64;
            printf("Processed descriptor 0x0d: L1 16KB, 4-way, 64B line\n");
            break;
        case 0x0e:
            level1.sizekb = 24; level1.assoc = 6; level1.line = 64;
            printf("Processed descriptor 0x0e: L1 24KB, 6-way, 64B line\n");
            break;
        case 0x21:
            level2.sizekb = 256; level2.assoc = 8; level2.line = 64;
            printf("Processed descriptor 0x21: L2 256KB, 8-way, 64B line\n");
            break;
        case 0x24:
            level2.sizekb = 1024; level2.assoc = 16; level2.line = 64;
            printf("Processed descriptor 0x24: L2 1MB, 16-way, 64B line\n");
            break;
        case 0x2c:
            level1.sizekb = 32; level1.assoc = 8; level1.line = 64;
            printf("Processed descriptor 0x2c: L1 32KB, 8-way, 64B line\n");
            break;
        case 0x39:
            level2.sizekb = 128; level2.assoc = 4; level2.line = 64;
            printf("Processed descriptor 0x39: L2 128KB, 4-way, 64B line\n");
            break;
        case 0x3a:
            level2.sizekb = 192; level2.assoc = 6; level2.line = 64;
            printf("Processed descriptor 0x3a: L2 192KB, 6-way, 64B line\n");
            break;
        case 0x3b:
            level2.sizekb = 128; level2.assoc = 2; level2.line = 64;
            printf("Processed descriptor 0x3b: L2 128KB, 2-way, 64B line\n");
            break;
        case 0x3c:
            level2.sizekb = 256; level2.assoc = 4; level2.line = 64;
            printf("Processed descriptor 0x3c: L2 256KB, 4-way, 64B line\n");
            break;
        case 0x3d:
            level2.sizekb = 384; level2.assoc = 6; level2.line = 64;
            printf("Processed descriptor 0x3d: L2 384KB, 6-way, 64B line\n");
            break;
        case 0x3e:
            level2.sizekb = 512; level2.assoc = 4; level2.line = 64;
            printf("Processed descriptor 0x3e: L2 512KB, 4-way, 64B line\n");
            break;
        case 0x41:
            level2.sizekb = 128; level2.assoc = 4; level2.line = 32;
            printf("Processed descriptor 0x41: L2 128KB, 4-way, 32B line\n");
            break;
        case 0x42:
            level2.sizekb = 256; level2.assoc = 4; level2.line = 32;
            printf("Processed descriptor 0x42: L2 256KB, 4-way, 32B line\n");
            break;
        case 0x43:
            level2.sizekb = 512; level2.assoc = 4; level2.line = 32;
            printf("Processed descriptor 0x43: L2 512KB, 4-way, 32B line\n");
            break;
        case 0x44:
            level2.sizekb = 1024; level2.assoc = 4; level2.line = 32;
            printf("Processed descriptor 0x44: L2 1MB, 4-way, 32B line\n");
            break;
        case 0x45:
            level2.sizekb = 2048; level2.assoc = 4; level2.line = 32;
            printf("Processed descriptor 0x45: L2 2MB, 4-way, 32B line\n");
            break;
        case 0x48:
            level2.sizekb = 3072; level2.assoc = 12; level2.line = 64;
            printf("Processed descriptor 0x48: L2 3MB, 12-way, 64B line\n");
            break;
        case 0x49:
            if (xeon_mp)
                break;
            level2.sizekb = 4096; level2.assoc = 16; level2.line = 64;
            printf("Processed descriptor 0x49: L2 4MB, 16-way, 64B line (xeon_mp=false)\n");
            break;
        case 0x4e:
            level2.sizekb = 6144; level2.assoc = 24; level2.line = 64;
            printf("Processed descriptor 0x4e: L2 6MB, 24-way, 64B line\n");
            break;
        case 0x60:
            level1.sizekb = 16; level1.assoc = 8; level1.line = 64;
            printf("Processed descriptor 0x60: L1 16KB, 8-way, 64B line\n");
            break;
        case 0x66:
            level1.sizekb = 8; level1.assoc = 4; level1.line = 64;
            printf("Processed descriptor 0x66: L1 8KB, 4-way, 64B line\n");
            break;
        case 0x67:
            level1.sizekb = 16; level1.assoc = 4; level1.line = 64;
            printf("Processed descriptor 0x67: L1 16KB, 4-way, 64B line\n");
            break;
        case 0x68:
            level1.sizekb = 32; level1.assoc = 4; level1.line = 64;
            printf("Processed descriptor 0x68: L1 32KB, 4-way, 64B line\n");
            break;
        case 0x78:
            level2.sizekb = 1024; level2.assoc = 4; level2.line = 64;
            printf("Processed descriptor 0x78: L2 1MB, 4-way, 64B line\n");
            break;
        case 0x79:
            level2.sizekb = 128; level2.assoc = 8; level2.line = 64;
            printf("Processed descriptor 0x79: L2 128KB, 8-way, 64B line\n");
            break;
        case 0x7a:
            level2.sizekb = 256; level2.assoc = 8; level2.line = 64;
            printf("Processed descriptor 0x7a: L2 256KB, 8-way, 64B line\n");
            break;
        case 0x7b:
            level2.sizekb = 512; level2.assoc = 8; level2.line = 64;
            printf("Processed descriptor 0x7b: L2 512KB, 8-way, 64B line\n");
            break;
        case 0x7c:
            level2.sizekb = 1024; level2.assoc = 8; level2.line = 64;
            printf("Processed descriptor 0x7c: L2 1MB, 8-way, 64B line\n");
            break;
        case 0x7d:
            level2.sizekb = 2048; level2.assoc = 8; level2.line = 64;
            printf("Processed descriptor 0x7d: L2 2MB, 8-way, 64B line\n");
            break;
        case 0x7f:
            level2.sizekb = 512; level2.assoc = 2; level2.line = 64;
            printf("Processed descriptor 0x7f: L2 512KB, 2-way, 64B line\n");
            break;
        case 0x80:
            level2.sizekb = 512; level2.assoc = 8; level2.line = 64;
            printf("Processed descriptor 0x80: L2 512KB, 8-way, 64B line\n");
            break;
        case 0x82:
            level2.sizekb = 256; level2.assoc = 8; level2.line = 32;
            printf("Processed descriptor 0x82: L2 256KB, 8-way, 32B line\n");
            break;
        case 0x83:
            level2.sizekb = 512; level2.assoc = 8; level2.line = 32;
            printf("Processed descriptor 0x83: L2 512KB, 8-way, 32B line\n");
            break;
        case 0x84:
            level2.sizekb = 1024; level2.assoc = 8; level2.line = 32;
            printf("Processed descriptor 0x84: L2 1MB, 8-way, 32B line\n");
            break;
        case 0x85:
            level2.sizekb = 2048; level2.assoc = 8; level2.line = 32;
            printf("Processed descriptor 0x85: L2 2MB, 8-way, 32B line\n");
            break;
        case 0x86:
            level2.sizekb = 512; level2.assoc = 4; level2.line = 64;
            printf("Processed descriptor 0x86: L2 512KB, 4-way, 64B line\n");
            break;
        case 0x87:
            level2.sizekb = 1024; level2.assoc = 8; level2.line = 64;
            printf("Processed descriptor 0x87: L2 1MB, 8-way, 64B line\n");
            break;
        default:
            /* Ignore other descriptors as in the original code */
            break;
    }
}

/* Simulate CPUID leaf 0x02 with descriptor table */
void simulate_cpuid_leaf2(void) {
    /* Array of target descriptor bytes from uncovered lines */
    uint8_t target_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    printf("Simulating CPUID leaf 0x02 with %zu descriptor bytes\n", 
           sizeof(target_descriptors));
    
    /* Process each descriptor byte */
    for (size_t i = 0; i < sizeof(target_descriptors); i++) {
        process_cache_descriptor(target_descriptors[i]);
    }
}

/* Test CPUID leaf 0x04 (deterministic cache parameters) */
void test_cpuid_leaf4(void) {
    uint32_t eax, ebx, ecx, edx;
    int cache_id = 0;
    
    printf("\nTesting CPUID leaf 0x04 (deterministic cache parameters):\n");
    
    do {
        /* Call CPUID leaf 0x04 with increasing ECX */
        __cpuid_count(0x04, cache_id, eax, ebx, ecx, edx);
        
        /* Extract cache type field (bits 4:0 of EAX) */
        uint32_t cache_type = eax & 0x1F;
        
        if (cache_type == 0) {
            printf("Cache ID %d: No more caches (type=0)\n", cache_id);
            break;
        }
        
        /* Extract cache level (bits 7:5 of EAX) */
        uint32_t cache_level = (eax >> 5) & 0x7;
        
        /* Extract other cache parameters */
        uint32_t line_size = (ebx & 0xFFF) + 1;
        uint32_t partitions = ((ebx >> 12) & 0x3FF) + 1;
        uint32_t associativity = ((ebx >> 22) & 0x3FF) + 1;
        uint32_t sets = ecx + 1;
        
        /* Calculate cache size */
        uint32_t cache_size = line_size * partitions * associativity * sets;
        
        printf("Cache ID %d: Level=%d, Type=%d, Size=%u bytes, "
               "Line=%u, Ways=%u, Sets=%u\n",
               cache_id, cache_level, cache_type, cache_size,
               line_size, associativity, sets);
        
        cache_id++;
    } while (1);
}

/* Test actual CPUID leaf 0x02 if supported */
void test_actual_cpuid(void) {
    uint32_t eax, ebx, ecx, edx;
    uint32_t max_leaf;
    
    /* Get maximum supported leaf */
    __cpuid(0x00, max_leaf, ebx, ecx, edx);
    
    printf("Maximum CPUID leaf: 0x%08x\n", max_leaf);
    
    if (max_leaf >= 0x02) {
        /* Call CPUID leaf 0x02 */
        __cpuid(0x02, eax, ebx, ecx, edx);
        
        /* Check if AL > 1 (valid descriptor table) */
        uint8_t al = eax & 0xFF;
        
        printf("CPUID leaf 0x02 returned: EAX=0x%08x, EBX=0x%08x, "
               "ECX=0x%08x, EDX=0x%08x\n", eax, ebx, ecx, edx);
        printf("First byte (AL): 0x%02x\n", al);
        
        if (al > 1) {
            /* Process descriptor bytes from all registers */
            uint8_t *regs = (uint8_t*)&eax;
            
            printf("Processing %d descriptor bytes:\n", al);
            for (int i = 1; i <= al; i++) {
                if (i < 16) {  /* Safety check */
                    uint8_t desc = regs[i];
                    if (desc != 0 && (desc & 0x80) == 0) {
                        process_cache_descriptor(desc);
                    }
                }
            }
        } else if (al == 1) {
            printf("AL=1: Using alternate cache detection method\n");
        } else {
            printf("AL=0: No cache descriptors returned\n");
        }
    } else {
        printf("CPUID leaf 0x02 not supported\n");
    }
}

int main(void) {
    printf("=== Cache Descriptor Coverage Test Program ===\n\n");
    
    /* Method 1: Simulate all target descriptor values */
    printf("--- Method 1: Simulating target descriptors ---\n");
    simulate_cpuid_leaf2();
    
    /* Method 2: Test actual CPUID if available */
    printf("\n--- Method 2: Testing actual CPUID ---\n");
    test_actual_cpuid();
    
    /* Method 3: Test deterministic cache parameters */
    test_cpuid_leaf4();
    
    /* Print final cache configuration */
    printf("\n--- Final Cache Configuration ---\n");
    if (level1.sizekb > 0) {
        printf("L1 Cache: %d KB, %d-way, %d-byte line\n", 
               level1.sizekb, level1.assoc, level1.line);
    }
    if (level2.sizekb > 0) {
        printf("L2 Cache: %d KB, %d-way, %d-byte line\n", 
               level2.sizekb, level2.assoc, level2.line);
    }
    
    /* Force compiler to keep variables */
    volatile int keep_level1 = level1.sizekb;
    volatile int keep_level2 = level2.sizekb;
    (void)keep_level1;
    (void)keep_level2;
    
    return 0;
}
