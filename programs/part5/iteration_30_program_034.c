#include <stdio.h>
#include <stdint.h>

#define SIZE 1000

int main(void) {
    // Declare source and destination arrays
    int32_t src[SIZE];
    int32_t dst[SIZE];
    
    // Initialize source array with pattern
    for (int i = 0; i < SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    // Use pointer arithmetic to create post-increment patterns
    // This should generate RTL with memory accesses followed by register increments
    int32_t *psrc = src;
    int32_t *pdst = dst;
    
    // Loop with clear post-increment pattern
    // The pointer increments happen after the memory access
    for (int i = 0; i < SIZE; i++) {
        // Memory load from src, then increment pointer
        int32_t val = *psrc++;
        
        // Simple operation on the value
        val = val + 7;
        
        // Memory store to dst, then increment pointer
        *pdst++ = val;
    }
    
    // Add another independent memory operation stream
    // This creates more candidate mem_insn structures
    int32_t src2[SIZE];
    int32_t dst2[SIZE];
    
    // Initialize second source
    for (int i = 0; i < SIZE; i++) {
        src2[i] = i * 5 + 2;
    }
    
    int32_t *psrc2 = src2;
    int32_t *pdst2 = dst2;
    
    // Second loop with similar pattern
    for (int i = 0; i < SIZE; i++) {
        // Multiple memory operations in same iteration
        int32_t val1 = *psrc2++;
        int32_t val2 = *psrc2++;  // Second access with same pointer
        
        *pdst2++ = val1 + val2;
    }
    
    // Compute checksum to prevent dead code elimination
    int64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst[i];
        checksum += dst2[i];
    }
    
    printf("Checksum: %lld\n", (long long)checksum);
    
    return 0;
}
