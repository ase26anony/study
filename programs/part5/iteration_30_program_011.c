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
    
    /* Use pointer arithmetic to create post-increment patterns */
    int32_t *psrc = src;
    int32_t *pdst = dst;
    
    /* 
     * Loop with clear post-increment pattern.
     * This should generate RTL with memory access followed by register increment.
     */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Memory load from src with implicit post-increment */
        int32_t val = *psrc;
        
        /* Simple operation on loaded value */
        val = val + 5;
        
        /* Memory store to dst with implicit post-increment */
        *pdst = val;
        
        /* Explicit pointer increments - these should be candidates
           for auto-increment addressing mode combination */
        psrc++;
        pdst++;
    }
    
    /* Alternative loop with different pattern to increase coverage */
    /* Reset pointers for second operation */
    psrc = src;
    pdst = dst;
    
    /* Loop with array indexing that may also trigger the optimization */
    for (int i = 0; i < ARRAY_SIZE / 2; i++) {
        /* Access with index that gets incremented */
        dst[i] = src[i] * 2;
        
        /* Additional independent memory stream */
        dst[ARRAY_SIZE - 1 - i] = src[ARRAY_SIZE - 1 - i] / 2;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst[i];
    }
    
    printf("Checksum: %lld\n", (long long)checksum);
    
    return 0;
}
