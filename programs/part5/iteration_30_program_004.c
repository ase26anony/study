#include <stdio.h>
#include <stdint.h>

#define SIZE 1000

int main(void) {
    /* Declare and initialize source and destination arrays */
    int32_t src[SIZE];
    int32_t dst[SIZE];
    
    /* Initialize source array with pattern */
    for (int i = 0; i < SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Use pointer arithmetic to create post-increment patterns */
    int32_t *psrc = src;
    int32_t *pdst = dst;
    
    /* 
     * Loop designed to create RTL patterns for auto-increment optimization.
     * The pointer post-increment operations should generate:
     * 1. Memory load from psrc
     * 2. Memory store to pdst  
     * 3. Increment of both pointers
     * This pattern is ideal for auto-inc-dec pass to combine.
     */
    for (int i = 0; i < SIZE; i++) {
        /* Direct post-increment pattern - most likely to trigger the optimization */
        *pdst++ = *psrc++;
        
        /* Additional independent memory operation to increase candidate count */
        if (i % 2 == 0) {
            /* This creates another memory reference pattern for analysis */
            volatile int32_t temp = src[SIZE - 1 - i];
            (void)temp; /* Prevent unused variable warning */
        }
    }
    
    /* 
     * Second loop with different pattern: array indexing with explicit increment
     * This provides alternative RTL patterns for the pass to analyze
     */
    int32_t src2[SIZE];
    int32_t dst2[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        src2[i] = i * 2;
    }
    
    int idx = 0;
    while (idx < SIZE) {
        /* Array access followed by increment - another candidate pattern */
        dst2[idx] = src2[idx];
        idx++;  /* Post-increment of index variable */
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst[i];
        checksum += dst2[i];
    }
    
    printf("Checksum: %lld\n", (long long)checksum);
    
    return 0;
}
