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
    volatile int32_t *volatile_src = src;  /* volatile prevents removal of loads */
    volatile int32_t *volatile_dst = dst;  /* volatile prevents removal of stores */
    
    /* Non-volatile pointers for the actual pointer arithmetic */
    int32_t *psrc = src;
    int32_t *pdst = dst;
    
    /* Loop with clear post-increment pattern */
    for (int i = 0; i < SIZE; i++) {
        /* Multiple memory operations with independent pointers */
        int32_t val1 = *volatile_src;      /* Read from source - volatile load */
        *volatile_dst = val1;              /* Write to dest - volatile store */
        
        /* Non-volatile pointer increments - these should create the RTL patterns */
        /* The auto-inc-dec pass should try to combine these with the memory ops */
        psrc++;
        pdst++;
        
        /* Also increment volatile pointers to maintain loop */
        volatile_src++;
        volatile_dst++;
    }
    
    /* Alternative loop with direct post-increment in array access */
    /* This creates another pattern for the pass to analyze */
    int32_t src2[SIZE];
    int32_t dst2[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        src2[i] = i * 2;
    }
    
    int32_t *p1 = src2;
    int32_t *p2 = dst2;
    int idx = 0;
    
    while (idx < SIZE) {
        /* Direct post-increment pattern */
        *p2 = *p1;
        p1++;  /* Post-increment of base register */
        p2++;  /* Another post-increment */
        idx++;
    }
    
    /* Third pattern: array indexing with separate increment */
    int32_t src3[SIZE];
    int32_t dst3[SIZE];
    int j = 0;
    
    for (int i = 0; i < SIZE; i++) {
        src3[i] = i * 5;
    }
    
    while (j < SIZE) {
        dst3[j] = src3[j];  /* Memory access with base + offset */
        j++;                /* Increment after access - post-increment pattern */
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst[i] + dst2[i] + dst3[i];
    }
    
    printf("Checksum: %lld\n", (long long)checksum);
    
    return 0;
}
