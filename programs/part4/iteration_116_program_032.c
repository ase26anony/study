/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific uncovered lines in GCC's
 * resource.cc file (lines 282-290) by generating RTL patterns containing
 * ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM expressions.
 * 
 * Compilation recommendations:
 *   gcc -O1 -fdump-rtl-all -fdump-rtl-expand test_resource_coverage.c -o test
 *   gcc -O2 -fschedule-insns -freschedule-modulo-scheduled-loops test_resource_coverage.c -o test
 *   For BPF: clang -target bpf -O2 -fdump-rtl-all test_resource_coverage.c -o test.bpf
 */

#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT/STRICT_LOW_PART */
struct __attribute__((packed)) bitfield_struct {
    volatile uint32_t full_word;
    struct {
        uint32_t low_bits : 4;
        uint32_t mid_bits : 8;
        uint32_t high_bits : 12;
        uint32_t reserved : 8;
    } parts;
};

/* Global variables to force memory operations */
static volatile int global_index = 0;
static volatile int global_mask = 0x0F;

int main(void) {
    /* Local volatile variables to prevent optimization */
    volatile uint32_t base_value = 0x12345678;
    volatile uint32_t accumulator = 0;
    volatile uint32_t temp_result = 0;
    
    /* Packed structure with bit-fields */
    struct bitfield_struct bf = {
        .full_word = 0xABCDEF01,
        .parts = {.low_bits = 1, .mid_bits = 0x23, .high_bits = 0x456, .reserved = 0x78}
    };
    
    /* Small array with complex addressing */
    volatile uint32_t data_array[16];
    for (int i = 0; i < 16; i++) {
        data_array[i] = i * 0x11111111;
    }
    
    /* Loop to create data dependencies and complex RTL patterns */
    for (volatile int i = 0; i < 100; i++) {
        /* 1. Arithmetic operation that may generate SUBREG in RTL */
        base_value = base_value + (i * 0x10001);
        
        /* 2. Bit-field assignment - may generate ZERO_EXTRACT/STRICT_LOW_PART */
        /* Direct bit-field modification */
        bf.parts.mid_bits = (bf.parts.mid_bits + i) & 0xFF;
        
        /* Manual bit manipulation on volatile - may also generate extract */
        base_value = (base_value & ~0xF0) | ((i & 0xF) << 4);
        
        /* 3. Complex memory addressing with pointer arithmetic */
        /* Compute index using mixed operations */
        int idx = (i * 3 + (base_value & 0x3)) & 0xF;
        
        /* Array access with computed index - creates complex MEM address */
        temp_result = data_array[idx];
        
        /* 4. More bit manipulation with memory destination */
        /* Modify specific bits of array element */
        data_array[(idx + 1) & 0xF] = (data_array[(idx + 1) & 0xF] & ~0xFF00) | 
                                     ((temp_result & 0xFF) << 8);
        
        /* 5. Mixed operations to create dependencies */
        /* Use bit-field value in calculation */
        accumulator += bf.parts.mid_bits * temp_result;
        
        /* Modify full_word through bit-field parts */
        bf.full_word = (bf.full_word & 0xFFFF0000) | 
                      (bf.parts.low_bits << 0) | 
                      (bf.parts.mid_bits << 4) | 
                      (bf.parts.high_bits << 12);
    }
    
    /* Final computation to ensure all operations are observable */
    volatile uint32_t final_result = accumulator + bf.full_word + base_value;
    
    /* Print result to prevent dead code elimination */
    printf("Result: 0x%08X\n", (unsigned int)final_result);
    
    return (final_result != 0) ? 0 : 1;
}
