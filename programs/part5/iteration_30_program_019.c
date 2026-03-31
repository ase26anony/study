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
    
    /* Loop with clear post-increment pattern
     * This should generate RTL with memory access followed by register increment */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Memory load from src, then pointer increment */
        int val = *psrc;
        psrc++;  /* Post-increment after load */
        
        /* Memory store to dst, then pointer increment */
        *pdst = val;
        pdst++;  /* Post-increment after store */
        
        /* Add another independent memory operation to increase chances */
        /* This creates more candidate mem_insn structures */
        static int counter[10] = {0};
        counter[i % 10] += val;
    }
    
    /* Additional loop with array indexing to create different pattern */
    /* This might trigger the reg1_val = 0 initialization */
    int sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Array access with constant offset 0 in addressing mode */
        sum += dst[i];
        
        /* Force simple addressing: base + 0 */
        /* This could match mem_insn.reg1_val = 0 pattern */
        int temp = dst[i + 0];
        sum += temp;
    }
    
    /* Another pattern: pointer with post-decrement */
    int *p = &dst[ARRAY_SIZE - 1];
    int rev_sum = 0;
    for (int i = 0; i < 100; i++) {
        rev_sum += *p;
        p--;  /* Post-decrement */
    }
    
    /* Compute final checksum to prevent dead code elimination */
    int final_result = sum + rev_sum;
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", final_result);
    
    return 0;
}
