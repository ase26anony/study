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
    volatile int32_t *volatile src_data = src;  /* volatile pointer to volatile data */
    volatile int32_t *volatile dst_data = dst;  /* prevents elimination of memory ops */
    
    /* Loop with clear post-increment pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Multiple memory operations with independent pointers */
        *dst_data = *src_data;      /* Load from src, store to dst */
        
        /* Post-increment operations - key pattern for auto-inc-dec */
        dst_data = dst_data + 1;    /* Should generate post-increment RTL */
        src_data = src_data + 1;    /* Multiple candidates for the pass to analyze */
        
        /* Additional independent access stream */
        if (i % 2 == 0) {
            /* Create another memory reference pattern */
            int32_t temp = src[i % 10];
            dst[i % 10] = temp;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    int32_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional test with decrement pattern */
    int32_t rev[ARRAY_SIZE];
    volatile int32_t *volatile rev_ptr = &rev[ARRAY_SIZE - 1];
    
    for (int i = ARRAY_SIZE - 1; i >= 0; i--) {
        *rev_ptr = dst[i];
        rev_ptr = rev_ptr - 1;  /* Post-decrement pattern */
    }
    
    /* Mixed increment/decrement in same loop */
    int32_t *p1 = src;
    int32_t *p2 = &dst[ARRAY_SIZE - 1];
    
    for (int i = 0; i < ARRAY_SIZE / 2; i++) {
        *p2 = *p1;      /* Load and store */
        p1 = p1 + 1;    /* Post-increment */
        p2 = p2 - 1;    /* Post-decrement */
    }
    
    return 0;
}
