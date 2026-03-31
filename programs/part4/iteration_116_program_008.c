/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific uncovered lines in GCC's
 * resource.cc file (lines 282-290) by generating RTL patterns containing
 * ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM expressions.
 * 
 * Compilation recommendations:
 *   For RTL analysis: gcc -O1 -fdump-rtl-all -fdump-rtl-expand test_resource_coverage.c
 *   For scheduling passes: gcc -O2 -fschedule-insns -freschedule-modulo-scheduled-loops test_resource_coverage.c
 *   For BPF target: gcc -target bpf -O2 -fdump-rtl-all test_resource_coverage.c
 */

#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT/STRICT_LOW_PART */
struct __attribute__((packed)) bitfield_struct {
    volatile uint32_t full_word;
    struct {
        uint32_t low_bits : 8;
        uint32_t mid_bits : 12;
        uint32_t high_bits : 10;
        uint32_t flag : 2;
    } parts;
};

/* Global array to create complex MEM addresses */
static volatile uint32_t global_array[64];

int main(void) {
    struct bitfield_struct bf = {0};
    volatile uint32_t accumulator = 0;
    volatile uint32_t index_reg = 0;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 64; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    /* Main loop to generate complex RTL patterns */
    for (volatile uint32_t i = 0; i < 100; i++) {
        /* 1. Create arithmetic operations that may generate SUBREG */
        uint32_t temp = i * 7;
        temp += (i << 3) | (i >> 2);  /* Mixed arithmetic/bitwise */
        
        /* 2. Bit-field assignments to encourage ZERO_EXTRACT/STRICT_LOW_PART */
        bf.parts.low_bits = temp & 0xFF;          /* Likely ZERO_EXTRACT */
        bf.parts.mid_bits = (temp >> 8) & 0xFFF;  /* Another bit-field */
        bf.parts.high_bits = (temp >> 20) & 0x3FF;
        
        /* 3. Complex memory addressing for MEM_P(x) path */
        /* Create index with arithmetic to prevent simple addressing */
        index_reg = (i * 13 + temp) & 0x3F;  /* 0-63 range */
        
        /* Array access with computed index - creates complex MEM address */
        uint32_t array_val = global_array[index_reg];
        
        /* 4. Mixed operations on the loaded value */
        /* Modify specific bits of the loaded value */
        array_val &= ~(0xF << 4);    /* Clear bits 4-7 */
        array_val |= (temp & 0xF) << 4;  /* Set them from temp */
        
        /* Store back with potentially complex addressing */
        global_array[index_reg] = array_val;
        
        /* 5. Accumulate results to prevent optimization */
        accumulator ^= bf.full_word;
        accumulator += array_val;
        accumulator ^= temp;
        
        /* 6. Additional pointer arithmetic for complex MEM addresses */
        volatile uint32_t *ptr = &global_array[0];
        ptr += (i & 0x1F);  /* Variable offset */
        accumulator += *ptr;
        
        /* 7. Nested bit-field access */
        bf.parts.flag = (accumulator >> 30) & 0x3;
    }
    
    /* Final output to prevent dead code elimination */
    printf("Result: %u\n", accumulator);
    
    return 0;
}
