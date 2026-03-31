#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* GCC cpuid.h provides __cpuid and __cpuid_count */
#ifdef __GNUC__
#include <cpuid.h>
#else
/* Fallback for non-GCC compilers */
static void __cpuid(int cpuInfo[4], int function_id) {
    asm volatile (
        "cpuid"
        : "=a"(cpuInfo[0]), "=b"(cpuInfo[1]), "=c"(cpuInfo[2]), "=d"(cpuInfo[3])
        : "a"(function_id)
    );
}

static void __cpuid_count(int cpuInfo[4], int function_id, int subfunction_id) {
    asm volatile (
        "cpuid"
        : "=a"(cpuInfo[0]), "=b"(cpuInfo[1]), "=c"(cpuInfo[2]), "=d"(cpuInfo[3])
        : "a"(function_id), "c"(subfunction_id)
    );
}
#endif

/* Structure matching cache_desc in driver-i386.cc */
struct cache_desc {
    int sizekb;
    int assoc;
    int line;
};

/* Global variables to simulate the driver state */
static struct cache_desc level1 = {0, 0, 0};
static struct cache_desc level2 = {0, 0, 0};
static int xeon_mp = 0;  /* We'll set this to 0 to hit case 0x49 */

/* Function to process cache descriptor bytes - mimics the uncovered switch logic */
static void process_cache_descriptor(uint8_t descriptor) {
    switch (descriptor) {
        case 0x0a:
            level1.sizekb = 8; level1.assoc = 2; level1.line = 32;
            printf("Descriptor 0x%02x: L1 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level1.sizekb, level1.assoc, level1.line);
            break;
        case 0x0c:
            level1.sizekb = 16; level1.assoc = 4; level1.line = 32;
            printf("Descriptor 0x%02x: L1 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level1.sizekb, level1.assoc, level1.line);
            break;
        case 0x0d:
            level1.sizekb = 16; level1.assoc = 4; level1.line = 64;
            printf("Descriptor 0x%02x: L1 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level1.sizekb, level1.assoc, level1.line);
            break;
        case 0x0e:
            level1.sizekb = 24; level1.assoc = 6; level1.line = 64;
            printf("Descriptor 0x%02x: L1 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level1.sizekb, level1.assoc, level1.line);
            break;
        case 0x21:
            level2.sizekb = 256; level2.assoc = 8; level2.line = 64;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x24:
            level2.sizekb = 1024; level2.assoc = 16; level2.line = 64;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x2c:
            level1.sizekb = 32; level1.assoc = 8; level1.line = 64;
            printf("Descriptor 0x%02x: L1 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level1.sizekb, level1.assoc, level1.line);
            break;
        case 0x39:
            level2.sizekb = 128; level2.assoc = 4; level2.line = 64;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x3a:
            level2.sizekb = 192; level2.assoc = 6; level2.line = 64;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x3b:
            level2.sizekb = 128; level2.assoc = 2; level2.line = 64;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x3c:
            level2.sizekb = 256; level2.assoc = 4; level2.line = 64;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x3d:
            level2.sizekb = 384; level2.assoc = 6; level2.line = 64;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x3e:
            level2.sizekb = 512; level2.assoc = 4; level2.line = 64;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x41:
            level2.sizekb = 128; level2.assoc = 4; level2.line = 32;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x42:
            level2.sizekb = 256; level2.assoc = 4; level2.line = 32;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x43:
            level2.sizekb = 512; level2.assoc = 4; level2.line = 32;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x44:
            level2.sizekb = 1024; level2.assoc = 4; level2.line = 32;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x45:
            level2.sizekb = 2048; level2.assoc = 4; level2.line = 32;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x48:
            level2.sizekb = 3072; level2.assoc = 12; level2.line = 64;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x49:
            if (xeon_mp) {
                printf("Descriptor 0x%02x: Skipped (xeon_mp is true)\n", descriptor);
                break;
            }
            level2.sizekb = 4096; level2.assoc = 16; level2.line = 64;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line (xeon_mp=false)\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x4e:
            level2.sizekb = 6144; level2.assoc = 24; level2.line = 64;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x60:
            level1.sizekb = 16; level1.assoc = 8; level1.line = 64;
            printf("Descriptor 0x%02x: L1 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level1.sizekb, level1.assoc, level1.line);
            break;
        case 0x66:
            level1.sizekb = 8; level1.assoc = 4; level1.line = 64;
            printf("Descriptor 0x%02x: L1 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level1.sizekb, level1.assoc, level1.line);
            break;
        case 0x67:
            level1.sizekb = 16; level1.assoc = 4; level1.line = 64;
            printf("Descriptor 0x%02x: L1 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level1.sizekb, level1.assoc, level1.line);
            break;
        case 0x68:
            level1.sizekb = 32; level1.assoc = 4; level1.line = 64;
            printf("Descriptor 0x%02x: L1 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level1.sizekb, level1.assoc, level1.line);
            break;
        case 0x78:
            level2.sizekb = 1024; level2.assoc = 4; level2.line = 64;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x79:
            level2.sizekb = 128; level2.assoc = 8; level2.line = 64;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x7a:
            level2.sizekb = 256; level2.assoc = 8; level2.line = 64;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x7b:
            level2.sizekb = 512; level2.assoc = 8; level2.line = 64;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x7c:
            level2.sizekb = 1024; level2.assoc = 8; level2.line = 64;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x7d:
            level2.sizekb = 2048; level2.assoc = 8; level2.line = 64;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x7f:
            level2.sizekb = 512; level2.assoc = 2; level2.line = 64;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x80:
            level2.sizekb = 512; level2.assoc = 8; level2.line = 64;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x82:
            level2.sizekb = 256; level2.assoc = 8; level2.line = 32;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x83:
            level2.sizekb = 512; level2.assoc = 8; level2.line = 32;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x84:
            level2.sizekb = 1024; level2.assoc = 8; level2.line = 32;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x85:
            level2.sizekb = 2048; level2.assoc = 8; level2.line = 32;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x86:
            level2.sizekb = 512; level2.assoc = 4; level2.line = 64;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        case 0x87:
            level2.sizekb = 1024; level2.assoc = 8; level2.line = 64;
            printf("Descriptor 0x%02x: L2 Cache: %dKB, %d-way, %d-byte line\n", 
                   descriptor, level2.sizekb, level2.assoc, level2.line);
            break;
        default:
            printf("Descriptor 0x%02x: Not a cache descriptor (ignored)\n", descriptor);
            break;
    }
}

/* Simulate CPUID leaf 0x02 descriptor table parsing */
static void simulate_cpuid_leaf2_parsing(void) {
    /* Target descriptor bytes from uncovered lines */
    uint8_t target_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    printf("=== Simulating CPUID Leaf 0x02 Cache Descriptor Parsing ===\n");
    printf("Number of target descriptors: %zu\n\n", sizeof(target_descriptors));
    
    /* Process each target descriptor */
    for (size_t i = 0; i < sizeof(target_descriptors); i++) {
        process_cache_descriptor(target_descriptors[i]);
    }
}

/* Actual CPUID leaf 0x02 call and parsing */
static void real_cpuid_leaf2_parsing(void) {
    int regs[4];
    
    printf("\n=== Real CPUID Leaf 0x02 Call ===\n");
    
    /* Call CPUID leaf 0x02 */
    __cpuid(regs, 0x02);
    
    /* Check if AL (first byte of EAX) is valid */
    uint8_t al = regs[0] & 0xFF;
    
    printf("CPUID Leaf 0x02 returned:\n");
    printf("EAX: 0x%08X, EBX: 0x%08X, ECX: 0x%08X, EDX: 0x%08X\n", 
           regs[0], regs[1], regs[2], regs[3]);
    printf("AL (first byte): 0x%02X\n", al);
    
    /* Early return check - if AL == 1, use different method */
    if (al == 1) {
        printf("AL == 1, using TLB method (early return)\n");
        return;
    }
    
    /* Process descriptor bytes if AL > 1 */
    if (al > 1) {
        printf("Processing %d descriptor bytes:\n", al);
        
        /* Process bytes from EAX (skip AL which is the count) */
        uint8_t *bytes = (uint8_t *)regs;
        for (int i = 1; i <= al && i < 16; i++) {  /* 16 bytes max in 4 registers */
            if (bytes[i] != 0 && (bytes[i] & 0x80) == 0) {  /* Valid descriptor */
                process_cache_descriptor(bytes[i]);
            }
        }
    }
}

/* CPUID leaf 0x04 deterministic cache parameters */
static void cpuid_leaf4_parsing(void) {
    int regs[4];
    int level = 0;
    
    printf("\n=== CPUID Leaf 0x04 Deterministic Cache Parameters ===\n");
    
    do {
        __cpuid_count(regs, 0x04, level);
        
        int cache_type = regs[0] & 0x1F;
        
        printf("Level %d: EAX=0x%08X, EBX=0x%08X, ECX=0x%08X, EDX=0x%08X\n",
               level, regs[0], regs[1], regs[2], regs[3]);
        printf("  Cache Type: %d (%s)\n", cache_type, 
               cache_type == 0 ? "No more caches" : 
               cache_type == 1 ? "Data Cache" :
               cache_type == 2 ? "Instruction Cache" :
               cache_type == 3 ? "Unified Cache" : "Reserved");
        
        if (cache_type == 0) {
            break;
        }
        
        level++;
    } while (1);
}

int main(void) {
    /* Ensure xeon_mp is false to hit case 0x49 */
    xeon_mp = 0;
    
    printf("Cache Descriptor Test Program\n");
    printf("=============================\n\n");
    
    /* Method 1: Simulate all target descriptors */
    simulate_cpuid_leaf2_parsing();
    
    /* Method 2: Real CPUID calls (if supported) */
    real_cpuid_leaf2_parsing();
    
    /* Method 3: CPUID leaf 0x04 */
    cpuid_leaf4_parsing();
    
    /* Print final cache configuration */
    printf("\n=== Final Cache Configuration ===\n");
    printf("L1 Cache: %dKB, %d-way, %d-byte line\n", 
           level1.sizekb, level1.assoc, level1.line);
    printf("L2 Cache: %dKB, %d-way, %d-byte line\n", 
           level2.sizekb, level2.assoc, level2.line);
    
    /* Prevent optimization */
    volatile int dummy = level1.sizekb + level2.sizekb;
    (void)dummy;
    
    return 0;
}
