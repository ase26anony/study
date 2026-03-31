#include <stdio.h>
#include <stdint.h>
#include <cpuid.h>
#include <string.h>

/* Mock structures matching the original driver-i386.cc */
struct cache_desc {
    int sizekb;
    int assoc;
    int line;
};

/* Global variables to track cache levels */
struct cache_desc level1 = {0, 0, 0};
struct cache_desc level2 = {0, 0, 0};
int xeon_mp = 0;  /* We'll set this to 0 to hit the 0x49 case */

/* Function to process cache descriptor bytes - extracted from the uncovered logic */
void process_cache_descriptor(uint8_t descriptor) {
    switch (descriptor) {
        case 0x0a:
            level1.sizekb = 8; level1.assoc = 2; level1.line = 32;
            printf("Processed 0x0a: L1 8KB, 2-way, 32B line\n");
            break;
        case 0x0c:
            level1.sizekb = 16; level1.assoc = 4; level1.line = 32;
            printf("Processed 0x0c: L1 16KB, 4-way, 32B line\n");
            break;
        case 0x0d:
            level1.sizekb = 16; level1.assoc = 4; level1.line = 64;
            printf("Processed 0x0d: L1 16KB, 4-way, 64B line\n");
            break;
        case 0x0e:
            level1.sizekb = 24; level1.assoc = 6; level1.line = 64;
            printf("Processed 0x0e: L1 24KB, 6-way, 64B line\n");
            break;
        case 0x21:
            level2.sizekb = 256; level2.assoc = 8; level2.line = 64;
            printf("Processed 0x21: L2 256KB, 8-way, 64B line\n");
            break;
        case 0x24:
            level2.sizekb = 1024; level2.assoc = 16; level2.line = 64;
            printf("Processed 0x24: L2 1MB, 16-way, 64B line\n");
            break;
        case 0x2c:
            level1.sizekb = 32; level1.assoc = 8; level1.line = 64;
            printf("Processed 0x2c: L1 32KB, 8-way, 64B line\n");
            break;
        case 0x39:
            level2.sizekb = 128; level2.assoc = 4; level2.line = 64;
            printf("Processed 0x39: L2 128KB, 4-way, 64B line\n");
            break;
        case 0x3a:
            level2.sizekb = 192; level2.assoc = 6; level2.line = 64;
            printf("Processed 0x3a: L2 192KB, 6-way, 64B line\n");
            break;
        case 0x3b:
            level2.sizekb = 128; level2.assoc = 2; level2.line = 64;
            printf("Processed 0x3b: L2 128KB, 2-way, 64B line\n");
            break;
        case 0x3c:
            level2.sizekb = 256; level2.assoc = 4; level2.line = 64;
            printf("Processed 0x3c: L2 256KB, 4-way, 64B line\n");
            break;
        case 0x3d:
            level2.sizekb = 384; level2.assoc = 6; level2.line = 64;
            printf("Processed 0x3d: L2 384KB, 6-way, 64B line\n");
            break;
        case 0x3e:
            level2.sizekb = 512; level2.assoc = 4; level2.line = 64;
            printf("Processed 0x3e: L2 512KB, 4-way, 64B line\n");
            break;
        case 0x41:
            level2.sizekb = 128; level2.assoc = 4; level2.line = 32;
            printf("Processed 0x41: L2 128KB, 4-way, 32B line\n");
            break;
        case 0x42:
            level2.sizekb = 256; level2.assoc = 4; level2.line = 32;
            printf("Processed 0x42: L2 256KB, 4-way, 32B line\n");
            break;
        case 0x43:
            level2.sizekb = 512; level2.assoc = 4; level2.line = 32;
            printf("Processed 0x43: L2 512KB, 4-way, 32B line\n");
            break;
        case 0x44:
            level2.sizekb = 1024; level2.assoc = 4; level2.line = 32;
            printf("Processed 0x44: L2 1MB, 4-way, 32B line\n");
            break;
        case 0x45:
            level2.sizekb = 2048; level2.assoc = 4; level2.line = 32;
            printf("Processed 0x45: L2 2MB, 4-way, 32B line\n");
            break;
        case 0x48:
            level2.sizekb = 3072; level2.assoc = 12; level2.line = 64;
            printf("Processed 0x48: L2 3MB, 12-way, 64B line\n");
            break;
        case 0x49:
            if (xeon_mp)
                break;
            level2.sizekb = 4096; level2.assoc = 16; level2.line = 64;
            printf("Processed 0x49: L2 4MB, 16-way, 64B line (xeon_mp=0)\n");
            break;
        case 0x4e:
            level2.sizekb = 6144; level2.assoc = 24; level2.line = 64;
            printf("Processed 0x4e: L2 6MB, 24-way, 64B line\n");
            break;
        case 0x60:
            level1.sizekb = 16; level1.assoc = 8; level1.line = 64;
            printf("Processed 0x60: L1 16KB, 8-way, 64B line\n");
            break;
        case 0x66:
            level1.sizekb = 8; level1.assoc = 4; level1.line = 64;
            printf("Processed 0x66: L1 8KB, 4-way, 64B line\n");
            break;
        case 0x67:
            level1.sizekb = 16; level1.assoc = 4; level1.line = 64;
            printf("Processed 0x67: L1 16KB, 4-way, 64B line\n");
            break;
        case 0x68:
            level1.sizekb = 32; level1.assoc = 4; level1.line = 64;
            printf("Processed 0x68: L1 32KB, 4-way, 64B line\n");
            break;
        case 0x78:
            level2.sizekb = 1024; level2.assoc = 4; level2.line = 64;
            printf("Processed 0x78: L2 1MB, 4-way, 64B line\n");
            break;
        case 0x79:
            level2.sizekb = 128; level2.assoc = 8; level2.line = 64;
            printf("Processed 0x79: L2 128KB, 8-way, 64B line\n");
            break;
        case 0x7a:
            level2.sizekb = 256; level2.assoc = 8; level2.line = 64;
            printf("Processed 0x7a: L2 256KB, 8-way, 64B line\n");
            break;
        case 0x7b:
            level2.sizekb = 512; level2.assoc = 8; level2.line = 64;
            printf("Processed 0x7b: L2 512KB, 8-way, 64B line\n");
            break;
        case 0x7c:
            level2.sizekb = 1024; level2.assoc = 8; level2.line = 64;
            printf("Processed 0x7c: L2 1MB, 8-way, 64B line\n");
            break;
        case 0x7d:
            level2.sizekb = 2048; level2.assoc = 8; level2.line = 64;
            printf("Processed 0x7d: L2 2MB, 8-way, 64B line\n");
            break;
        case 0x7f:
            level2.sizekb = 512; level2.assoc = 2; level2.line = 64;
            printf("Processed 0x7f: L2 512KB, 2-way, 64B line\n");
            break;
        case 0x80:
            level2.sizekb = 512; level2.assoc = 8; level2.line = 64;
            printf("Processed 0x80: L2 512KB, 8-way, 64B line\n");
            break;
        case 0x82:
            level2.sizekb = 256; level2.assoc = 8; level2.line = 32;
            printf("Processed 0x82: L2 256KB, 8-way, 32B line\n");
            break;
        case 0x83:
            level2.sizekb = 512; level2.assoc = 8; level2.line = 32;
            printf("Processed 0x83: L2 512KB, 8-way, 32B line\n");
            break;
        case 0x84:
            level2.sizekb = 1024; level2.assoc = 8; level2.line = 32;
            printf("Processed 0x84: L2 1MB, 8-way, 32B line\n");
            break;
        case 0x85:
            level2.sizekb = 2048; level2.assoc = 8; level2.line = 32;
            printf("Processed 0x85: L2 2MB, 8-way, 32B line\n");
            break;
        case 0x86:
            level2.sizekb = 512; level2.assoc = 4; level2.line = 64;
            printf("Processed 0x86: L2 512KB, 4-way, 64B line\n");
            break;
        case 0x87:
            level2.sizekb = 1024; level2.assoc = 8; level2.line = 64;
            printf("Processed 0x87: L2 1MB, 8-way, 64B line\n");
            break;
        default:
            printf("Unknown descriptor: 0x%02x\n", descriptor);
            break;
    }
}

