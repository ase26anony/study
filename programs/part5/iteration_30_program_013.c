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
    
    /* Loop with clear post-increment pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Memory load from src with implicit post-increment */
        int32_t val = *psrc;
        
        /* Simple operation on loaded value */
        val = val + 5;
        
        /* Memory store to dst with implicit post-increment */
        *pdst = val;
        
        /* Explicit pointer increments - these should be adjacent to memory ops in RTL */
        psrc++;
        pdst++;
    }
    
    /* Alternative: Multiple independent memory streams */
    int32_t tmp[ARRAY_SIZE];
    int32_t *p1 = src;
    int32_t *p2 = dst;
    int32_t *p3 = tmp;
    
    /* Second loop with three parallel memory streams */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Three memory operations with post-increment pattern */
        *p3 = *p1 + *p2;
        p1++;
        p2++;
        p3++;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst[i];
        checksum += tmp[i];
    }
    
    printf("Checksum: %lld\n", (long long)checksum);
    
    return 0;
}
