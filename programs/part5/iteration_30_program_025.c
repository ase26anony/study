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
        /* Memory load from src with post-increment */
        int32_t val = *psrc++;
        
        /* Simple operation on loaded value */
        val = val + 7;
        
        /* Memory store to dst with post-increment */
        *pdst++ = val;
    }
    
    /* 
     * Second loop with different pattern to increase chances.
     * Using array indexing with explicit increment.
     */
    int32_t sum1 = 0;
    int32_t sum2 = 0;
    int idx1 = 0;
    int idx2 = 0;
    
    for (int i = 0; i < ARRAY_SIZE / 2; i++) {
        /* Two independent memory access streams */
        sum1 += dst[idx1];
        idx1++;  /* Increment after use - post-increment pattern */
        
        sum2 += src[idx2];
        idx2++;  /* Another post-increment pattern */
    }
    
    /* 
     * Third example: Reverse copy with post-decrement pattern
     */
    int32_t *psrc_end = &src[ARRAY_SIZE - 1];
    int32_t *pdst_end = &dst[ARRAY_SIZE - 1];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *pdst_end-- = *psrc_end-- + 2;
    }
    
    /* Compute final checksum to prevent dead code elimination */
    int32_t final_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_sum += dst[i];
    }
    
    printf("Final checksum: %d\n", (int)final_sum);
    printf("Partial sums: %d, %d\n", (int)sum1, (int)sum2);
    
    return 0;
}
