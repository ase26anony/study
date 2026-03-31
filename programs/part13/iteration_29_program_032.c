#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <cpuid.h>

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
            printf("Case 0x49: L2 Cache - 4096KB, 16-way, 64B line (xeon_mp = %d)\n", xeon_mp);
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
            // Not one of our target cases
            break;
    }
}

// Simulate CPUID leaf 0x02 with specific descriptor bytes
void simulate_cpuid_leaf2() {
    printf("\n=== Simulating CPUID Leaf 0x02 with target descriptors ===\n");
    
    // Array of all target descriptor bytes from uncovered lines
    uint8_t target_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
        0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
        0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    // Simulate CPUID leaf 0x02 returning multiple descriptors
    // First byte (AL) indicates number of valid descriptor bytes
    uint32_t eax, ebx, ecx, edx;
    
    // We'll simulate 6 iterations with different descriptor combinations
    for (int iter = 0; iter < 6; iter++) {
        // Reset cache descriptors
        memset(&level1, 0, sizeof(level1));
        memset(&level2, 0, sizeof(level2));
        
        // Simulate different CPUID results
        switch (iter) {
            case 0:
                // Simulate AL = 0x03 (3 valid descriptor bytes)
                eax = 0x03000000 | (target_descriptors[0] << 16) | 
                      (target_descriptors[1] << 8) | target_descriptors[2];
                ebx = 0;
                ecx = 0;
                edx = 0;
                break;
            case 1:
                // Simulate more descriptors across registers
                eax = 0x06000000 | (target_descriptors[3] << 16) | 
                      (target_descriptors[4] << 8) | target_descriptors[5];
                ebx = (target_descriptors[6] << 24) | (target_descriptors[7] << 16) |
                      (target_descriptors[8] << 8) | target_descriptors[9];
                ecx = (target_descriptors[10] << 24) | (target_descriptors[11] << 16);
                edx = 0;
                break;
            case 2:
                // Test the 0x49 case with xeon_mp = 0
                xeon_mp = 0;
                eax = 0x01000000 | target_descriptors[19];  // 0x49
                ebx = 0;
                ecx = 0;
                edx = 0;
                break;
            case 3:
                // Test the 0x49 case with xeon_mp = 1 (should skip)
                xeon_mp = 1;
                eax = 0x01000000 | target_descriptors[19];  // 0x49
                ebx = 0;
                ecx = 0;
                edx = 0;
                break;
            case 4:
                // Mix of L1 and L2 cache descriptors
                eax = 0x04000000 | (target_descriptors[20] << 16) | 
                      (target_descriptors[21] << 8) | target_descriptors[22];
                ebx = (target_descriptors[23] << 24);
                ecx = 0;
                edx = 0;
                break;
            case 5:
                // Remaining descriptors
                eax = 0x0A000000 | (target_descriptors[24] << 16) | 
                      (target_descriptors[25] << 8) | target_descriptors[26];
                ebx = (target_descriptors[27] << 24) | (target_descriptors[28] << 16) |
                      (target_descriptors[29] << 8) | target_descriptors[30];
                ecx = (target_descriptors[31] << 24) | (target_descriptors[32] << 16) |
                      (target_descriptors[33] << 8) | target_descriptors[34];
                edx = (target_descriptors[35] << 24) | (target_descriptors[36] << 16) |
                      (target_descriptors[37] << 8) | target_descriptors[38];
                break;
        }
        
        printf("\n--- Simulation Iteration %d ---\n", iter);
        printf("EAX: 0x%08X, EBX: 0x%08X, ECX: 0x%08X, EDX: 0x%08X\n", 
               eax, ebx, ecx, edx);
        
        // Process the descriptor bytes like the original code does
        uint8_t *regs[] = {(uint8_t*)&eax, (uint8_t*)&ebx, 
                          (uint8_t*)&ecx, (uint8_t*)&edx};
        int total_bytes = (eax >> 24) & 0xFF;  // AL contains byte count
        
        if (total_bytes > 0 && total_bytes <= 16) {
            int bytes_processed = 0;
            for (int reg = 0; reg < 4 && bytes_processed < total_bytes; reg++) {
                for (int byte = 0; byte < 4 && bytes_processed < total_bytes; byte++) {
                    uint8_t descriptor = regs[reg][byte];
                    if (descriptor != 0x00 && descriptor != 0xFF) {
                        process_cache_descriptor(descriptor);
                    }
                    bytes_processed++;
                }
            }
        }
    }
}

