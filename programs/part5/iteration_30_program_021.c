#include <stdio.h>
#include <stdint.h>

#define SIZE 1000

int main(void) {
    /* Declare and initialize source and destination arrays */
    int32_t src[SIZE];
    int32_t dst[SIZE];
    
    /* Initialize source array with pattern */
    for (int i = 0; i < SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Use pointer arithmetic to create post-increment patterns */
    int32_t *psrc = src;
    int32_t *pdst = dst;
    
    /* Loop with clear post-increment pattern
     * This should generate RTL with memory access followed by register increment */
    for (int i = 0; i < SIZE; i++) {
        /* Memory load from src, then pointer increment */
        int32_t val = *psrc;
        psrc = psrc + 1;  /* This creates separate increment operation */
        
        /* Memory store to dst, then pointer increment */
        *pdst = val;
        pdst = pdst + 1;  /* Another separate increment operation */
    }
    
    /* Alternative loop with array indexing that also creates increment patterns */
    /* This creates multiple memory operation candidates for the pass to analyze */
    int32_t src2[SIZE];
    int32_t dst2[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        src2[i] = i * 2;
    }
    
    int idx = 0;
    while (idx < SIZE) {
        /* Access array element, then increment index separately */
        int32_t temp = src2[idx];
        idx = idx + 1;  /* Separate increment operation */
        
        /* Store to destination with separate index */
        int dest_idx = idx - 1;
        dst2[dest_idx] = temp;
    }
    
    /* Third pattern: mixed operations to create more candidates */
    int32_t src3[SIZE];
    int32_t dst3[SIZE];
    int32_t *p1 = src3;
    int32_t *p2 = dst3;
    
    for (int i = 0; i < SIZE; i++) {
        /* Direct assignment with post-increment in expression */
        *p2 = *p1;
        
        /* Separate increment statements to ensure they appear as distinct RTL insns */
        p1 = p1 + 1;
        p2 = p2 + 1;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst[i];
        checksum += dst2[i];
        checksum += dst3[i];
    }
    
    printf("Checksum: %lld\n", (long long)checksum);
    
    return 0;
}
