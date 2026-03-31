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
    int32_t *psrc = src;
    int32_t *pdst = dst;
    
    /* 
     * Loop designed to create RTL patterns for auto-increment optimization.
     * The pointer post-increment operations (*pdst++ = *psrc++) should
     * generate memory loads/stores followed by register increments.
     */
    for (int i = 0; i < SIZE; i++) {
        /* Core pattern: memory access followed by pointer increment */
        *pdst++ = *psrc++;
        
        /*
         * Add another independent memory operation to increase chances
         * of hitting the uncovered block with different mem_insn structures
         */
        if (i % 2 == 0) {
            /* Additional array access with different base */
            volatile int32_t temp = src[SIZE - 1 - i];
            (void)temp; /* Prevent unused variable warning */
        }
    }
    
    /* Reset pointers for verification */
    psrc = src;
    pdst = dst;
    
    /* Verify copy and compute checksum */
    int32_t checksum = 0;
    int errors = 0;
    
    for (int i = 0; i < SIZE; i++) {
        if (*pdst != *psrc) {
            errors++;
        }
        checksum += *pdst;
        pdst++;
        psrc++;
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Errors: %d\n", errors);
    
    /* Additional test with decrement pattern */
    int32_t reverse[SIZE];
    int32_t *preverse = reverse + SIZE - 1; /* Start from end */
    psrc = src + SIZE - 1; /* Start from end of source */
    
    /* Post-decrement pattern */
    for (int i = 0; i < SIZE; i++) {
        *preverse-- = *psrc--;
    }
    
    /* Verify reverse copy */
    int32_t reverse_checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        reverse_checksum += reverse[i];
    }
    printf("Reverse checksum: %d\n", reverse_checksum);
    
    return errors == 0 ? 0 : 1;
}
