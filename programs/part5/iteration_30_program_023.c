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
    volatile int32_t *volatile src_ptr = src;  /* volatile pointer to volatile data */
    volatile int32_t *volatile dst_ptr = dst;  /* prevents some optimizations */
    
    /* Loop with clear post-increment pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Multiple memory operations with independent pointers */
        int32_t val = *src_ptr++;      /* Read with post-increment - candidate for auto-inc */
        *dst_ptr++ = val;              /* Write with post-increment - another candidate */
        
        /* Additional independent access stream to increase candidates */
        if (i % 2 == 0) {
            /* This creates another memory reference pattern */
            src[ARRAY_SIZE - 1 - i] = val * 2;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst[i];
    }
    
    /* Use checksum to ensure code isn't optimized away */
    printf("Checksum: %lld\n", (long long)checksum);
    
    return 0;
}