// Real CPUID calls for leaf 0x02 and leaf 0x04
void call_real_cpuid() {
    printf("\n=== Calling Real CPUID ===\n");
    
    uint32_t eax, ebx, ecx, edx;
    
    // Check if CPUID leaf 0x02 is supported
    __cpuid(0, eax, ebx, ecx, edx);
    uint32_t max_leaf = eax;
    
    if (max_leaf >= 2) {
        printf("CPUID Leaf 0x02 supported (max leaf: 0x%x)\n", max_leaf);
        
        // Call CPUID leaf 0x02
        __cpuid(2, eax, ebx, ecx, edx);
        
        printf("Leaf 0x02 results:\n");
        printf("  EAX: 0x%08X\n", eax);
        printf("  EBX: 0x%08X\n", ebx);
        printf("  ECX: 0x%08X\n", ecx);
        printf("  EDX: 0x%08X\n", edx);
        
        // Check first byte (AL) - should be > 1 for descriptor table
        uint8_t descriptor_count = (eax >> 24) & 0xFF;
        printf("  Descriptor bytes in registers: %d\n", descriptor_count);
        
        if (descriptor_count > 0 && descriptor_count <= 16) {
            // Process descriptor bytes
            uint8_t *regs[] = {(uint8_t*)&eax, (uint8_t*)&ebx, 
                              (uint8_t*)&ecx, (uint8_t*)&edx};
            int bytes_processed = 0;
            
            for (int reg = 0; reg < 4 && bytes_processed < descriptor_count; reg++) {
                for (int byte = 0; byte < 4 && bytes_processed < descriptor_count; byte++) {
                    uint8_t descriptor = regs[reg][byte];
                    if (descriptor != 0x00 && descriptor != 0xFF) {
                        printf("  Processing descriptor: 0x%02x\n", descriptor);
                        process_cache_descriptor(descriptor);
                    }
                    bytes_processed++;
                }
            }
        }
    } else {
        printf("CPUID Leaf 0x02 not supported\n");
    }
    
    // Try CPUID leaf 0x04 (deterministic cache parameters)
    if (max_leaf >= 4) {
        printf("\nCPUID Leaf 0x04 (Deterministic Cache Parameters):\n");
        
        for (int i = 0; ; i++) {
            __cpuid_count(4, i, eax, ebx, ecx, edx);
            
            int cache_type = eax & 0x1F;
            if (cache_type == 0) {
                printf("  No more caches at index %d\n", i);
                break;
            }
            
            printf("  Cache %d: type=%d, level=%d\n", 
                   i, cache_type, (eax >> 5) & 0x7);
            
            // Check if this is one of our target cache configurations
            // by comparing with known values from the switch cases
        }
    }
}

// Direct simulation using inline assembly to force specific paths
void simulate_with_asm() {
    printf("\n=== Direct Simulation with Inline Assembly ===\n");
    
    // We'll use inline assembly to simulate the CPUID results
    // This forces the compiler to generate code that includes the switch logic
    
    volatile uint32_t fake_eax, fake_ebx, fake_ecx, fake_edx;
    
    // Test specific cases using inline assembly to prevent optimization
    for (int i = 0; i < 5; i++) {
        // Reset cache descriptors
        memset(&level1, 0, sizeof(level1));
        memset(&level2, 0, sizeof(level2));
        
        switch (i) {
            case 0:
                // Test case 0x49 with xeon_mp = 0
                xeon_mp = 0;
                fake_eax = 0x01490000;  // AL=1, descriptor=0x49
                fake_ebx = 0;
                fake_ecx = 0;
                fake_edx = 0;
                break;
            case 1:
                // Test multiple L1 cache descriptors
                fake_eax = 0x030A0C0D;  // AL=3, descriptors: 0x0A, 0x0C, 0x0D
                fake_ebx = 0;
                fake_ecx = 0;
                fake_edx = 0;
                break;
            case 2:
                // Test L2 cache descriptors
                fake_eax = 0x02212400;  // AL=2, descriptors: 0x21, 0x24
                fake_ebx = 0;
                fake_ecx = 0;
                fake_edx = 0;
                break;
            case 3:
                // Test the 0x2c case (L1 cache)
                fake_eax = 0x012C0000;  // AL=1, descriptor=0x2C
                fake_ebx = 0;
                fake_ecx = 0;
                fake_edx = 0;
                break;
            case 4:
                // Test a mix
                fake_eax = 0x04606667;  // AL=4, descriptors: 0x60, 0x66, 0x67
                fake_ebx = 0x68000000;  // descriptor: 0x68
                fake_ecx = 0;
                fake_edx = 0;
                break;
        }
        
        printf("\nSimulation %d:\n", i);
        
        // Process the fake CPUID results
        uint8_t descriptor_count = (fake_eax >> 24) & 0xFF;
        uint8_t *regs[] = {(uint8_t*)&fake_eax, (uint8_t*)&fake_ebx, 
                          (uint8_t*)&fake_ecx, (uint8_t*)&fake_edx};
        
        int bytes_processed = 0;
        for (int reg = 0; reg < 4 && bytes_processed < descriptor_count; reg++) {
            for (int byte = 0; byte < 4 && bytes_processed < descriptor_count; byte++) {
                uint8_t descriptor = regs[reg][byte];
                if (descriptor != 0x00 && descriptor != 0xFF) {
                    // Use asm volatile to ensure the switch is compiled
                    asm volatile("" : : "r"(descriptor));  // Prevent optimization
                    process_cache_descriptor(descriptor);
                }
                bytes_processed++;
            }
        }
    }
}

int main() {
    printf("Cache Descriptor Coverage Test Program\n");
    printf("======================================\n");
    
    // Method 1: Simulate CPUID leaf 0x02 with target descriptors
    simulate_cpuid_leaf2();
    
    // Method 2: Make real CPUID calls
    call_real_cpuid();
    
    // Method 3: Direct simulation with inline assembly
    simulate_with_asm();
    
    // Summary
    printf("\n=== Summary ===\n");
    printf("Final L1 Cache: %dKB, %d-way, %dB line\n", 
           level1.sizekb, level1.assoc, level1.line);
    printf("Final L2 Cache: %dKB, %d-way, %dB line\n", 
           level2.sizekb, level2.assoc, level2.line);
    
    return 0;
}
