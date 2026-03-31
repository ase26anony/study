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
    
    /* Loop with clear post-increment pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Memory load from src with implicit post-increment */
        int32_t val = *psrc;
        
        /* Simple operation on the value */
        val = val + 5;
        
        /* Memory store to dst with implicit post-increment */
        *pdst = val;
        
        /* Explicit pointer increments - these should be adjacent to memory ops in RTL */
        psrc++;
        pdst++;
    }
    
    /* Alternative loop with multiple independent access streams */
    /* This creates more candidate mem_insn structures */
    int32_t *psrc2 = src;
    int32_t *pdst2 = dst;
    int32_t *ptmp = dst + ARRAY_SIZE/2;  /* Another pointer for more patterns */
    
    for (int i = 0; i < ARRAY_SIZE/2; i++) {
        /* Multiple memory operations in same iteration */
        int32_t a = *psrc2;      /* Load with potential auto-inc */
        psrc2++;                 /* Increment - should be adjacent in RTL */
        
        int32_t b = *ptmp;       /* Another load */
        ptmp--;                  /* Post-decrement pattern */
        
        *pdst2 = a + b;          /* Store with potential auto-inc */
        pdst2++;                 /* Increment - should be adjacent in RTL */
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst[i];
    }
    
    printf("Checksum: %lld\n", (long long)checksum);
    
    return 0;
}
