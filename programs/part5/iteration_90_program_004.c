#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Cross-platform CPUID intrinsics */
#ifdef _WIN32
#include <intrin.h>
#else
#include <cpuid.h>
#endif

/* Cache descriptor structure matching driver-i386.cc */
struct cache_desc {
    int sizekb;     /* Size in KB */
    int assoc;      /* Associativity */
    int line;       /* Line size in bytes */
    int level;      /* Cache level (1 or 2) */
    int type;       /* Cache type */
};

/* Function to execute CPUID instruction */
static void cpuid(uint32_t leaf, uint32_t subleaf, 
                  uint32_t* eax, uint32_t* ebx, 
                  uint32_t* ecx, uint32_t* edx) {
#ifdef _WIN32
    int regs[4];
    __cpuidex(regs, leaf, subleaf);
    *eax = regs[0];
    *ebx = regs[1];
    *ecx = regs[2];
    *edx = regs[3];
#else
    __cpuid_count(leaf, subleaf, *eax, *ebx, *ecx, *edx);
#endif
}

/* Check if CPU is Xeon MP (simplified check) */
static int is_xeon_mp(uint32_t family, uint32_t model, uint32_t stepping) {
    /* Simplified: Check for family 0xF (Pentium 4/Xeon) and model >= 0x4 */
    /* This mimics the logic that would trigger the xeon_mp condition */
    if (family == 0xF && model >= 0x4) {
        /* Additional check for MP capability */
        uint32_t eax, ebx, ecx, edx;
        cpuid(1, 0, &eax, &ebx, &ecx, &edx);
        /* Check HTT bit (bit 28) and APIC ID */
        if ((edx >> 28) & 1) {
            /* Extract initial APIC ID to guess MP */
            uint8_t apic_id = (ebx >> 24) & 0xFF;
            return (apic_id > 1); /* Assume MP if APIC ID > 1 */
        }
    }
    return 0;
}

