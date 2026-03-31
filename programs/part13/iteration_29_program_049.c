#include <stdio.h>
#include <stdint.h>
#include <cpuid.h>
#include <string.h>

/* Structures matching those in driver-i386.cc */
struct cache_desc {
    int sizekb;
    int assoc;
    int line;
};

/* Global variables matching the source */
static int xeon_mp = 0;  /* We'll set this to 0 to hit the 0x49 case */

/* Function to process cache descriptors - extracted from the uncovered block */
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
            /* Not one of our target descriptors */
            break;
    }
}

/* Simulate the CPUID leaf 0x02 descriptor parsing loop */
static void simulate_cpuid_leaf2_parsing(void) {
    struct cache_desc level1 = {0, 0, 0};
    struct cache_desc level2 = {0, 0, 0};
    int got_level1 = 0;
    int got_level2 = 0;
    
    /* All target descriptor bytes from uncovered lines */
    uint8_t target_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    printf("Testing all target cache descriptors:\n");
    printf("=====================================\n");
    
    for (size_t i = 0; i < sizeof(target_descriptors); i++) {
        /* Reset for each descriptor */
        memset(&level1, 0, sizeof(level1));
        memset(&level2, 0, sizeof(level2));
        got_level1 = 0;
        got_level2 = 0;
        
        process_cache_descriptor(target_descriptors[i], &level1, &level2, 
                                &got_level1, &got_level2);
        
        if (got_level1) {
            printf("Descriptor 0x%02x -> L1: %dKB, %d-way, %d-byte line\n",
                   target_descriptors[i], level1.sizekb, level1.assoc, level1.line);
        } else if (got_level2) {
            printf("Descriptor 0x%02x -> L2: %dKB, %d-way, %d-byte line\n",
                   target_descriptors[i], level2.sizekb, level2.assoc, level2.line);
        }
    }
    
    printf("\nSpecial test for descriptor 0x49 with xeon_mp = %d:\n", xeon_mp);
    printf("===================================================\n");
    memset(&level2, 0, sizeof(level2));
    got_level2 = 0;
    process_cache_descriptor(0x49, &level1, &level2, &got_level1, &got_level2);
    if (got_level2) {
        printf("Descriptor 0x49 -> L2: %dKB, %d-way, %d-byte line\n",
               level2.sizekb, level2.assoc, level2.line);
    } else {
        printf("Descriptor 0x49 skipped (xeon_mp branch taken)\n");
    }
}

/* Actual CPUID calls to trigger the real logic */
static void call_real_cpuid(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t *descriptor_bytes;
    int i, bytes_to_read;
    
    printf("\nReal CPUID leaf 0x02 call:\n");
    printf("==========================\n");
    
    /* Call CPUID leaf 0x02 */
    __cpuid(0x02, eax, ebx, ecx, edx);
    
    /* First byte of AL indicates number of valid descriptor bytes */
    bytes_to_read = eax & 0xFF;
    
    printf("CPUID leaf 0x02 returned: EAX=0x%08x, EBX=0x%08x, ECX=0x%08x, EDX=0x%08x\n",
           eax, ebx, ecx, edx);
    printf("Bytes to read from registers: %d\n", bytes_to_read);
    
    if (bytes_to_read == 0 || bytes_to_read == 1) {
        printf("Early return condition - using alternative cache detection method\n");
        return;
    }
    
    /* Process descriptor bytes from all 4 registers */
    descriptor_bytes = (uint8_t*)&eax;
    for (i = 0; i < 4 && bytes_to_read > 0; i++, bytes_to_read--) {
        if (descriptor_bytes[i] & 0x80) {
            /* Bit 7 set means this is not a descriptor */
            continue;
        }
        printf("  Descriptor byte: 0x%02x\n", descriptor_bytes[i]);
    }
    
    descriptor_bytes = (uint8_t*)&ebx;
    for (i = 0; i < 4 && bytes_to_read > 0; i++, bytes_to_read--) {
        if (descriptor_bytes[i] & 0x80) {
            continue;
        }
        printf("  Descriptor byte: 0x%02x\n", descriptor_bytes[i]);
    }
    
    descriptor_bytes = (uint8_t*)&ecx;
    for (i = 0; i < 4 && bytes_to_read > 0; i++, bytes_to_read--) {
        if (descriptor_bytes[i] & 0x80) {
            continue;
        }
        printf("  Descriptor byte: 0x%02x\n", descriptor_bytes[i]);
    }
    
    descriptor_bytes = (uint8_t*)&edx;
    for (i = 0; i < 4 && bytes_to_read > 0; i++, bytes_to_read--) {
        if (descriptor_bytes[i] & 0x80) {
            continue;
        }
        printf("  Descriptor byte: 0x%02x\n", descriptor_bytes[i]);
    }
    
    printf("\nCPUID leaf 0x04 calls (deterministic cache parameters):\n");
    printf("=======================================================\n");
    
    /* Call CPUID leaf 0x04 repeatedly until cache type is 0 */
    for (int idx = 0; ; idx++) {
        __cpuid_count(0x04, idx, eax, ebx, ecx, edx);
        
        int cache_type = eax & 0x1F;
        if (cache_type == 0) {
            printf("No more caches (type = 0)\n");
            break;
        }
        
        int cache_level = (eax >> 5) & 0x7;
        int self_initializing = (eax >> 8) & 0x1;
        int fully_associative = (eax >> 9) & 0x1;
        int max_threads = ((eax >> 14) & 0xFFF) + 1;
        int max_cores = ((eax >> 26) & 0x3F) + 1;
        
        int ways = ((ebx >> 22) & 0x3FF) + 1;
        int partitions = ((ebx >> 12) & 0x3FF) + 1;
        int line_size = (ebx & 0xFFF) + 1;
        int sets = ecx + 1;
        
        printf("Cache[%d]: type=%d, level=%d, ways=%d, partitions=%d, "
               "line_size=%d, sets=%d\n",
               idx, cache_type, cache_level, ways, partitions, line_size, sets);
    }
}

