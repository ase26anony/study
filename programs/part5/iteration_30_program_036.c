#include <stdio.h>
#include <stdint.h>

#define SIZE 1000

int main(void) {
    /* Declare source and destination arrays */
    int32_t src[SIZE];
    int32_t dst[SIZE];
    
    /* Initialize source array with pattern */
    for (int i = 0; i < SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Use pointer arithmetic to create post-increment patterns */
    int32_t *psrc = src;
    int32_t *pdst = dst;
    
    /* 
     * Loop designed to create RTL patterns for auto-increment optimization.
     * The pointer post-increment operations should generate memory accesses
     * followed by register increments that the auto-inc-dec pass can combine.
     */
    for (int i = 0; i < SIZE; i++) {
        /* Multiple memory operations with clear increment patterns */
        *pdst++ = *psrc++;  /* Primary pattern: load then store with post-increment */
        
        /* Additional independent memory stream to increase candidate matches */
        if (i % 2 == 0) {
            /* Create another memory access pattern */
            volatile int32_t temp = src[SIZE - 1 - i];  /* Prevent optimization */
            (void)temp;  /* Use variable to avoid unused warning */
        }
    }
    
    /* Reset pointers for verification */
    psrc = src;
    pdst = dst;
    
    /* Verify copy and compute checksum */
    uint64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        if (*pdst != *psrc) {
            printf("Mismatch at index %d\n", i);
            return 1;
        }
        checksum += *pdst++;
        checksum += *psrc++;
    }
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    /* Additional test with decrement pattern */
    int32_t *rsrc = &src[SIZE - 1];
    int32_t *rdst = &dst[SIZE - 1];
    
    for (int i = 0; i < SIZE / 2; i++) {
        *rdst-- = *rsrc--;  /* Post-decrement pattern */
    }
    
    /* Final checksum to ensure all code is executed */
    uint64_t final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += dst[i];
    }
    
    printf("Final sum: %llu\n", (unsigned long long)final_sum);
    
    return 0;
}
