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
    volatile int32_t *volatile src_ptr = src;  /* volatile pointer to volatile data */
    volatile int32_t *volatile dst_ptr = dst;  /* prevents some optimizations */
    
    /* Loop with clear post-increment pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Multiple memory operations with independent pointers */
        int32_t val = *src_ptr;    /* Load with pointer dereference */
        *dst_ptr = val;            /* Store with pointer dereference */
        
        /* Explicit pointer increments - creates arithmetic ops adjacent to mem ops */
        src_ptr = src_ptr + 1;     /* This should create REG + CONST pattern */
        dst_ptr = dst_ptr + 1;     /* Another independent increment */
    }
    
    /* Alternative loop with different increment pattern */
    {
        int32_t *p1 = src;
        int32_t *p2 = dst;
        int32_t *p3 = src + ARRAY_SIZE/2;  /* Third pointer for more candidates */
        
        for (int i = 0; i < ARRAY_SIZE/2; i++) {
            /* Multiple array accesses with different base registers */
            int32_t a = p1[i];      /* Array index form */
            int32_t b = p3[i];      /* Another independent access */
            p2[i] = a + b;          /* Store operation */
            
            /* Force index variable increment that could be combined */
            /* The i++ in loop control creates increment pattern */
        }
    }
    
    /* Another pattern: pointer post-increment in loop body */
    {
        int32_t *read_ptr = src;
        int32_t *write_ptr = dst;
        int count = ARRAY_SIZE;
        
        while (count-- > 0) {
            /* Classic post-increment pattern */
            *write_ptr++ = *read_ptr++ + 5;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst[i];
    }
    
    printf("Checksum: %lld\n", (long long)checksum);
    
    return 0;
}