/* Simulate CPUID leaf 0x02 descriptor table parsing */
void simulate_cpuid_leaf2_parsing(void) {
    /* All the target descriptor bytes from uncovered lines */
    uint8_t target_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    printf("Simulating CPUID leaf 0x02 descriptor parsing...\n");
    printf("Number of descriptors to process: %zu\n\n", sizeof(target_descriptors));
    
    /* Process each descriptor */
    for (size_t i = 0; i < sizeof(target_descriptors); i++) {
        process_cache_descriptor(target_descriptors[i]);
    }
}

/* Actual CPUID leaf 0x02 call and parsing */
void real_cpuid_leaf2_test(void) {
    uint32_t eax, ebx, ecx, edx;
    
    printf("\n=== Real CPUID Leaf 0x02 Test ===\n");
    
    /* Call CPUID leaf 0x02 */
    __cpuid_count(0x02, 0, eax, ebx, ecx, edx);
    
    printf("CPUID Leaf 0x02 results:\n");
    printf("EAX: 0x%08x\n", eax);
    printf("EBX: 0x%08x\n", ebx);
    printf("ECX: 0x%08x\n", ecx);
    printf("EDX: 0x%08x\n", edx);
    
    /* Check if we should use descriptor table method */
    uint8_t al = eax & 0xFF;
    if (al > 1) {
        printf("Using descriptor table method (AL = 0x%02x)\n", al);
        
        /* Process bytes from EAX (skip first byte which is AL) */
        uint8_t *bytes = (uint8_t*)&eax;
        for (int i = 1; i < 4; i++) {
            if (bytes[i] != 0 && (bytes[i] & 0x80) == 0) {
                process_cache_descriptor(bytes[i]);
            }
        }
        
        /* Process bytes from EBX, ECX, EDX */
        bytes = (uint8_t*)&ebx;
        for (int i = 0; i < 4; i++) {
            if (bytes[i] != 0 && (bytes[i] & 0x80) == 0) {
                process_cache_descriptor(bytes[i]);
            }
        }
        
        bytes = (uint8_t*)&ecx;
        for (int i = 0; i < 4; i++) {
            if (bytes[i] != 0 && (bytes[i] & 0x80) == 0) {
                process_cache_descriptor(bytes[i]);
            }
        }
        
        bytes = (uint8_t*)&edx;
        for (int i = 0; i < 4; i++) {
            if (bytes[i] != 0 && (bytes[i] & 0x80) == 0) {
                process_cache_descriptor(bytes[i]);
            }
        }
    } else {
        printf("Not using descriptor table method (AL = 0x%02x)\n", al);
    }
}

