/* auto_inc_test.c
 * Designed to trigger GCC's auto-increment/decrement optimization
 * Target: architectures with auto-modify addressing modes (ARM, MIPS, PowerPC)
 */

#include <stdio.h>
#include <stdint.h>

#define ARRAY_SIZE 1000

int main(void) {
    /* Declare source and destination arrays */
    int src[ARRAY_SIZE];
    int dst[ARRAY_SIZE];
    
    /* Initialize source array with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Use pointer arithmetic to create post-increment patterns */
    int *psrc = src;
    int *pdst = dst;
    
    /* 
     * Loop designed to create RTL patterns for auto-inc-dec optimization:
     * - Memory load from *psrc
     * - Memory store to *pdst  
     * - Post-increment of both pointers
     * This should generate adjacent memory and arithmetic operations
     */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* These accesses create mem_insn patterns */
        *pdst = *psrc;
        
        /* Post-increment operations - prime candidates for auto-inc-dec */
        psrc++;
        pdst++;
        
        /* Additional independent memory stream to increase match chances */
        volatile int dummy = src[ARRAY_SIZE - 1 - i];  /* Prevent optimization */
        (void)dummy;  /* Suppress unused warning */
    }
    
    /* Alternative loop with explicit index for different pattern */
    int idx1 = 0;
    int idx2 = 0;
    while (idx1 < ARRAY_SIZE / 2) {
        /* Array access with index that gets incremented */
        dst[idx2] = src[idx1];
        
        /* Increment indices separately */
        idx1++;
        idx2++;
    }
    
    /* Compute checksum to prevent dead code elimination */
    uint32_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += (uint32_t)dst[i];
    }
    
    printf("Checksum: %u\n", checksum);
    
    return (checksum > 0) ? 0 : 1;
}
