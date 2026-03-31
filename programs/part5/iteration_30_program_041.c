#include <stdio.h>
#include <stdint.h>

#define ARRAY_SIZE 1000

int main(void) {
    /* Declare source and destination arrays */
    int src[ARRAY_SIZE];
    int dst[ARRAY_SIZE];
    
    /* Initialize source array with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Use pointer arithmetic to create post-increment patterns */
    int *psrc = src;
    int *pdst = dst;
    
    /* Loop with clear post-increment pattern
     * This should generate RTL with memory access followed by register increment */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Memory load from src, then pointer increment */
        int val = *psrc;
        psrc++;  /* Post-increment after load */
        
        /* Memory store to dst, then pointer increment */
        *pdst = val;
        pdst++;  /* Post-increment after store */
    }
    
    /* Alternative: Direct array access with index increment
     * This creates another pattern for the optimizer to analyze */
    int sum1 = 0;
    int idx = 0;
    while (idx < ARRAY_SIZE) {
        sum1 += dst[idx];  /* Memory load */
        idx++;             /* Index increment - potential post-increment candidate */
    }
    
    /* Another pattern: Multiple independent memory streams */
    int temp[ARRAY_SIZE];
    int *p1 = dst;
    int *p2 = temp;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *p2 = *p1 + 5;  /* Load from dst, store to temp */
        p1++;
        p2++;
    }
    
    /* Compute checksum to prevent dead code elimination */
    uint32_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += (uint32_t)dst[i];
        checksum += (uint32_t)temp[i];
    }
    
    printf("Checksum: %u\n", checksum);
    printf("Sum1: %d\n", sum1);
    
    return 0;
}