/* CPUID leaf 0x04 test (deterministic cache parameters) */
void cpuid_leaf4_test(void) {
    uint32_t eax, ebx, ecx, edx;
    int cache_level = 0;
    
    printf("\n=== CPUID Leaf 0x04 Test ===\n");
    
    while (1) {
        __cpuid_count(0x04, cache_level, eax, ebx, ecx, edx);
        
        uint32_t cache_type = eax & 0x1F;
        if (cache_type == 0) {
            printf("No more caches at level %d\n", cache_level);
            break;
        }
        
        printf("Cache Level %d:\n", cache_level);
        printf("  Type: %u\n", cache_type);
        printf("  Level: %u\n", (eax >> 5) & 0x7);
        printf("  Self Initializing: %u\n", (eax >> 8) & 0x1);
        printf("  Fully Associative: %u\n", (eax >> 9) & 0x1);
        
        cache_level++;
    }
}

/* Test with fabricated CPUID results to force specific paths */
void fabricated_cpuid_test(void) {
    printf("\n=== Fabricated CPUID Test ===\n");
    
    /* Fabricate a CPUID leaf 0x02 result that will use descriptor table */
    /* AL = 0x03 means 3 valid descriptor bytes follow in EAX */
    uint32_t fabricated_eax = 0x030a0c0d;  /* AL=0x03, then descriptors 0x0a, 0x0c, 0x0d */
    uint32_t fabricated_ebx = 0x0e21242c;  /* descriptors 0x0e, 0x21, 0x24, 0x2c */
    uint32_t fabricated_ecx = 0x393a3b3c;  /* descriptors 0x39, 0x3a, 0x3b, 0x3c */
    uint32_t fabricated_edx = 0x3d3e4142;  /* descriptors 0x3d, 0x3e, 0x41, 0x42 */
    
    printf("Fabricated CPUID Leaf 0x02 results:\n");
    printf("EAX: 0x%08x\n", fabricated_eax);
    printf("EBX: 0x%08x\n", fabricated_ebx);
    printf("ECX: 0x%08x\n", fabricated_ecx);
    printf("EDX: 0x%08x\n", fabricated_edx);
    
    /* Process the fabricated bytes */
    uint8_t *bytes = (uint8_t*)&fabricated_eax;
    for (int i = 1; i < 4; i++) {
        if (bytes[i] != 0 && (bytes[i] & 0x80) == 0) {
            process_cache_descriptor(bytes[i]);
        }
    }
    
    bytes = (uint8_t*)&fabricated_ebx;
    for (int i = 0; i < 4; i++) {
        if (bytes[i] != 0 && (bytes[i] & 0x80) == 0) {
            process_cache_descriptor(bytes[i]);
        }
    }
    
    bytes = (uint8_t*)&fabricated_ecx;
    for (int i = 0; i < 4; i++) {
        if (bytes[i] != 0 && (bytes[i] & 0x80) == 0) {
            process_cache_descriptor(bytes[i]);
        }
    }
    
    bytes = (uint8_t*)&fabricated_edx;
    for (int i = 0; i < 4; i++) {
        if (bytes[i] != 0 && (bytes[i] & 0x80) == 0) {
            process_cache_descriptor(bytes[i]);
        }
    }
}

int main(void) {
    printf("=== Cache Descriptor Parser Test Program ===\n\n");
    
    /* Set xeon_mp to 0 to hit the 0x49 case */
    xeon_mp = 0;
    printf("xeon_mp flag set to: %d (0 = non-Xeon-MP, will process 0x49)\n\n", xeon_mp);
    
    /* Test 1: Simulate parsing of all target descriptors */
    simulate_cpuid_leaf2_parsing();
    
    /* Test 2: Real CPUID calls */
    real_cpuid_leaf2_test();
    
    /* Test 3: CPUID leaf 0x04 */
    cpuid_leaf4_test();
    
    /* Test 4: Fabricated CPUID results */
    fabricated_cpuid_test();
    
    /* Print final cache configuration */
    printf("\n=== Final Cache Configuration ===\n");
    printf("Level 1: %d KB, %d-way, %d-byte line\n", 
           level1.sizekb, level1.assoc, level1.line);
    printf("Level 2: %d KB, %d-way, %d-byte line\n", 
           level2.sizekb, level2.assoc, level2.line);
    
    return 0;
}
