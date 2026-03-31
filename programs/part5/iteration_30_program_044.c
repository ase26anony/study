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
        
        /* Simple operation on loaded value */
        val = val + 5;
        
        /* Memory store to dst with implicit post-increment */
        *pdst = val;
        
        /* Explicit pointer increments - these should be adjacent to memory ops in RTL */
        psrc++;
        pdst++;
    }
    
    /* Alternative: Multiple independent memory streams */
    int32_t src2[SIZE];
    int32_t dst2[SIZE];
    int32_t *p1 = src2;
    int32_t *p2 = dst2;
    
    /* Initialize second set */
    for (int i = 0; i < SIZE; i++) {
        src2[i] = i * 2;
    }
    
    /* Another loop with pointer arithmetic */
    for (int i = 0; i < SIZE; i++) {
        *p2 = *p1 + 7;
        p1++;  /* Post-increment candidate */
        p2++;  /* Post-increment candidate */
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