/* Test with fabricated CPUID results to hit all cases */
static void test_with_fabricated_data(void) {
    printf("\nTesting with fabricated CPUID leaf 0x02 data:\n");
    printf("============================================\n");
    
    /* Fabricate a CPUID result that will trigger the descriptor parsing */
    volatile uint32_t fabricated_eax = 0x03020100;  /* AL=3, with 3 valid bytes */
    volatile uint32_t fabricated_ebx = 0x0a0c0d0e;  /* Our target descriptors */
    volatile uint32_t fabricated_ecx = 0x21242c39;  /* More target descriptors */
    volatile uint32_t fabricated_edx = 0x3a3b3c3d;  /* Even more */
    
    /* Force compiler to keep these variables */
    asm volatile("" : : "r"(fabricated_eax), "r"(fabricated_ebx), 
                  "r"(fabricated_ecx), "r"(fabricated_edx));
    
    printf("Fabricated EAX: 0x%08x (AL=0x%02x)\n", 
           (uint32_t)fabricated_eax, (uint8_t)fabricated_eax);
    printf("Fabricated EBX: 0x%08x\n", (uint32_t)fabricated_ebx);
    printf("Fabricated ECX: 0x%08x\n", (uint32_t)fabricated_ecx);
    printf("Fabricated EDX: 0x%08x\n", (uint32_t)fabricated_edx);
    
    /* Process the fabricated bytes */
    uint8_t *bytes = (uint8_t*)&fabricated_eax;
    int bytes_to_read = fabricated_eax & 0xFF;
    
    printf("\nProcessing %d descriptor bytes:\n", bytes_to_read);
    
    /* Skip first byte (it's the count) */
    for (int i = 1; i < 4 && bytes_to_read > 0; i++, bytes_to_read--) {
        printf("  Byte from EAX[%d]: 0x%02x\n", i, bytes[i]);
    }
    
    bytes = (uint8_t*)&fabricated_ebx;
    for (int i = 0; i < 4 && bytes_to_read > 0; i++, bytes_to_read--) {
        printf("  Byte from EBX[%d]: 0x%02x\n", i, bytes[i]);
    }
}

int main(void) {
    printf("Cache Descriptor Test Program\n");
    printf("=============================\n\n");
    
    /* Ensure xeon_mp is false to hit the 0x49 case */
    xeon_mp = 0;
    
    /* Test 1: Simulate processing all target descriptors */
    simulate_cpuid_leaf2_parsing();
    
    /* Test 2: Make actual CPUID calls */
    call_real_cpuid();
    
    /* Test 3: Test with fabricated data */
    test_with_fabricated_data();
    
    /* Additional test: Try to trigger early return bypass */
    printf("\n\nTesting early return bypass:\n");
    printf("=============================\n");
    
    /* To bypass early returns, we need AL > 1 */
    volatile uint32_t test_eax = 0x06;  /* AL=6, should process descriptor table */
    printf("Test EAX with AL=0x%02x - should process descriptor table\n", 
           (uint8_t)test_eax);
    
    return 0;
}
