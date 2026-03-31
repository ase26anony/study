/* auto_inc_test.c
 * Designed to trigger auto-increment/decrement optimization in GCC RTL
 * Targets architectures with auto-modify addressing modes (ARM, MIPS, PowerPC)
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
     * Loop designed to create RTL patterns for auto-inc-dec pass:
     * - Memory load from psrc (mem_insn)
     * - Memory store to pdst (mem_insn)  
     * - Pointer increments (candidate for find_inc)
     * The simple post-increment pattern should trigger the optimization
     */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* This creates the exact pattern: memory access followed by increment */
        *pdst++ = *psrc++;
        
        /* Additional independent memory stream to increase chances */
        /* Uncomment for more coverage:
        int temp = src[ARRAY_SIZE - 1 - i];
        dst[ARRAY_SIZE - 1 - i] = temp;
        */
    }
    
    /* 
     * Alternative loop style that also creates good patterns:
     * Using array indexing with post-increment in the same statement
     */
    psrc = src;
    pdst = dst;
    int idx = 0;
    while (idx < ARRAY_SIZE) {
        pdst[idx] = psrc[idx];
        idx++;  /* This increment should be adjacent to memory access in RTL */
    }
    
    /* 
     * Third pattern: Decrementing loop for auto-decrement
     * Some architectures favor decrement patterns
     */
    int *psrc_end = &src[ARRAY_SIZE - 1];
    int *pdst_end = &dst[ARRAY_SIZE - 1];
    for (int i = ARRAY_SIZE - 1; i >= 0; i--) {
        *pdst_end-- = *psrc_end--;
    }
    
    /* Compute checksum to prevent dead code elimination */
    uint32_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += (uint32_t)dst[i];
    }
    
    /* Use result to prevent optimization */
    printf("Checksum: %u\n", checksum);
    
    return (checksum > 0) ? 0 : 1;
}
