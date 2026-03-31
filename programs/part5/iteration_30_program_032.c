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
    volatile int32_t *volatile src_ptr = src;  /* volatile pointer to volatile data */
    volatile int32_t *volatile dst_ptr = dst;  /* prevents some optimizations */
    
    int32_t *psrc = (int32_t *)src_ptr;  /* Cast away volatile for pointer arithmetic */
    int32_t *pdst = (int32_t *)dst_ptr;
    
    /* Loop with clear post-increment pattern */
    for (int i = 0; i < SIZE; i++) {
        /* Multiple memory operations with independent pointers */
        *pdst = *psrc;      /* Memory load followed by store */
        
        /* Post-increment operations - key pattern for auto-inc-dec */
        pdst = pdst + 1;    /* This should create REG + CONST pattern */
        psrc = psrc + 1;    /* Another independent increment */
        
        /* Alternative form that might trigger different patterns */
        if (i % 2 == 0) {
            /* Additional memory access with different offset */
            dst[i] = src[i] + 1;
        }
    }
    
    /* Second loop with decrement pattern */
    psrc = (int32_t *)src_ptr + SIZE - 1;
    pdst = (int32_t *)dst_ptr + SIZE - 1;
    
    for (int i = SIZE - 1; i >= 0; i--) {
        *pdst = *psrc + 2;  /* Different operation to avoid CSE */
        pdst = pdst - 1;    /* Post-decrement pattern */
        psrc = psrc - 1;    /* Another independent decrement */
    }
    
    /* Third loop with mixed operations */
    int32_t *p1 = (int32_t *)src_ptr;
    int32_t *p2 = (int32_t *)dst_ptr;
    int32_t *p3 = (int32_t *)dst_ptr + SIZE/2;
    
    for (int i = 0; i < SIZE/2; i++) {
        /* Multiple independent memory streams */
        int32_t val1 = *p1;         /* Load */
        *p2 = val1 * 2;             /* Store with computation */
        *p3 = val1 + *p2;           /* Another store with two loads */
        
        /* All pointers increment independently */
        p1 = p1 + 1;
        p2 = p2 + 1;
        p3 = p3 + 1;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst[i];
    }
    
    /* Use result to prevent optimization */
    printf("Checksum: %lld\n", (long long)checksum);
    
    return (int)(checksum % 256);  /* Return non-deterministic value */
}
