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
    uint16_t field_a : 4;
    uint16_t field_b : 5;
    uint16_t field_c : 7;
    volatile uint8_t byte_field;
};

/* Global variables to force memory operations */
static volatile int global_index = 0;
static volatile int global_mask = 0x1F;

int main(void) {
    struct bitfield_struct bf = {0};
    volatile uint32_t accumulator = 0;
    
    /* Array with volatile elements to prevent optimization */
    volatile uint32_t mem_array[16] = {0};
    
    /* Initialize with non-zero values */
    for (int i = 0; i < 16; i++) {
        mem_array[i] = i * 3 + 1;
    }
    
    /* Main loop to generate complex RTL patterns */
    for (volatile int i = 0; i < 100; i++) {
        /* 1. Arithmetic operation that may generate SUBREG in RTL */
        uint32_t temp = bf.full_word + i * 7;
        
        /* 2. Bit-field assignment - may generate ZERO_EXTRACT/STRICT_LOW_PART */
        /* Access through pointer to volatile to force memory RTL */
        volatile struct bitfield_struct *bf_ptr = &bf;
        
        /* Modify specific bits using bitwise operations */
        bf_ptr->field_a = (temp >> 2) & 0xF;      /* Likely ZERO_EXTRACT */
        bf_ptr->field_b = (temp >> 6) & 0x1F;     /* Likely ZERO_EXTRACT */
        bf_ptr->field_c = (temp >> 11) & 0x7F;    /* Likely ZERO_EXTRACT */
        
        /* 3. Complex memory addressing - may generate MEM with non-trivial address */
        /* Compute index with arithmetic and bit manipulation */
        int idx = (i * 13 + (bf.field_a << 1)) & 0xF;
        
        /* Array access with computed index - complex addressing mode */
        uint32_t array_val = mem_array[idx];
        
        /* 4. Mixed operations that may generate SUBREG */
        /* Combine arithmetic with bitwise masking */
        bf.byte_field = (array_val + temp) & 0xFF;
        
        /* 5. Additional bit manipulation on memory location */
        /* This may generate ZERO_EXTRACT when combined with volatile */
        bf_ptr->full_word ^= (1 << (i & 0x1F));
        
        /* 6. Pointer arithmetic followed by dereference */
        /* Create complex MEM address expression */
        volatile uint32_t *ptr = mem_array + ((i * 3) % 16);
        *ptr = *ptr + bf.byte_field;
        
        /* 7. Accumulate results to prevent optimization */
        accumulator += bf.full_word + array_val + bf.byte_field;
        
        /* Force memory barrier-like behavior */
        global_index = idx;
    }
    
    /* Additional test case: nested bit-field operations */
    {
        struct bitfield_struct local_bf = {0};
        volatile struct bitfield_struct *volatile_bf_ptr = &local_bf;
        
        /* Multiple bit-field assignments in sequence */
        for (int j = 0; j < 10; j++) {
            /* These assignments are likely to generate ZERO_EXTRACT/STRICT_LOW_PART */
            volatile_bf_ptr->field_a = (accumulator >> (j * 2)) & 0xF;
            volatile_bf_ptr->field_b = (accumulator >> (j * 3)) & 0x1F;
            volatile_bf_ptr->field_c = (accumulator >> (j * 4)) & 0x7F;
            
            /* Complex memory reference */
            int offset = (j * 5) & 0xF;
            mem_array[offset] = local_bf.full_word;
            
            accumulator += mem_array[offset];
        }
    }
    
    /* Print result to ensure all operations are observable */
    printf("Result: %u\n", (unsigned int)accumulator);
    
    return 0;
}
