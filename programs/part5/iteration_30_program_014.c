/* auto_inc_dec_test.c
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
     * Loop with clear post-increment pattern
     * This should generate RTL with memory access followed by register increment
     */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Memory load from src with post-increment */
        int val = *psrc++;
        
        /* Simple operation on loaded value */
        val = val * 2 - 1;
        
        /* Memory store to dst with post-increment */
        *pdst++ = val;
    }
    
    /* 
     * Second loop with different pattern to increase chances
     * Using array indexing with explicit increment
     */
    int sum1 = 0;
    int j = 0;
    while (j < ARRAY_SIZE) {
        /* Access with array index, increment after use */
        sum1 += dst[j];
        j++;  /* This increment should be adjacent to memory access in RTL */
    }
    
    /* 
     * Third pattern: pointer decrement for auto-decrement
     */
    int *p = &dst[ARRAY_SIZE - 1];
    int sum2 = 0;
    for (int k = ARRAY_SIZE - 1; k >= 0; k--) {
        /* Memory access with pre-decrement would be optimized differently,
           so we use post-decrement pattern */
        sum2 += *p;
        p--;  /* Decrement after use - candidate for auto-decrement */
    }
    
    /* 
     * Fourth pattern: multiple independent streams
     * Creates multiple mem_insn structures for analysis
     */
    int temp[ARRAY_SIZE];
    int *p1 = src;
    int *p2 = dst;
    int *p3 = temp;
    
    for (int m = 0; m < ARRAY_SIZE; m++) {
        /* Three independent memory operations with increments */
        int a = *p1++;
        int b = *p2++;
        *p3++ = a + b;
    }
    
    /* Compute final checksum to prevent dead code elimination */
    int final_sum = sum1 + sum2;
    for (int n = 0; n < ARRAY_SIZE; n++) {
        final_sum += temp[n];
    }
    
    printf("Result checksum: %d\n", final_sum);
    
    return 0;
}
