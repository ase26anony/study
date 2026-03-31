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
    
    /* Use pointer arithmetic to create post-increment patterns */
    int32_t *psrc = src;
    int32_t *pdst = dst;
    
    /* 
     * Loop designed to generate RTL with memory access followed by increment
     * This should create patterns for auto-inc-dec pass to match
     */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Memory load from src with implied post-increment */
        int32_t val = *psrc;
        psrc++;  /* Post-increment operation */
        
        /* Memory store to dst with implied post-increment */
        *pdst = val;
        pdst++;  /* Post-increment operation */
    }
    
    /* Alternative loop with multiple independent access streams */
    /* This increases chances of hitting the uncovered initialization block */
    int32_t *psrc2 = src;
    int32_t *pdst2 = dst;
    int32_t *pdst3 = dst + ARRAY_SIZE/2;  /* Another destination pointer */
    
    for (int i = 0; i < ARRAY_SIZE/2; i++) {
        /* Multiple memory operations in same iteration */
        int32_t val1 = *psrc2;
        psrc2++;
        
        *pdst2 = val1;
        pdst2++;
        
        *pdst3 = val1 * 2;
        pdst3++;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst[i];
    }
    
    printf("Checksum: %lld\n", (long long)checksum);
    
    /* Additional test with decrement patterns */
    int32_t *psrc_rev = src + ARRAY_SIZE - 1;
    int32_t *pdst_rev = dst + ARRAY_SIZE - 1;
    
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        *pdst_rev = *psrc_rev;
        psrc_rev--;  /* Post-decrement operation */
        pdst_rev--;  /* Post-decrement operation */
    }
    
    /* Final checksum */
    checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %lld\n", (long long)checksum);
    
    return 0;
}
