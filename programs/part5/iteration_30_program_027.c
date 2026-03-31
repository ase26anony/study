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
    // This should generate RTL sequences with memory access followed by register increment
    int32_t *psrc = src;
    int32_t *pdst = dst;
    
    // Loop with clear post-increment pattern
    // The auto-inc-dec pass should recognize these as candidates
    for (int i = 0; i < SIZE; i++) {
        // Multiple memory operations to increase chances of hitting the uncovered code
        *pdst = *psrc;      // Load from src, store to dst
        
        // Post-increment operations - these should be adjacent to memory ops in RTL
        psrc++;
        pdst++;
        
        // Add another independent access pattern to create more candidates
        // This creates additional mem_insn structures for the pass to analyze
        if (i % 2 == 0) {
            // Another potential auto-increment candidate
            src[i] = src[i] + 1;
        }
    }
    
    // Compute checksum to prevent dead code elimination
    int32_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst[i];
    }
    
    // Also use the modified src elements
    for (int i = 0; i < SIZE; i += 2) {
        checksum += src[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
