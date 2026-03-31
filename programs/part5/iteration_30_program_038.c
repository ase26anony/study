/* auto_inc_dec_test.c
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
    
    /* Use pointer arithmetic to create post-increment patterns */
    volatile int32_t *volatile src_ptr = src;  /* volatile pointer to volatile data */
    volatile int32_t *volatile dst_ptr = dst;  /* prevents some optimizations */
    
    /* Loop with clear post-increment pattern */
    /* This should generate RTL with memory access followed by register increment */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Multiple memory operations with independent pointers */
        /* Each creates a candidate mem_insn structure */
        *dst_ptr++ = *src_ptr++;  /* Post-increment pattern 1 */
        
        /* Additional independent access stream */
        /* Creates more opportunities for find_address_inc to be called */
        if (i % 2 == 0) {
            /* Alternate pattern that might use different addressing */
            dst[ARRAY_SIZE - 1 - i] = src[ARRAY_SIZE - 1 - i] + 1;
        }
    }
    
    /* Second loop with decrement pattern */
    /* Tests auto-decrement optimization */
    int32_t *p1 = &src[ARRAY_SIZE - 1];
    int32_t *p2 = &dst[ARRAY_SIZE - 1];
    
    for (int i = 0; i < ARRAY_SIZE / 2; i++) {
        *p2-- = *p1--;  /* Post-decrement pattern */
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst[i];
    }
    
    /* Use result to prevent optimization */
    printf("Checksum: %lld\n", (long long)checksum);
    
    return (checksum != 0) ? 0 : 1;
}