/* Process cache descriptor byte */
static void process_descriptor(uint8_t desc, struct cache_desc* level1, 
                               struct cache_desc* level2, int xeon_mp) {
    switch (desc) {
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            level1->level = 1; level1->type = 1; /* Data cache */
            printf("L1 Cache: 8KB, 2-way, 32-byte line (0x0a)\n");
            break;
        case 0x0c:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 32;
            level1->level = 1; level1->type = 1;
            printf("L1 Cache: 16KB, 4-way, 32-byte line (0x0c)\n");
            break;
        case 0x0d:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            level1->level = 1; level1->type = 1;
            printf("L1 Cache: 16KB, 4-way, 64-byte line (0x0d)\n");
            break;
        case 0x0e:
            level1->sizekb = 24; level1->assoc = 6; level1->line = 64;
            level1->level = 1; level1->type = 1;
            printf("L1 Cache: 24KB, 6-way, 64-byte line (0x0e)\n");
            break;
        case 0x21:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 3; /* Unified cache */
            printf("L2 Cache: 256KB, 8-way, 64-byte line (0x21)\n");
            break;
        case 0x24:
            level2->sizekb = 1024; level2->assoc = 16; level2->line = 64;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 1MB, 16-way, 64-byte line (0x24)\n");
            break;
        case 0x2c:
            level1->sizekb = 32; level1->assoc = 8; level1->line = 64;
            level1->level = 1; level1->type = 1;
            printf("L1 Cache: 32KB, 8-way, 64-byte line (0x2c)\n");
            break;
        case 0x39:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 128KB, 4-way, 64-byte line (0x39)\n");
            break;
        case 0x3a:
            level2->sizekb = 192; level2->assoc = 6; level2->line = 64;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 192KB, 6-way, 64-byte line (0x3a)\n");
            break;
        case 0x3b:
            level2->sizekb = 128; level2->assoc = 2; level2->line = 64;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 128KB, 2-way, 64-byte line (0x3b)\n");
            break;
        case 0x3c:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 256KB, 4-way, 64-byte line (0x3c)\n");
            break;
        case 0x3d:
            level2->sizekb = 384; level2->assoc = 6; level2->line = 64;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 384KB, 6-way, 64-byte line (0x3d)\n");
            break;
        case 0x3e:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 512KB, 4-way, 64-byte line (0x3e)\n");
            break;
        case 0x41:
            level2->sizekb = 128; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 128KB, 4-way, 32-byte line (0x41)\n");
            break;
        case 0x42:
            level2->sizekb = 256; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 256KB, 4-way, 32-byte line (0x42)\n");
            break;
        case 0x43:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 512KB, 4-way, 32-byte line (0x43)\n");
            break;
        case 0x44:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 1MB, 4-way, 32-byte line (0x44)\n");
            break;
        case 0x45:
            level2->sizekb = 2048; level2->assoc = 4; level2->line = 32;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 2MB, 4-way, 32-byte line (0x45)\n");
            break;
        case 0x48:
            level2->sizekb = 3072; level2->assoc = 12; level2->line = 64;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 3MB, 12-way, 64-byte line (0x48)\n");
            break;
        case 0x49:
            if (xeon_mp) {
                printf("Case 0x49: Xeon MP detected, skipping L2 cache init\n");
                break;
            }
            level2->sizekb = 4096; level2->assoc = 16; level2->line = 64;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 4MB, 16-way, 64-byte line (0x49)\n");
            break;
        case 0x4e:
            level2->sizekb = 6144; level2->assoc = 24; level2->line = 64;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 6MB, 24-way, 64-byte line (0x4e)\n");
            break;
        case 0x60:
            level1->sizekb = 16; level1->assoc = 8; level1->line = 64;
            level1->level = 1; level1->type = 1;
            printf("L1 Cache: 16KB, 8-way, 64-byte line (0x60)\n");
            break;
        case 0x66:
            level1->sizekb = 8; level1->assoc = 4; level1->line = 64;
            level1->level = 1; level1->type = 1;
            printf("L1 Cache: 8KB, 4-way, 64-byte line (0x66)\n");
            break;
        case 0x67:
            level1->sizekb = 16; level1->assoc = 4; level1->line = 64;
            level1->level = 1; level1->type = 1;
            printf("L1 Cache: 16KB, 4-way, 64-byte line (0x67)\n");
            break;
        case 0x68:
            level1->sizekb = 32; level1->assoc = 4; level1->line = 64;
            level1->level = 1; level1->type = 1;
            printf("L1 Cache: 32KB, 4-way, 64-byte line (0x68)\n");
            break;
        case 0x78:
            level2->sizekb = 1024; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 1MB, 4-way, 64-byte line (0x78)\n");
            break;
        case 0x79:
            level2->sizekb = 128; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 128KB, 8-way, 64-byte line (0x79)\n");
            break;
        case 0x7a:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 256KB, 8-way, 64-byte line (0x7a)\n");
            break;
        case 0x7b:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 512KB, 8-way, 64-byte line (0x7b)\n");
            break;
        case 0x7c:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 1MB, 8-way, 64-byte line (0x7c)\n");
            break;
        case 0x7d:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 2MB, 8-way, 64-byte line (0x7d)\n");
            break;
        case 0x7f:
            level2->sizekb = 512; level2->assoc = 2; level2->line = 64;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 512KB, 2-way, 64-byte line (0x7f)\n");
            break;
        case 0x80:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 512KB, 8-way, 64-byte line (0x80)\n");
            break;
        case 0x82:
            level2->sizekb = 256; level2->assoc = 8; level2->line = 32;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 256KB, 8-way, 32-byte line (0x82)\n");
            break;
        case 0x83:
            level2->sizekb = 512; level2->assoc = 8; level2->line = 32;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 512KB, 8-way, 32-byte line (0x83)\n");
            break;
        case 0x84:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 32;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 1MB, 8-way, 32-byte line (0x84)\n");
            break;
        case 0x85:
            level2->sizekb = 2048; level2->assoc = 8; level2->line = 32;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 2MB, 8-way, 32-byte line (0x85)\n");
            break;
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 512KB, 4-way, 64-byte line (0x86)\n");
            break;
        case 0x87:
            level2->sizekb = 1024; level2->assoc = 8; level2->line = 64;
            level2->level = 2; level2->type = 3;
            printf("L2 Cache: 1MB, 8-way, 64-byte line (0x87)\n");
            break;
        case 0x00:
            /* Valid terminator */
            break;
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x08:
        case 0x09:
        case 0x0b:
        case 0x10:
        case 0x15:
        case 0x1a:
        case 0x88:
        case 0x89:
        case 0x8a:
        case 0x8d:
        case 0x90:
        case 0x96:
        case 0x9b:
            /* Other valid descriptors we don't specifically target */
            printf("Other descriptor: 0x%02x\n", desc);
            break;
        default:
            /* Invalid or reserved descriptor */
            if (desc & 0x80) {
                /* Valid descriptor but not in our target list */
                printf("Valid descriptor (bit 7 set): 0x%02x\n", desc);
            }
            break;
    }
}

/* Extract descriptor bytes from CPUID results */
static void extract_descriptors(uint32_t eax, uint32_t ebx, uint32_t ecx, 
                                uint32_t edx, uint8_t* descriptors, int* count) {
    int i = 0;
    
    /* AL register contains number of times CPUID(2) should be called */
    uint8_t times = eax & 0xFF;
    
    /* Extract descriptors from registers (Intel manual order) */
    uint8_t* regs = (uint8_t*)&eax;
    for (int j = 1; j < 4; j++) { /* Skip AL byte at index 0 */
        if (regs[j] != 0x00) {
            descriptors[i++] = regs[j];
        }
    }
    
    regs = (uint8_t*)&ebx;
    for (int j = 0; j < 4; j++) {
        if (regs[j] != 0x00) {
            descriptors[i++] = regs[j];
        }
    }
    
    regs = (uint8_t*)&ecx;
    for (int j = 0; j < 4; j++) {
        if (regs[j] != 0x00) {
            descriptors[i++] = regs[j];
        }
    }
    
    regs = (uint8_t*)&edx;
    for (int j = 0; j < 4; j++) {
        if (regs[j] != 0x00) {
            descriptors[i++] = regs[j];
        }
    }
    
    *count = i;
}

