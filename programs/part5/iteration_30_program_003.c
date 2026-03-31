#include <stdio.h>
#include <stdint.h>

#define SIZE 1000

int main(void) {
    /* Declare source and destination arrays */
    int32_t src[SIZE];
    int32_t dst[SIZE];
    
    /* Initialize source array with pattern */
    for (int i = 0; i < SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Use pointer arithmetic to create post-increment patterns */
    int32_t *psrc = src;
    int32_t *pdst = dst;
    
    /* Loop with clear post-increment memory access pattern */
    for (int i = 0; i < SIZE; i++) {
        /* Memory load from src with implicit post-increment */
        int32_t val = *psrc;
        
        /* Simple operation on loaded value */
        val = val + 5;
        
        /* Memory store to dst with implicit post-increment */
        *pdst = val;
        
        /* Explicit pointer increments - these should be adjacent to memory ops in RTL */
        psrc++;
        pdst++;
        
        /* Alternative form that also creates good patterns:
         * *pdst++ = *psrc++ + 5;
         */
    }
    
    /* Second loop with different pattern to increase coverage */
    /* Reset pointers for reverse traversal */
    psrc = &src[SIZE-1];
    pdst = &dst[SIZE-1];
    
    /* Post-decrement pattern */
    for (int i = 0; i < SIZE; i++) {
        *pdst = *psrc - 3;
        psrc--;
        pdst--;
    }
    
    /* Third example: Array indexing with separate index variable */
    int32_t src2[SIZE];
    int32_t dst2[SIZE];
    int idx = 0;
    
    for (int i = 0; i < SIZE; i++) {
        /* This creates: MEM[base + offset], then base = base + constant */
        dst2[idx] = src2[idx] * 2;
        idx = idx + 1;  /* Simple increment adjacent to memory access */
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst[i];
    }
    
    printf("Checksum: %lld\n", (long long)checksum);
    
    return 0;
}
