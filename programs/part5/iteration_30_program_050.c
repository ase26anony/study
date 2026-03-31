/* auto_inc_test.c - Test program for GCC auto-increment/decrement optimization */
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
    
    /* Loop with clear post-increment pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Memory load with implicit base register (psrc) */
        int val = *psrc;
        
        /* Simple arithmetic operation on the value */
        val = val + 5;
        
        /* Memory store with implicit base register (pdst) */
        *pdst = val;
        
        /* Post-increment operations - these should create RTL patterns
           that the auto-inc-dec pass can match */
        psrc++;
        pdst++;
    }
    
    /* Alternative loop style with array indexing that also creates
       increment patterns in the address calculation */
    int sum1 = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* This creates: MEM[base + offset], then i = i + 1 */
        sum1 += dst[i];
    }
    
    /* Another pattern: reverse traversal with decrement */
    int sum2 = 0;
    int *p = &dst[ARRAY_SIZE - 1];
    for (int i = ARRAY_SIZE - 1; i >= 0; i--) {
        /* Memory access followed by pointer decrement */
        sum2 += *p;
        p--;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Checksum 1: %d\n", sum1);
    printf("Checksum 2: %d\n", sum2);
    printf("Total: %d\n", sum1 + sum2);
    
    return 0;
}
