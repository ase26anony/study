/* auto_inc_dec_test.c
 * Designed to trigger auto-increment/decrement optimization in GCC RTL
 * Targets the specific uncovered lines in auto-inc-dec.cc:1352-1358
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
     * CRITICAL SECTION: Create patterns for auto-inc-dec optimization
     * Using pointer arithmetic with post-increment to generate
     * memory access followed by register increment in RTL
     */
    int32_t *psrc = src;
    int32_t *pdst = dst;
    
    /* 
     * Loop 1: Simple copy with post-increment - classic pattern
     * This should generate load/store with adjacent address increment
     */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *pdst++ = *psrc++;
    }
    
    /* Reset pointers for second pattern */
    psrc = src;
    pdst = dst;
    
    /* 
     * Loop 2: Multiple independent memory streams
     * Creates multiple mem_insn candidates for the pass to analyze
     */
    int32_t *psrc2 = src + ARRAY_SIZE/2;
    int32_t *pdst2 = dst + ARRAY_SIZE/2;
    
    for (int i = 0; i < ARRAY_SIZE/2; i++) {
        /* Two independent post-increment operations */
        *pdst++ = *psrc++;
        *pdst2++ = *psrc2++;
    }
    
    /* 
     * Loop 3: Mixed pre/post operations to test different patterns
     * The optimizer should still find the post-increment opportunities
     */
    int32_t *psrc3 = src;
    int32_t *pdst3 = dst;
    
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        /* Access then increment - post-increment pattern */
        int32_t val = *psrc3;
        psrc3++;
        *pdst3 = val;
        pdst3++;
    }
    
    /* 
     * Loop 4: Decrement pattern (auto-decrement optimization)
     * Starting from end and moving backward
     */
    int32_t *psrc4 = src + ARRAY_SIZE - 1;
    int32_t *pdst4 = dst + ARRAY_SIZE - 1;
    
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        *pdst4-- = *psrc4--;
    }
    
    /* 
     * Loop 5: Array indexing with explicit increment
     * Should still generate the pattern after optimization
     */
    int idx = 0;
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        dst[idx] = src[idx] * 2;
        idx++;  /* Increment after use - post-increment pattern */
    }
    
    /* 
     * Loop 6: Volatile array to prevent over-optimization
     * but keep increment pattern clear
     */
    volatile int32_t vsrc[100];
    volatile int32_t vdst[100];
    int32_t *pvsrc = (int32_t*)vsrc;
    int32_t *pvdst = (int32_t*)vdst;
    
    for (int i = 0; i < 100; i++) {
        *pvdst++ = *pvsrc++;
    }
    
    /* Compute checksum to ensure code isn't dead */
    int64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst[i];
    }
    
    /* Use result to prevent dead code elimination */
    printf("Checksum: %lld\n", (long long)checksum);
    
    /* Additional volatile store to force memory operations */
    volatile int32_t sink = dst[0];
    (void)sink;  /* Suppress unused variable warning */
    
    return 0;
}
