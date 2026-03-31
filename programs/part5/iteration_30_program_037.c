/* auto_inc_test.c
 * Designed to trigger auto-increment/decrement optimization in GCC RTL
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stdio.h>
#include <stdint.h>

#define ARRAY_SIZE 1000

/* Use volatile for array data to prevent dead code elimination
 * while keeping index operations non-volatile for pattern matching */
static volatile int src_data[ARRAY_SIZE];
static volatile int dst_data[ARRAY_SIZE];

int main(void) {
    /* Initialize source array with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src_data[i] = i * 3 + 1;
    }
    
    /* Clear destination array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        dst_data[i] = 0;
    }
    
    /* KEY PATTERN: Multiple pointer-based array traversals
     * This creates ideal RTL patterns for auto-inc-dec optimization */
    volatile int *psrc = src_data;
    volatile int *pdst = dst_data;
    
    /* Loop with post-increment pattern - prime candidate for optimization */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Multiple memory operations in sequence:
         * 1. Load from psrc (creates mem_insn)
         * 2. Store to pdst (creates another mem_insn)
         * 3. Post-increment both pointers
         * This should generate RTL where memory references are followed
         * by arithmetic operations on the base registers */
        *pdst = *psrc;
        
        /* Post-increment operations - these should be adjacent to
         * the memory operations in RTL, allowing find_address_inc to match */
        pdst++;
        psrc++;
        
        /* Additional independent access stream to increase chances */
        if (i % 2 == 0) {
            /* Another memory operation pattern */
            volatile int temp = src_data[ARRAY_SIZE - 1 - i];
            dst_data[ARRAY_SIZE - 1 - i] = temp;
        }
    }
    
    /* Second loop with different pattern: pre-decrement */
    psrc = src_data + ARRAY_SIZE - 1;
    pdst = dst_data + ARRAY_SIZE - 1;
    
    for (int i = 0; i < ARRAY_SIZE / 2; i++) {
        /* Pre-decrement pattern */
        --psrc;
        --pdst;
        *pdst = *psrc;
    }
    
    /* Third pattern: mixed operations in same expression */
    int *simple_src = (int*)src_data;
    int *simple_dst = (int*)dst_data;
    
    for (int i = 0; i < ARRAY_SIZE; i += 2) {
        /* Combined access and increment - may generate different RTL pattern */
        simple_dst[i] = simple_src[i];
        simple_dst[i + 1] = simple_src[i + 1];
    }
    
    /* Compute checksum to prevent dead code elimination */
    uint32_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst_data[i];
    }
    
    printf("Checksum: %u\n", (unsigned int)checksum);
    
    return 0;
}
