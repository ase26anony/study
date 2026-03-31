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
    
    /* Use pointer arithmetic with post-increment patterns */
    volatile int32_t *volatile src_data = src;  /* volatile pointer to volatile data */
    volatile int32_t *volatile dst_data = dst;  /* prevents some optimizations */
    
    int32_t *psrc = (int32_t *)src_data;
    int32_t *pdst = (int32_t *)dst_data;
    
    /* Loop with clear post-increment pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Multiple memory operations with independent pointers */
        *pdst = *psrc;          /* Load from src, store to dst */
        
        /* Post-increment operations - key pattern for auto-inc-dec */
        pdst = pdst + 1;        /* This should create RTL increment after memory op */
        psrc = psrc + 1;        /* Another independent increment */
        
        /* Additional array access to create more candidates */
        if (i % 2 == 0) {
            /* Alternate path with different offset */
            src[i] = src[i] + 1;
        }
    }
    
    /* Second loop with decrement pattern */
    psrc = (int32_t *)src_data + ARRAY_SIZE - 1;
    pdst = (int32_t *)dst_data + ARRAY_SIZE - 1;
    
    for (int i = ARRAY_SIZE - 1; i >= 0; i--) {
        *pdst = *psrc;          /* Memory access */
        pdst = pdst - 1;        /* Post-decrement */
        psrc = psrc - 1;        /* Another post-decrement */
    }
    
    /* Compute checksum to prevent dead code elimination */
    int32_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional test with byte arrays for different addressing modes */
    char src_bytes[ARRAY_SIZE];
    char dst_bytes[ARRAY_SIZE];
    
    char *psrc_b = src_bytes;
    char *pdst_b = dst_bytes;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *pdst_b++ = *psrc_b++;  /* Direct post-increment in expression */
    }
    
    return 0;
}
