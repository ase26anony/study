/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific uncovered lines in GCC's
 * resource.cc file (lines 282-290) by generating RTL patterns containing
 * ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM expressions.
 * 
 * Compilation recommendations:
 *   For RTL expansion: gcc -O1 -fdump-rtl-all -fdump-rtl-expand test_resource_coverage.c
 *   For scheduling passes: gcc -O2 -fschedule-insns -freschedule-modulo-scheduled-loops test_resource_coverage.c
 *   For BPF target: gcc -target bpf -O2 -fdump-rtl-all test_resource_coverage.c
 */

#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT/STRICT_LOW_PART */
struct __attribute__((packed)) bitfield_struct {
    volatile uint32_t full_word;
    struct {
        volatile uint16_t low : 4;
        volatile uint16_t mid : 8;
        volatile uint16_t high : 4;
    } bits;
    volatile uint8_t byte_array[8];
};

/* Global variables to force memory operations */
static volatile int global_index = 0;
static volatile int global_mask = 0x0F;

int main(void) {
    struct bitfield_struct bf = {0};
    volatile uint32_t accumulator = 0;
    volatile int *ptr_array[4];
    volatile int temp_vars[4] = {1, 2, 3, 4};
    
    /* Initialize pointer array with complex addressing patterns */
    for (int i = 0; i < 4; i++) {
        ptr_array[i] = &temp_vars[i];
    }
    
    /* Main loop creating data dependencies and complex RTL patterns */
    for (volatile int i = 0; i < 100; i++) {
        /* 1. Arithmetic operation that may generate SUBREG in RTL */
        bf.full_word = bf.full_word + i * 3;
        
        /* 2. Bit-field assignments - may generate ZERO_EXTRACT/STRICT_LOW_PART */
        /* Modify specific bits using bitwise operations */
        bf.bits.low = (bf.bits.low + i) & 0x0F;      /* 4-bit field */
        bf.bits.mid = (bf.bits.mid ^ i) & 0xFF;      /* 8-bit field */
        bf.bits.high = (bf.bits.high | (i >> 2)) & 0x0F; /* 4-bit field */
        
        /* 3. Array access with computed index - creates complex MEM addresses */
        /* Use pointer arithmetic to force non-trivial addressing */
        int idx = (i * 7 + global_index) & 0x7;
        bf.byte_array[idx] = (bf.byte_array[idx] + i) & 0xFF;
        
        /* 4. Mixed operations with volatile to prevent optimization */
        /* Create data dependency chain */
        global_index = (global_index + bf.bits.low) & 0x3;
        
        /* 5. Pointer dereference with complex addressing */
        /* This should generate MEM with non-simple address expression */
        volatile int *ptr = ptr_array[global_index];
        *ptr = (*ptr + bf.bits.mid) & global_mask;
        
        /* 6. Accumulate results to prevent dead code elimination */
        accumulator += bf.full_word + bf.byte_array[idx] + *ptr;
        
        /* 7. Additional bit manipulation on the accumulated value */
        /* May generate ZERO_EXTRACT for partial word updates */
        accumulator = (accumulator & ~0xF0) | ((accumulator + i) & 0xF0);
    }
    
    /* Print result to ensure all operations are observable */
    printf("Result: %u\n", (unsigned int)accumulator);
    
    return 0;
}
