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
        
        /* Simple operation on the value */
        val = val * 2 + 5;
        
        /* Memory store to dst with implicit post-increment */
        *pdst = val;
        pdst++;  /* This increment should be adjacent to the store in RTL */
    }
    
    /* Alternative: Direct pointer arithmetic in loop */
    int32_t *psrc2 = src;
    int32_t *pdst2 = dst;
    for (int i = 0; i < SIZE; i++) {
        /* Combined access and increment - classic post-increment pattern */
        *pdst2++ = *psrc2++ * 3 - 7;
    }
    
    /* Third pattern: Array indexing with separate increment */
    int32_t *psrc3 = src;
    int32_t *pdst3 = dst;
    for (int i = 0; i < SIZE; i++) {
        /* Force separate but adjacent operations */
        int32_t temp = psrc3[i];  /* Array index form */
        pdst3[i] = temp + 10;     /* Another array index form */
        /* The optimizer should convert these to pointer + offset forms */
    }
    
    /* Fourth pattern: Multiple independent streams */
    int32_t src2[SIZE];
    int32_t dst2[SIZE];
    int32_t *p1 = src;
    int32_t *p2 = src2;
    int32_t *q1 = dst;
    int32_t *q2 = dst2;
    
    for (int i = 0; i < SIZE; i++) {
        /* Multiple memory operations with their own pointers */
        *q1++ = *p1++ + 1;
        *q2++ = *p2++ - 1;
    }
    
    /* Calculate checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst[i];
        checksum += dst2[i];
    }
    
    printf("Checksum: %lld\n", (long long)checksum);
    
    return 0;
}
