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
     * The pointer post-increment operations should generate:
     * 1. Memory load from psrc (mem_insn)
     * 2. Arithmetic increment of psrc
     * 3. Memory store to pdst (another mem_insn)
     * 4. Arithmetic increment of pdst
     */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* This creates the memory reference pattern that find_address_inc looks for */
        *pdst++ = *psrc++;
        
        /* Add another independent access pattern to increase chances */
        if (i % 2 == 0) {
            /* Additional memory operations with different base pointers */
            volatile int32_t temp = src[ARRAY_SIZE - 1 - i];
            (void)temp; /* Use temp to prevent optimization */
        }
    }
    
    /* Alternative loop with explicit index for different pattern */
    int32_t *psrc2 = src;
    int32_t *pdst2 = dst + ARRAY_SIZE/2;
    
    /* Reverse copy with post-decrement pattern */
    for (int i = 0; i < ARRAY_SIZE/2; i++) {
        *(--pdst2) = *(psrc2++);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst[i];
    }
    
    printf("Checksum: %lld\n", (long long)checksum);
    
    /* Additional pattern: array processing with mixed operations */
    int32_t *p = dst;
    int32_t sum = 0;
    
    /* Process array with post-increment in condition */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *p++;
        if (i % 4 == 0) {
            *(p - 1) = sum;
        }
    }
    
    printf("Final sum: %d\n", sum);
    
    return 0;
}
