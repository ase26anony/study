#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef __GNUC__
#include <cpuid.h>
#else
// Fallback for non-GCC compilers
static void __cpuid_count(int leaf, int subleaf, int *eax, int *ebx, int *ecx, int *edx) {
    // This is a stub - in real implementation, this would use CPUID instruction
    *eax = *ebx = *ecx = *edx = 0;
}
#endif

// Structure matching the cache descriptor in driver-i386.cc
struct cache_desc {
    int sizekb;
    int assoc;
    int line;
};

// Global variables to track cache levels
struct cache_desc level1 = {0, 0, 0};
struct cache_desc level2 = {0, 0, 0};
int xeon_mp = 0;  // We'll set this to 0 to hit the 0x49 case

// Function to process cache descriptor bytes (mimicking the uncovered logic)
void process_cache_descriptor_byte(unsigned char desc) {
    switch (desc) {
        case 0x0a:
            level1.sizekb = 8; level1.assoc = 2; level1.line = 32;
            printf("Processed 0x0a: L1 - 8KB, 2-way, 32B line\n");
            break;
        case 0x0c:
            level1.sizekb = 16; level1.assoc = 4; level1.line = 32;
            printf("Processed 0x0c: L1 - 16KB, 4-way, 32B line\n");
            break;
        case 0x0d:
            level1.sizekb = 16; level1.assoc = 4; level1.line = 64;
            printf("Processed 0x0d: L1 - 16KB, 4-way, 64B line\n");
            break;
        case 0x0e:
            level1.sizekb = 24; level1.assoc = 6; level1.line = 64;
            printf("Processed 0x0e: L1 - 24KB, 6-way, 64B line\n");
            break;
        case 0x21:
            level2.sizekb = 256; level2.assoc = 8; level2.line = 64;
            printf("Processed 0x21: L2 - 256KB, 8-way, 64B line\n");
            break;
        case 0x24:
            level2.sizekb = 1024; level2.assoc = 16; level2.line = 64;
            printf("Processed 0x24: L2 - 1024KB, 16-way, 64B line\n");
            break;
        case 0x2c:
            level1.sizekb = 32; level1.assoc = 8; level1.line = 64;
            printf("Processed 0x2c: L1 - 32KB, 8-way, 64B line\n");
            break;
        case 0x39:
            level2.sizekb = 128; level2.assoc = 4; level2.line = 64;
            printf("Processed 0x39: L2 - 128KB, 4-way, 64B line\n");
            break;
        case 0x3a:
            level2.sizekb = 192; level2.assoc = 6; level2.line = 64;
            printf("Processed 0x3a: L2 - 192KB, 6-way, 64B line\n");
            break;
        case 0x3b:
            level2.sizekb = 128; level2.assoc = 2; level2.line = 64;
            printf("Processed 0x3b: L2 - 128KB, 2-way, 64B line\n");
            break;
        case 0x3c:
            level2.sizekb = 256; level2.assoc = 4; level2.line = 64;
            printf("Processed 0x3c: L2 - 256KB, 4-way, 64B line\n");
            break;
        case 0x3d:
            level2.sizekb = 384; level2.assoc = 6; level2.line = 64;
            printf("Processed 0x3d: L2 - 384KB, 6-way, 64B line\n");
            break;
        case 0x3e:
            level2.sizekb = 512; level2.assoc = 4; level2.line = 64;
            printf("Processed 0x3e: L2 - 512KB, 4-way, 64B line\n");
            break;
        case 0x41:
            level2.sizekb = 128; level2.assoc = 4; level2.line = 32;
            printf("Processed 0x41: L2 - 128KB, 4-way, 32B line\n");
            break;
        case 0x42:
            level2.sizekb = 256; level2.assoc = 4; level2.line = 32;
            printf("Processed 0x42: L2 - 256KB, 4-way, 32B line\n");
            break;
        case 0x43:
            level2.sizekb = 512; level2.assoc = 4; level2.line = 32;
            printf("Processed 0x43: L2 - 512KB, 4-way, 32B line\n");
            break;
        case 0x44:
            level2.sizekb = 1024; level2.assoc = 4; level2.line = 32;
            printf("Processed 0x44: L2 - 1024KB, 4-way, 32B line\n");
            break;
        case 0x45:
            level2.sizekb = 2048; level2.assoc = 4; level2.line = 32;
            printf("Processed 0x45: L2 - 2048KB, 4-way, 32B line\n");
            break;
        case 0x48:
            level2.sizekb = 3072; level2.assoc = 12; level2.line = 64;
            printf("Processed 0x48: L2 - 3072KB, 12-way, 64B line\n");
            break;
        case 0x49:
            if (xeon_mp)
                break;
            level2.sizekb = 4096; level2.assoc = 16; level2.line = 64;
            printf("Processed 0x49: L2 - 4096KB, 16-way, 64B line (xeon_mp=%d)\n", xeon_mp);
            break;
        case 0x4e:
            level2.sizekb = 6144; level2.assoc = 24; level2.line = 64;
            printf("Processed 0x4e: L2 - 6144KB, 24-way, 64B line\n");
            break;
        case 0x60:
            level1.sizekb = 16; level1.assoc = 8; level1.line = 64;
            printf("Processed 0x60: L1 - 16KB, 8-way, 64B line\n");
            break;
        case 0x66:
            level1.sizekb = 8; level1.assoc = 4; level1.line = 64;
            printf("Processed 0x66: L1 - 8KB, 4-way, 64B line\n");
            break;
        case 0x67:
            level1.sizekb = 16; level1.assoc = 4; level1.line = 64;
            printf("Processed 0x67: L1 - 16KB, 4-way, 64B line\n");
            break;
        case 0x68:
            level1.sizekb = 32; level1.assoc = 4; level1.line = 64;
            printf("Processed 0x68: L1 - 32KB, 4-way, 64B line\n");
            break;
        case 0x78:
            level2.sizekb = 1024; level2.assoc = 4; level2.line = 64;
            printf("Processed 0x78: L2 - 1024KB, 4-way, 64B line\n");
            break;
        case 0x79:
            level2.sizekb = 128; level2.assoc = 8; level2.line = 64;
            printf("Processed 0x79: L2 - 128KB, 8-way, 64B line\n");
            break;
        case 0x7a:
            level2.sizekb = 256; level2.assoc = 8; level2.line = 64;
            printf("Processed 0x7a: L2 - 256KB, 8-way, 64B line\n");
            break;
        case 0x7b:
            level2.sizekb = 512; level2.assoc = 8; level2.line = 64;
            printf("Processed 0x7b: L2 - 512KB, 8-way, 64B line\n");
            break;
        case 0x7c:
            level2.sizekb = 1024; level2.assoc = 8; level2.line = 64;
            printf("Processed 0x7c: L2 - 1024KB, 8-way, 64B line\n");
            break;
        case 0x7d:
            level2.sizekb = 2048; level2.assoc = 8; level2.line = 64;
            printf("Processed 0x7d: L2 - 2048KB, 8-way, 64B line\n");
            break;
        case 0x7f:
            level2.sizekb = 512; level2.assoc = 2; level2.line = 64;
            printf("Processed 0x7f: L2 - 512KB, 2-way, 64B line\n");
            break;
        case 0x80:
            level2.sizekb = 512; level2.assoc = 8; level2.line = 64;
            printf("Processed 0x80: L2 - 512KB, 8-way, 64B line\n");
            break;
        case 0x82:
            level2.sizekb = 256; level2.assoc = 8; level2.line = 32;
            printf("Processed 0x82: L2 - 256KB, 8-way, 32B line\n");
            break;
        case 0x83:
            level2.sizekb = 512; level2.assoc = 8; level2.line = 32;
            printf("Processed 0x83: L2 - 512KB, 8-way, 32B line\n");
            break;
        case 0x84:
            level2.sizekb = 1024; level2.assoc = 8; level2.line = 32;
            printf("Processed 0x84: L2 - 1024KB, 8-way, 32B line\n");
            break;
        case 0x85:
            level2.sizekb = 2048; level2.assoc = 8; level2.line = 32;
            printf("Processed 0x85: L2 - 2048KB, 8-way, 32B line\n");
            break;
        case 0x86:
            level2.sizekb = 512; level2.assoc = 4; level2.line = 64;
            printf("Processed 0x86: L2 - 512KB, 4-way, 64B line\n");
            break;
        case 0x87:
            level2.sizekb = 1024; level2.assoc = 8; level2.line = 64;
            printf("Processed 0x87: L2 - 1024KB, 8-way, 64B line\n");
            break;
        default:
            // Ignore other values as in the original code
            break;
    }
}

