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
     * Key pattern: Use pointer arithmetic with post-increment
     * This should generate RTL sequences where memory access
     * is followed by register increment
     */
    int32_t *psrc = src;
    int32_t *pdst = dst;
    
    /* 
     * Simple loop with post-increment on both pointers
     * This creates the pattern: load from *psrc, increment psrc
     *                         store to *pdst, increment pdst
     */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* 
         * Volatile qualifier prevents the entire operation from being
         * optimized away, but keeps the increment pattern visible
         */
        int32_t volatile temp = *psrc;
        *pdst = temp;
        
        /* Post-increment operations - prime candidates for auto-inc-dec */
        psrc++;
        pdst++;
    }
    
    /* 
     * Alternative pattern: Multiple independent access streams
     * This creates more candidate mem_insn structures
     */
    int32_t *psrc2 = src;
    int32_t *pdst2 = dst;
    int32_t *pdst3 = dst + ARRAY_SIZE/2;
    
    for (int i = 0; i < ARRAY_SIZE/2; i++) {
        /* Multiple memory operations with increments */
        int32_t val1 = *psrc2;
        psrc2++;
        
        *pdst2 = val1;
        pdst2++;
        
        *pdst3 = val1 * 2;
        pdst3++;
    }
    
    /* 
     * Pattern with negative increments (auto-decrement)
     * This tests the decrement matching logic
     */
    int32_t *psrc_rev = src + ARRAY_SIZE - 1;
    int32_t *pdst_rev = dst + ARRAY_SIZE - 1;
    
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        int32_t val = *psrc_rev;
        *pdst_rev = val;
        
        /* Post-decrement operations */
        psrc_rev--;
        pdst_rev--;
    }
    
    /* 
     * Array indexing with explicit increment
     * This creates a different RTL pattern that might still match
     */
    int idx = 0;
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        dst[idx] = src[idx] + 1;
        idx++;  /* Increment after use - post-increment pattern */
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst[i];
    }
    
    /* Use the result to prevent optimization */
    printf("Checksum: %lld\n", (long long)checksum);
    
    return 0;
}
