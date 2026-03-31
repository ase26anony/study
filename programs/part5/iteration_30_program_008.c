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
    volatile int32_t *volatile src_data = src;  /* volatile pointer to volatile data */
    volatile int32_t *volatile dst_data = dst;  /* prevents some optimizations */
    
    /* Loop with clear post-increment pattern */
    for (int i = 0; i < SIZE; i++) {
        /* These accesses should generate MEM references followed by REG increments */
        *dst_data = *src_data;
        
        /* Post-increment operations - key pattern for auto-inc-dec */
        dst_data = dst_data + 1;  /* Should become auto-increment in RTL */
        src_data = src_data + 1;  /* Another candidate for auto-increment */
        
        /* Alternative form that also creates good patterns */
        int32_t temp = src[i];    /* Array access with index */
        dst[i] = temp;            /* Another array access */
    }
    
    /* Second loop with different pattern to increase coverage */
    int32_t *p1 = &src[0];
    int32_t *p2 = &dst[0];
    int32_t *end = &src[SIZE];
    
    while (p1 < end) {
        /* Direct pointer dereference with post-increment */
        *p2++ = *p1++;  /* Classic post-increment pattern */
    }
    
    /* Third variation: mixed patterns */
    int32_t *src_ptr = src;
    int32_t *dst_ptr = dst;
    
    for (int count = 0; count < SIZE; count++) {
        /* Access through pointer with offset */
        dst_ptr[0] = src_ptr[0];
        
        /* Explicit increment after use */
        src_ptr = &src_ptr[1];  /* Creates address calculation */
        dst_ptr = &dst_ptr[1];  /* Another address calculation */
    }
    
    /* Compute checksum to prevent dead code elimination */
    int32_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional test with negative increments */
    int32_t reverse[SIZE];
    int32_t *src_end = &src[SIZE - 1];
    int32_t *rev_ptr = &reverse[0];
    
    for (int i = 0; i < SIZE; i++) {
        *rev_ptr++ = *src_end--;  /* Post-increment and post-decrement */
    }
    
    /* Compute second checksum */
    int32_t checksum2 = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum2 += reverse[i];
    }
    
    printf("Reverse checksum: %d\n", checksum2);
    
    return 0;
}