// Simulate CPUID leaf 0x02 with specific descriptor bytes
void simulate_cpuid_leaf2_descriptors() {
    // All the target descriptor bytes from uncovered lines
    unsigned char target_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    printf("Simulating CPUID leaf 0x02 with %zu target descriptors\n", 
           sizeof(target_descriptors)/sizeof(target_descriptors[0]));
    
    // Simulate the iteration through descriptor bytes
    // In the real code, these would come from EAX, EBX, ECX, EDX registers
    for (size_t i = 0; i < sizeof(target_descriptors)/sizeof(target_descriptors[0]); i++) {
        process_cache_descriptor_byte(target_descriptors[i]);
    }
}

// Test CPUID leaf 0x04 (deterministic cache parameters)
void test_cpuid_leaf4() {
    int eax, ebx, ecx, edx;
    int cache_index = 0;
    
    printf("\nTesting CPUID leaf 0x04 (deterministic cache parameters):\n");
    
    do {
        __cpuid_count(0x04, cache_index, &eax, &ebx, &ecx, &edx);
        
        int cache_type = eax & 0x1F;
        if (cache_type == 0) {
            printf("Cache index %d: No more caches\n", cache_index);
            break;
        }
        
        int cache_level = (eax >> 5) & 0x7;
        int self_initializing = (eax >> 8) & 0x1;
        int fully_associative = (eax >> 9) & 0x1;
        int max_threads = ((eax >> 14) & 0xFFF) + 1;
        int max_cores = ((eax >> 26) & 0x3F) + 1;
        
        int line_size = (ebx & 0xFFF) + 1;
        int partitions = ((ebx >> 12) & 0x3FF) + 1;
        int associativity = ((ebx >> 22) & 0x3FF) + 1;
        
        int sets = ecx + 1;
        
        printf("Cache %d: Type=%d, Level=%d, Line=%dB, Sets=%d, Ways=%d\n",
               cache_index, cache_type, cache_level, line_size, sets, associativity);
        
        cache_index++;
    } while (1);
}

