/* auto_inc_test.c
 * Designed to trigger auto-increment/decrement optimization in GCC RTL
 * Targets architectures with auto-modify addressing modes (ARM, MIPS, PowerPC)
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
     * CRITICAL PATTERN: Multiple memory operations with post-increment
     * This creates RTL sequences where memory accesses are followed by
     * register increments, which the auto-inc-dec pass tries to combine.
     */
    int32_t *psrc = src;
    int32_t *pdst = dst;
    
    /* 
     * Use volatile for array pointers to prevent complete optimization
     * but keep the increment pattern intact
     */
    volatile int32_t *vpsrc = src;
    volatile int32_t *vpdst = dst;
    
    /* 
     * Pattern 1: Simple pointer post-increment (most likely to trigger)
     * This directly creates the load/store + increment pattern
     */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *pdst++ = *psrc++;
    }
    
    /* Reset pointers for second pattern */
    psrc = src;
    pdst = dst;
    
    /* 
     * Pattern 2: Multiple independent access streams
     * Creates multiple mem_insn candidates for the pass to analyze
     */
    for (int i = 0; i < ARRAY_SIZE / 2; i++) {
        /* First stream: read from src, write to dst */
        int32_t val1 = *psrc;
        *pdst = val1;
        psrc++;
        pdst++;
        
        /* Second stream: read from src with offset, write to dst with offset */
        int32_t val2 = *(psrc + 1);
        *(pdst + 1) = val2;
    }
    
    /* 
     * Pattern 3: Mixed pre/post operations to test different cases
     * The auto-inc-dec pass looks for both pre and post variants
     */
    int32_t *p1 = &src[0];
    int32_t *p2 = &dst[ARRAY_SIZE - 1];
    
    for (int i = 0; i < ARRAY_SIZE / 4; i++) {
        /* Post-increment on source, pre-decrement on destination */
        *p2 = *p1;
        p1++;
        p2--;
        
        /* Another access to create more candidates */
        *(p2 - 1) = *(p1 + 1);
    }
    
    /* 
     * Pattern 4: Array indexing with explicit increment
     * This should generate base + offset addressing that might
     * match the reg1_is_const = true, reg1_val = 0 case
     */
    int idx = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        dst[idx] = src[idx];
        idx = idx + 1;  /* Simple increment - might create separate insn */
    }
    
    /* 
     * Pattern 5: Using volatile to ensure memory ops aren't reordered
     * but keeping the increment pattern visible to RTL
     */
    int32_t *volatile vp1 = src + 100;
    int32_t *volatile vp2 = dst + 100;
    
    for (int i = 0; i < 100; i++) {
        *vp2 = *vp1;
        vp1 = vp1 + 1;  /* Separate increment operation */
        vp2 = vp2 + 1;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst[i];
    }
    
    /* Use the result */
    printf("Checksum: %lld\n", (long long)checksum);
    
    return (checksum > 0) ? 0 : 1;
}
