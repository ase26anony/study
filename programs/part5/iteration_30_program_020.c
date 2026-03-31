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
        /* Memory load from src, then pointer increment */
        int32_t val = *psrc;
        psrc++;
        
        /* Memory store to dst, then pointer increment */
        *pdst = val;
        pdst++;
        
        /* 
         * Alternative form that also creates good patterns:
         * *pdst++ = *psrc++;
         * But using separate statements gives clearer RTL separation
         */
    }
    
    /* 
     * Second loop with different pattern to increase chances.
     * Using array indexing with post-increment of index.
     */
    int32_t dst2[ARRAY_SIZE];
    int j = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        dst2[j] = src[i];
        j++;  /* Post-increment of index variable */
    }
    
    /* 
     * Third pattern: Multiple independent memory streams.
     * This creates more candidate mem_insn structures.
     */
    int32_t tmp[ARRAY_SIZE];
    int32_t *p1 = src;
    int32_t *p2 = dst;
    int32_t *p3 = tmp;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Three independent memory operations with increments */
        int32_t a = *p1;
        p1++;
        
        *p2 = a;
        p2++;
        
        *p3 = a * 2;
        p3++;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst[i];
        checksum += dst2[i];
        checksum += tmp[i];
    }
    
    printf("Checksum: %lld\n", (long long)checksum);
    
    return 0;
}
