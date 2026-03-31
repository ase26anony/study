#include <stdio.h>
#include <stdint.h>

#define SIZE 1000

int main(void) {
    // Declare and initialize arrays
    int src[SIZE];
    int dst[SIZE];
    
    // Initialize source array with pattern
    for (int i = 0; i < SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    // Use pointer arithmetic with post-increment patterns
    // This creates ideal RTL sequences for auto-inc-dec optimization
    int *psrc = src;
    int *pdst = dst;
    
    // Simple loop with clear post-increment pattern
    // The pointer increments happen after the memory access
    for (int i = 0; i < SIZE; i++) {
        // Memory load from src with implied post-increment
        int val = *psrc;
        
        // Memory store to dst with implied post-increment
        *pdst = val;
        
        // Explicit pointer increments - these should be adjacent to
        // the memory operations in RTL
        psrc++;
        pdst++;
    }
    
    // Alternative: Multiple independent memory streams
    // This creates more candidate mem_insn structures
    int src2[SIZE];
    int dst2[SIZE];
    
    // Initialize second source
    for (int i = 0; i < SIZE; i++) {
        src2[i] = i * 2 + 5;
    }
    
    int *psrc2 = src2;
    int *pdst2 = dst2;
    
    // Second loop with similar pattern
    for (int i = 0; i < SIZE; i++) {
        // Two memory loads and two stores per iteration
        // Increases chance of hitting the uncovered code
        int val1 = *psrc2;
        int val2 = *pdst;  // Using dst from previous loop as source
        
        *pdst2 = val1 + val2;
        
        psrc2++;
        pdst2++;
    }
    
    // Compute checksum to prevent dead code elimination
    uint64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst[i];
        checksum += dst2[i];
    }
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
