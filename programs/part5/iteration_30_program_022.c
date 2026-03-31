#include <stdio.h>
#include <stdint.h>

#define SIZE 1000

int main(void) {
    // Declare source and destination arrays
    int src[SIZE];
    int dst[SIZE];
    
    // Initialize source array with pattern
    for (int i = 0; i < SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    // Use pointer arithmetic with post-increment patterns
    // This should generate RTL sequences that the auto-inc-dec pass can optimize
    int *psrc = src;
    int *pdst = dst;
    
    // Loop with clear post-increment memory access patterns
    // The index variable is separate to ensure it's not optimized away
    for (int i = 0; i < SIZE; i++) {
        // Multiple memory operations with post-increment
        // This creates multiple candidate mem_insn structures
        *pdst++ = *psrc++;
        
        // Add another independent access stream to increase chances
        // of hitting the uncovered initialization block
        if (i % 2 == 0) {
            // Additional memory operation that might use different addressing
            int temp = src[i % 100];
            (void)temp; // Use to prevent optimization
        }
    }
    
    // Reset pointers for second loop with different pattern
    psrc = src;
    pdst = dst;
    
    // Another loop with post-decrement pattern
    // This tests the decrement side of the optimization
    for (int i = SIZE - 1; i >= 0; i--) {
        pdst[i] = psrc[i];
        // Simulate pointer decrement through array indexing
        // The compiler might convert this to pointer arithmetic
        if (i > 0) {
            int prev_val = src[i - 1];
            (void)prev_val;
        }
    }
    
    // Compute checksum to prevent dead code elimination
    uint32_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += (uint32_t)dst[i];
    }
    
    printf("Checksum: %u\n", checksum);
    
    // Additional test with volatile to control optimization
    volatile int* volatile_src = src;
    volatile int* volatile_dst = dst;
    
    // Simple pointer traversal that should generate clean RTL
    for (int i = 0; i < 100; i++) {
        *volatile_dst++ = *volatile_src++;
    }
    
    return 0;
}
