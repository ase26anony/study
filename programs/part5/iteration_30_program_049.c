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
     * Loop designed to create RTL patterns for auto-increment optimization.
     * The pointer post-increment operations should generate memory accesses
     * followed by register increments that the auto-inc-dec pass can combine.
     */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Multiple memory operations with clear increment patterns */
        *pdst++ = *psrc++;      /* Primary pattern: load then increment, store then increment */
        
        /* Additional independent memory stream to increase candidate matches */
        if (i % 2 == 0) {
            /* Create another memory access pattern */
            volatile int32_t temp = src[ARRAY_SIZE - 1 - i];
            (void)temp;  /* Use temp to prevent optimization */
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst[i];
    }
    
    printf("Checksum: %lld\n", (long long)checksum);
    
    /* Additional test case with decrement pattern */
    int32_t *psrc2 = &src[ARRAY_SIZE - 1];
    int32_t *pdst2 = &dst[ARRAY_SIZE - 1];
    
    for (int i = 0; i < ARRAY_SIZE / 2; i++) {
        *pdst2-- = *psrc2--;  /* Post-decrement pattern */
    }
    
    /* Final checksum */
    checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %lld\n", (long long)checksum);
    
    return 0;
}