// Test real CPUID leaf 0x02 if available
void test_real_cpuid_leaf2() {
    int eax, ebx, ecx, edx;
    
    printf("\nTesting real CPUID leaf 0x02:\n");
    __cpuid_count(0x02, 0, &eax, &ebx, &ecx, &edx);
    
    // Check if we should use descriptor table method (AL > 1)
    unsigned char al_val = eax & 0xFF;
    printf("CPUID leaf 0x02 returned: EAX=0x%08X, EBX=0x%08X, ECX=0x%08X, EDX=0x%08X\n", 
           eax, ebx, ecx, edx);
    printf("AL (first byte) = 0x%02X\n", al_val);
    
    if (al_val > 1) {
        printf("Using descriptor table method (AL=%d > 1)\n", al_val);
        
        // Extract descriptor bytes from registers
        unsigned char *regs = (unsigned char*)&eax;
        for (int i = 0; i < 4; i++) {
            if (regs[i] != 0 && (regs[i] & 0x80) == 0) { // Valid descriptor
                process_cache_descriptor_byte(regs[i]);
            }
        }
        
        regs = (unsigned char*)&ebx;
        for (int i = 0; i < 4; i++) {
            if (regs[i] != 0 && (regs[i] & 0x80) == 0) {
                process_cache_descriptor_byte(regs[i]);
            }
        }
        
        regs = (unsigned char*)&ecx;
        for (int i = 0; i < 4; i++) {
            if (regs[i] != 0 && (regs[i] & 0x80) == 0) {
                process_cache_descriptor_byte(regs[i]);
            }
        }
        
        regs = (unsigned char*)&edx;
        for (int i = 0; i < 4; i++) {
            if (regs[i] != 0 && (regs[i] & 0x80) == 0) {
                process_cache_descriptor_byte(regs[i]);
            }
        }
    } else if (al_val == 1) {
        printf("Using TLB method (AL=1), not descriptor table\n");
    } else {
        printf("Invalid CPUID leaf 0x02 response (AL=0)\n");
    }
}

int main() {
    printf("=== Cache Descriptor Coverage Test Program ===\n\n");
    
    // Set xeon_mp to 0 to ensure case 0x49 is hit
    xeon_mp = 0;
    printf("xeon_mp flag set to: %d (0 = non-Xeon-MP, will hit case 0x49)\n\n", xeon_mp);
    
    // Method 1: Simulate all target descriptor bytes
    printf("--- Method 1: Simulating target descriptor bytes ---\n");
    simulate_cpuid_leaf2_descriptors();
    
    // Method 2: Test real CPUID leaf 0x02
    printf("\n--- Method 2: Testing real CPUID leaf 0x02 ---\n");
    test_real_cpuid_leaf2();
    
    // Method 3: Test CPUID leaf 0x04
    test_cpuid_leaf4();
    
    // Print final cache configuration
    printf("\n=== Final Cache Configuration ===\n");
    printf("Level 1: %d KB, %d-way, %d byte line\n", 
           level1.sizekb, level1.assoc, level1.line);
    printf("Level 2: %d KB, %d-way, %d byte line\n", 
           level2.sizekb, level2.assoc, level2.line);
    
    // Force compiler to keep variables
    volatile int keep_level1 = level1.sizekb;
    volatile int keep_level2 = level2.sizekb;
    
    return 0;
}
