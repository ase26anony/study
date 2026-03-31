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
     * - Memory load from psrc (src array)
     * - Memory store to pdst (dst array)  
     * - Post-increment of both pointers
     * This should generate adjacent memory access and increment operations
     */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* These create the mem_insn patterns the pass looks for */
        *pdst = *psrc;
        
        /* Post-increment operations - should be adjacent to memory ops in RTL */
        pdst++;
        psrc++;
        
        /* Alternative form that also creates good patterns */
        // *pdst++ = *psrc++;
    }
    
    /* 
     * Second loop with different pattern to increase chances of hitting
     * the specific initialization block (reg1_val = 0)
     */
    int sum = 0;
    int *p = dst;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Access with constant offset 0 pattern */
        sum += p[0];
        p++;  /* Post-increment */
    }
    
    /* 
     * Third pattern: array copy with index variable
     * This creates different RTL that might hit the specific code path
     */
    int src2[ARRAY_SIZE];
    int dst2[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src2[i] = i * 2;
    }
    
    int idx = 0;
    while (idx < ARRAY_SIZE) {
        dst2[idx] = src2[idx];
        idx++;  /* Post-increment of index */
    }
    
    /* Compute checksum to prevent dead code elimination */
    int total = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += dst[i] + dst2[i];
    }
    
    printf("Checksum: %d\n", total);
    printf("Sum: %d\n", sum);
    
    return 0;
}
