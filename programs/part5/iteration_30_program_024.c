#include <stdio.h>
#include <stdint.h>

#define ARRAY_SIZE 1000

int main(void) {
    /* Declare source and destination arrays */
    int32_t src[ARRAY_SIZE];
    int32_t dst[ARRAY_SIZE];
    
    /* Initialize source array with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Clear destination array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        dst[i] = 0;
    }
    
    /* 
     * KEY PATTERN: Pointer traversal with post-increment
     * This creates the memory access + increment pattern that
     * auto-inc-dec pass looks for
     */
    int32_t *psrc = src;
    int32_t *pdst = dst;
    
    /* 
     * Simple loop with clear post-increment pattern
     * The memory access and pointer increment are adjacent operations
     */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Memory load from src, store to dst with post-increment */
        *pdst++ = *psrc++;
        
        /* 
         * Additional independent memory stream to increase chances
         * of hitting the uncovered code path
         */
        if (i % 2 == 0) {
            /* Another memory operation that might create a candidate */
            volatile int32_t temp = src[ARRAY_SIZE - 1 - i];
            (void)temp; /* Use temp to avoid unused variable warning */
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst[i];
    }
    
    /* Use the result to ensure code isn't optimized away */
    printf("Checksum: %lld\n", (long long)checksum);
    
    /* Expected checksum: sum of i*3+1 for i=0..999 */
    /* (0*3+1) + (1*3+1) + ... + (999*3+1) = 1000 + 3*(999*1000/2) = 1000 + 3*499500 = 1500500 */
    printf("Expected: 1500500\n");
    
    return (checksum == 1500500) ? 0 : 1;
}
