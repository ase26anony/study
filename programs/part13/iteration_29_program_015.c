#include <stdio.h>
#include <stdint.h>
#include <cpuid.h>

/* Mock structures similar to driver-i386.cc */
struct cache_desc {
    int sizekb;
    int assoc;
    int line;
};

/* Global flag to simulate xeon_mp condition */
static int xeon_mp = 0;

/* Function to process cache descriptors - mimics the uncovered logic */
static void process_cache_descriptor(uint8_t descriptor, struct cache_desc *level1, struct cache_desc *level2) {
    switch (descriptor) {
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            printf("Descriptor 0x%02x: L1 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level1->sizekb, level1->assoc, level1->line);
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            printf("Descriptor 0x%02x: L1 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level1->sizekb, level1->assoc, level1->line);
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("Descriptor 0x%02x: L1 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level1->sizekb, level1->assoc, level1->line);
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            printf("Descriptor 0x%02x: L1 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level1->sizekb, level1->assoc, level1->line);
            break;
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            printf("Descriptor 0x%02x: L1 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level1->sizekb, level1->assoc, level1->line);
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x49:
            if (xeon_mp)
                break;
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line (xeon_mp=%d)\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line, xeon_mp);
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            printf("Descriptor 0x%02x: L1 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level1->sizekb, level1->assoc, level1->line);
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            printf("Descriptor 0x%02x: L1 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level1->sizekb, level1->assoc, level1->line);
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            printf("Descriptor 0x%02x: L1 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level1->sizekb, level1->assoc, level1->line);
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            printf("Descriptor 0x%02x: L1 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level1->sizekb, level1->assoc, level1->line);
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            printf("Descriptor 0x%02x: L2 - %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2->sizekb, level2->assoc, level2->line);
            break;
        default:
            printf("Descriptor 0x%02x: Not in target uncovered cases\n", descriptor);
            break;
    }
}

/* Simulate CPUID leaf 0x02 descriptor iteration */
static void simulate_cpuid_leaf2_iteration(uint8_t *descriptors, int count) {
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    
    printf("\n=== Simulating CPUID Leaf 0x02 Descriptor Processing ===\n");
    
    for (int i = 0; i < count; i++) {
        process_cache_descriptor(descriptors[i], &level1, &level2);
    }
}

/* Test CPUID leaf 0x04 (deterministic cache parameters) */
static void test_cpuid_leaf4(void) {
    uint32_t eax, ebx, ecx, edx;
    int cache_id = 0;
    
    printf("\n=== Testing CPUID Leaf 0x04 ===\n");
    
    do {
        __cpuid_count(0x04, cache_id, eax, ebx, ecx, edx);
        
        uint32_t cache_type = eax & 0x1F;
        
        if (cache_type == 0) {
            printf("Cache ID %d: No more caches\n", cache_id);
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
        
        printf("Cache ID %d: Type=%u, Level=%u, Size=%uKB, "
               "Line=%uB, Ways=%u, Sets=%u\n",
               cache_id, cache_type, cache_level, size,
               line_size, ways, sets);
        
        cache_id++;
    } while (1);
}

int main(void) {
    /* Target descriptor values from uncovered lines */
    uint8_t target_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c,
        0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e,
        0x41, 0x42, 0x43, 0x44, 0x45,
        0x48, 0x49, 0x4e,
        0x60, 0x66, 0x67, 0x68,
        0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7f, 0x80,
        0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    int num_descriptors = sizeof(target_descriptors) / sizeof(target_descriptors[0]);
    
    /* First, test with xeon_mp = 0 to hit the 0x49 case */
    printf("=== Test 1: xeon_mp = 0 (should process descriptor 0x49) ===\n");
    xeon_mp = 0;
    simulate_cpuid_leaf2_iteration(target_descriptors, num_descriptors);
    
    /* Then test with xeon_mp = 1 to show the difference */
    printf("\n=== Test 2: xeon_mp = 1 (should skip descriptor 0x49) ===\n");
    xeon_mp = 1;
    simulate_cpuid_leaf2_iteration(target_descriptors, num_descriptors);
    
    /* Test actual CPUID leaf 0x02 if supported */
    printf("\n=== Testing actual CPUID Leaf 0x02 ===\n");
    uint32_t eax, ebx, ecx, edx;
    __cpuid(0x02, eax, ebx, ecx, edx);
    
    /* Check if AL > 1 (valid descriptor table) */
    uint8_t al = eax & 0xFF;
    if (al > 1) {
        printf("CPUID Leaf 0x02 returned AL=0x%02x (valid descriptor table)\n", al);
        
        /* Extract descriptor bytes */
        uint8_t *regs = (uint8_t *)&eax;
        int bytes_to_process = al;
        
        printf("Descriptor bytes: ");
        for (int i = 0; i < 4 && bytes_to_process > 0; i++) {
            uint8_t byte = regs[i];
            if (byte != 0 && (byte & 0x80) == 0) { /* Valid descriptor, not a TLB */
                printf("0x%02x ", byte);
                bytes_to_process--;
            }
        }
        printf("\n");
    } else if (al == 1) {
        printf("CPUID Leaf 0x02 returned AL=1 (using alternate method)\n");
    } else {
        printf("CPUID Leaf 0x02 not supported or invalid\n");
    }
    
    /* Test CPUID leaf 0x04 */
    test_cpuid_leaf4();
    
    return 0;
}
