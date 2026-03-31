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
    volatile int32_t *volatile psrc = src;  /* volatile pointer to volatile data */
    volatile int32_t *volatile pdst = dst;  /* prevents elimination of memory ops */
    
    /* Loop with clear post-increment pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Memory load from src with implied post-increment */
        int32_t val = *psrc;
        psrc = psrc + 1;  /* Explicit increment after load */
        
        /* Memory store to dst with implied post-increment */
        *pdst = val;
        pdst = pdst + 1;  /* Explicit increment after store */
    }
    
    /* Alternative: Direct pointer post-increment in loop */
    /* Reset pointers */
    int32_t *psrc2 = src;
    int32_t *pdst2 = dst;
    
    /* Classic post-increment pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *pdst2++ = *psrc2++;  /* Direct post-increment in expression */
    }
    
    /* Third pattern: Array indexing with separate increment */
    int32_t *psrc3 = src;
    int32_t *pdst3 = dst;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Access with offset 0, then increment */
        dst[i] = src[i];      /* Creates base+offset addressing */
        /* The index 'i' is incremented in loop update */
    }
    
    /* Fourth pattern: Multiple independent streams */
    int32_t src2[ARRAY_SIZE];
    int32_t dst2[ARRAY_SIZE];
    int32_t *p1 = src;
    int32_t *p2 = src2;
    int32_t *q1 = dst;
    int32_t *q2 = dst2;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Two loads with post-increment */
        int32_t v1 = *p1++;
        int32_t v2 = *p2++;
        
        /* Two stores with post-increment */
        *q1++ = v1;
        *q2++ = v2;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst[i];
        checksum += dst2[i];
    }
    
    printf("Checksum: %lld\n", (long long)checksum);
    return 0;
}
