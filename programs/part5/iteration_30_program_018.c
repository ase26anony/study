#include <stdio.h>
#include <stdint.h>

#define SIZE 1000

int main(void) {
    // Declare and initialize source and destination arrays
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
    
    // Simple loop with clear post-increment pattern
    // The pointer increments happen after the memory access
    for (int i = 0; i < SIZE; i++) {
        // Memory load from src, then pointer increment
        int32_t val = *psrc++;
        
        // Simple arithmetic operation to prevent elimination
        val = val + 1;
        
        // Memory store to dst, then pointer increment
        *pdst++ = val;
    }
    
    // Add a second independent access pattern to increase chances
    // This creates another candidate for the auto-inc-dec pass
    int32_t *psrc2 = src;
    int32_t *pdst2 = dst + SIZE/2;  // Different destination offset
    
    // Another loop with similar pattern
    for (int i = 0; i < SIZE/2; i++) {
        int32_t val2 = *psrc2++;
        *pdst2++ = val2 * 2;
    }
    
    // Compute checksum to prevent dead code elimination
    int64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst[i];
    }
    
    // Print result to ensure code isn't optimized away
    printf("Checksum: %lld\n", (long long)checksum);
    
    return 0;
}