/* Use cache information for computation to prevent optimization */
static void cache_aware_computation(struct cache_desc* l1, struct cache_desc* l2) {
    int line_size = l1->line > 0 ? l1->line : 64; /* Default to 64 if not set */
    int array_size = 1024 * 1024; /* 1MB */
    volatile char* buffer = (volatile char*)malloc(array_size);
    volatile int sum = 0;
    
    if (buffer) {
        /* Access with cache line alignment */
        for (int i = 0; i < array_size; i += line_size) {
            sum += buffer[i];
        }
        
        /* Use the sum to prevent dead code elimination */
        printf("Cache-aware computation result: %d (line size: %d)\n", sum, line_size);
        free((void*)buffer);
    }
}

int main() {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptors[64];
    int desc_count = 0;
    int xeon_mp = 0;
    
    struct cache_desc l1_cache = {0, 0, 0, 1, 0};
    struct cache_desc l2_cache = {0, 0, 0, 2, 0};
    
    printf("=== Intel CPUID Cache Descriptor Test ===\n");
    
    /* Step 1: Get CPU family/model/stepping for Xeon MP check */
    cpuid(1, 0, &eax, &ebx, &ecx, &edx);
    
    uint32_t stepping = eax & 0xF;
    uint32_t model = (eax >> 4) & 0xF;
    uint32_t family = (eax >> 8) & 0xF;
    uint32_t extended_model = (eax >> 16) & 0xF;
    uint32_t extended_family = (eax >> 20) & 0xFF;
    
    if (family == 0xF) {
        model += (extended_model << 4);
        family += extended_family;
    }
    
    printf("CPU Family: %u, Model: %u, Stepping: %u\n", 
           family, model, stepping);
    
    xeon_mp = is_xeon_mp(family, model, stepping);
    printf("Xeon MP detection: %s\n", xeon_mp ? "YES" : "NO");
    
    /* Step 2: Call CPUID leaf 0x2 multiple times */
    printf("\nCPUID Leaf 0x2 Cache Descriptors:\n");
    
    /* First call to get iteration count */
    cpuid(2, 0, &eax, &ebx, &ecx, &edx);
    
    uint8_t times = eax & 0xFF;
    printf("CPUID(2) should be called %d time(s)\n", times);
    
    /* Extract descriptors from first call */
    extract_descriptors(eax, ebx, ecx, edx, descriptors, &desc_count);
    
    /* Additional calls if needed (simulating loop) */
    for (int i = 1; i < times; i++) {
        uint32_t eax2, ebx2, ecx2, edx2;
        cpuid(2, i, &eax2, &ebx2, &ecx2, &edx2);
        
        int additional_count = 0;
        uint8_t additional_desc[16];
        extract_descriptors(eax2, ebx2, ecx2, edx2, additional_desc, &additional_count);
        
        /* Append to main descriptor array */
        for (int j = 0; j < additional_count && desc_count < 64; j++) {
            descriptors[desc_count++] = additional_desc[j];
        }
    }
    
    /* Step 3: Process all descriptor bytes */
    printf("\nProcessing %d descriptor bytes:\n", desc_count);
    for (int i = 0; i < desc_count; i++) {
        printf("  Byte %d: 0x%02x\n", i, descriptors[i]);
        process_descriptor(descriptors[i], &l1_cache, &l2_cache, xeon_mp);
    }
    
    /* Step 4: Print final cache information */
    printf("\n=== Final Cache Configuration ===\n");
    if (l1_cache.sizekb > 0) {
        printf("L1 Cache: %dKB, %d-way, %d-byte line\n", 
               l1_cache.sizekb, l1_cache.assoc, l1_cache.line);
    }
    if (l2_cache.sizekb > 0) {
        printf("L2 Cache: %dKB, %d-way, %d-byte line\n", 
               l2_cache.sizekb, l2_cache.assoc, l2_cache.line);
    }
    
    /* Step 5: Perform cache-aware computation */
    printf("\n=== Cache-Aware Computation ===\n");
    cache_aware_computation(&l1_cache, &l2_cache);
    
    /* Step 6: Test specific cases by simulating different descriptors */
    printf("\n=== Testing Specific Uncovered Cases ===\n");
    
    /* Test case 0x49 with Xeon MP condition */
    printf("\nTesting case 0x49 with Xeon MP=%d:\n", xeon_mp);
    process_descriptor(0x49, &l1_cache, &l2_cache, xeon_mp);
    
    /* Test a few other cases */
    printf("\nTesting other target cases:\n");
    uint8_t test_cases[] = {0x0a, 0x2c, 0x39, 0x60, 0x78, 0x87};
    for (int i = 0; i < sizeof(test_cases); i++) {
        struct cache_desc test_l1 = {0, 0, 0, 1, 0};
        struct cache_desc test_l2 = {0, 0, 0, 2, 0};
        process_descriptor(test_cases[i], &test_l1, &test_l2, 0);
    }
    
    return 0;
}
