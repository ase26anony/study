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
        /* Multiple memory operations with post-increment */
        *dst_ptr++ = *src_ptr++;  /* This should generate RTL with mem+inc pattern */
        
        /* Add another independent access stream to increase chances */
        if (i % 2 == 0) {
            /* Additional memory operation that might use different addressing */
            dst[i] = src[i] + 1;
        }
    }
    
    /* Alternative: Simple array copy with index increment */
    /* This creates another candidate pattern */
    {
        int32_t *p1 = src;
        int32_t *p2 = dst;
        for (int i = 0; i < ARRAY_SIZE / 2; i++) {
            p2[i] = p1[i];  /* Array access with index */
            /* The index increment happens in loop update */
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst[i];
    }
    
    printf("Checksum: %lld\n", (long long)checksum);
    
    return 0;
}
