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
     * Loop designed to generate RTL with:
     * 1. Memory load from address in register
     * 2. Memory store to address in another register  
     * 3. Increment of both address registers
     * This should create candidate patterns for auto-inc-dec pass
     */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Memory access followed by pointer increment - classic post-increment */
        *pdst = *psrc;
        
        /* Increment pointers separately to keep operations adjacent */
        pdst = pdst + 1;
        psrc = psrc + 1;
        
        /* Alternative form that also creates good patterns */
        if (i % 2 == 0) {
            /* Additional memory operation with different offset */
            volatile int32_t temp = src[ARRAY_SIZE - 1 - i];
            (void)temp; /* Use temp to avoid unused variable warning */
        }
    }
    
    /* Second loop with decrement pattern */
    int32_t *psrc_end = &src[ARRAY_SIZE - 1];
    int32_t *pdst_end = &dst[ARRAY_SIZE - 1];
    
    for (int i = 0; i < ARRAY_SIZE / 2; i++) {
        /* Post-decrement pattern */
        *pdst_end = *psrc_end;
        pdst_end = pdst_end - 1;
        psrc_end = psrc_end - 1;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst[i];
    }
    
    printf("Checksum: %lld\n", (long long)checksum);
    
    return 0;
}
