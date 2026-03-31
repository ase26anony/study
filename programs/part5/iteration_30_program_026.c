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
    
    /* Loop with clear post-increment pattern */
    for (int i = 0; i < SIZE; i++) {
        /* Memory load from src with implicit post-increment */
        int32_t val = *psrc;
        psrc++;  /* This increment should be adjacent to the load in RTL */
        
        /* Simple arithmetic operation on the value */
        val = val * 2 + 5;
        
        /* Memory store to dst with implicit post-increment */
        *pdst = val;
        pdst++;  /* This increment should be adjacent to the store in RTL */
    }
    
    /* Alternative: Direct post-increment in array access (even clearer pattern) */
    int32_t *psrc2 = src;
    int32_t *pdst2 = dst;
    
    /* This creates the classic *p++ pattern */
    for (int i = 0; i < SIZE; i++) {
        *pdst2++ = *psrc2++ * 3 - 7;
    }
    
    /* Third pattern: Multiple independent memory streams */
    int32_t src2[SIZE];
    int32_t dst2[SIZE];
    int32_t *p1 = src;
    int32_t *p2 = src2;
    int32_t *q1 = dst;
    int32_t *q2 = dst2;
    
    /* Initialize second source */
    for (int i = 0; i < SIZE; i++) {
        src2[i] = i * 2 + 3;
    }
    
    /* Multiple memory operations in same loop - increases chances of hitting the block */
    for (int i = 0; i < SIZE; i++) {
        /* Four memory operations with post-increments */
        int32_t v1 = *p1++;
        int32_t v2 = *p2++;
        *q1++ = v1 + v2;
        *q2++ = v1 - v2;
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
