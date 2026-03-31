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
    volatile int32_t *volatile_src = src;  /* volatile prevents removal of memory ops */
    volatile int32_t *volatile_dst = dst;
    
    /* Main loop with clear post-increment patterns */
    /* Multiple independent memory access streams */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Pattern 1: Read from src with post-increment */
        int32_t val1 = *volatile_src++;
        
        /* Pattern 2: Write to dst with post-increment */
        *volatile_dst++ = val1;
        
        /* Additional patterns to increase candidate count */
        if (i % 2 == 0) {
            /* Alternate path with different base */
            static int32_t alt_src[ARRAY_SIZE];
            static int32_t alt_dst[ARRAY_SIZE];
            volatile int32_t *alt_p = alt_src;
            volatile int32_t *alt_q = alt_dst;
            
            /* Nested simple pattern */
            *alt_q++ = *alt_p++;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst[i];
    }
    
    /* Use result to prevent optimization */
    printf("Checksum: %lld\n", (long long)checksum);
    
    /* Additional test case with decrement pattern */
    {
        int32_t rev_src[100];
        int32_t rev_dst[100];
        volatile int32_t *p = &rev_src[99];
        volatile int32_t *q = &rev_dst[99];
        
        for (int i = 0; i < 100; i++) {
            *q-- = *p--;  /* Post-decrement pattern */
        }
    }
    
    return 0;
}
