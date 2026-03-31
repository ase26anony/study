/* auto_inc_test.c
 * Designed to trigger GCC's auto-increment/decrement optimization
 * Target: Uncovered lines 1352-1358 in auto-inc-dec.cc
 */

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
    
    /* Clear destination array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        dst[i] = 0;
    }
    
    /* 
     * CRITICAL SECTION: Create post-increment patterns
     * Using pointer arithmetic to generate clear memory access + increment sequences
     */
    volatile int32_t *volatile_src = src;  /* volatile pointer to prevent some optimizations */
    volatile int32_t *volatile_dst = dst;  /* but keep the increment pattern visible */
    
    int32_t *psrc = (int32_t *)volatile_src;
    int32_t *pdst = (int32_t *)volatile_dst;
    
    /* 
     * Loop designed to generate RTL with:
     * 1. Memory load from address in register
     * 2. Memory store to address in another register  
     * 3. Increment of both address registers
     * This should create candidates for auto-inc-dec optimization
     */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Pattern 1: Direct array access with post-increment in same statement */
        *pdst = *psrc;
        psrc++;
        pdst++;
        
        /* Pattern 2: Alternative form - separate increment */
        /* This creates another candidate for the optimization pass */
        if (i < ARRAY_SIZE - 1) {
            dst[i + 1] = src[i + 1];  /* Additional memory operation pattern */
        }
    }
    
    /* 
     * Additional test: Reverse traversal for auto-decrement
     * This creates different addressing patterns
     */
    int32_t *rsrc = &src[ARRAY_SIZE - 1];
    int32_t *rdst = &dst[ARRAY_SIZE - 1];
    
    for (int i = ARRAY_SIZE - 1; i >= 0; i--) {
        *rdst = *rsrc;
        rsrc--;  /* Post-decrement pattern */
        rdst--;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst[i];
    }
    
    /* Use result to ensure code isn't optimized away */
    printf("Checksum: %lld\n", (long long)checksum);
    
    /* Additional volatile store to force memory operations */
    volatile int32_t sink = dst[0];
    
    return (checksum != 0) ? 0 : 1;
}
