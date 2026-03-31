#include <stdio.h>
#include <stdint.h>

#define ARRAY_SIZE 1000

int main(void) {
    /* Declare and initialize source and destination arrays */
    int32_t src[ARRAY_SIZE];
    int32_t dst[ARRAY_SIZE];
    
    /* Initialize source array with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Use pointer arithmetic with post-increment patterns */
    /* Multiple independent memory access streams increase chances */
    int32_t *psrc = src;
    int32_t *pdst = dst;
    
    /* Simple loop with clear post-increment pattern */
    /* The volatile qualifier prevents premature optimization of memory ops */
    volatile int32_t * volatile_src = src;
    volatile int32_t * volatile_dst = dst;
    
    /* First pattern: Classic post-increment pointer traversal */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *pdst++ = *psrc++;
    }
    
    /* Reset pointers for second pattern */
    psrc = src;
    pdst = dst;
    
    /* Second pattern: Mixed pre/post operations */
    /* This creates different RTL patterns for the pass to analyze */
    for (int i = 0; i < ARRAY_SIZE / 2; i++) {
        /* Post-increment on read, pre-increment on write */
        int32_t val = *psrc++;
        *++pdst = val;
    }
    
    /* Third pattern: Array indexing with explicit increment */
    /* This should generate base+offset addressing */
    int idx = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        dst[idx] = src[idx];
        idx = idx + 1;  /* Simple increment, not compound */
    }
    
    /* Fourth pattern: Decrementing loop */
    /* Tests auto-decrement optimization */
    int32_t *psrc_end = src + ARRAY_SIZE - 1;
    int32_t *pdst_end = dst + ARRAY_SIZE - 1;
    
    for (int i = ARRAY_SIZE - 1; i >= 0; i--) {
        *pdst_end-- = *psrc_end--;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst[i];
    }
    
    /* Use the result */
    printf("Checksum: %lld\n", (long long)checksum);
    
    return (int)(checksum % 256);
}
