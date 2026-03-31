/* test_resource_cc.c
 * 
 * This program is designed to trigger specific uncovered lines in GCC's
 * resource.cc file (lines 282-290) by generating RTL patterns containing
 * ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM expressions.
 * 
 * Compilation recommendations:
 *   gcc -O1 -fdump-rtl-all -fdump-rtl-expand test_resource_cc.c -o test
 *   gcc -O2 -fschedule-insns -freschedule-modulo-scheduled-loops test_resource_cc.c -o test
 *   For BPF: clang -target bpf -O2 -fdump-rtl-all test_resource_cc.c -o test.bpf
 */

#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT/STRICT_LOW_PART */
struct __attribute__((packed)) bitfield_struct {
    volatile uint32_t full_word;
    uint16_t low_part : 4;   /* Likely to generate STRICT_LOW_PART */
    uint16_t mid_part : 8;   /* May generate ZERO_EXTRACT */
    uint16_t high_part : 4;  /* Another bit-field for extraction */
};

/* Global variables to force memory operations */
static volatile int global_index = 0;
static volatile int global_mask = 0x0F0F0F0F;

/* Array with complex addressing */
static volatile uint32_t mem_array[256];

int main(void) {
    struct bitfield_struct bf = {0};
    volatile uint32_t accumulator = 0;
    volatile uint32_t *ptr_array[4];
    
    /* Initialize pointer array with different offsets into mem_array */
    for (int i = 0; i < 4; i++) {
        ptr_array[i] = &mem_array[i * 16];
    }
    
    /* Main loop to create complex RTL patterns */
    for (volatile int i = 0; i < 100; i++) {
        /* 1. Arithmetic operation that may create SUBREG in RTL */
        uint32_t temp = bf.full_word + i * 3;
        
        /* 2. Bit-field assignments - likely to generate ZERO_EXTRACT/STRICT_LOW_PART */
        bf.low_part = (temp & 0xF);           /* STRICT_LOW_PART candidate */
        bf.mid_part = ((temp >> 4) & 0xFF);   /* ZERO_EXTRACT candidate */
        bf.high_part = ((temp >> 12) & 0xF);  /* Another bit-field extraction */
        
        /* 3. Complex memory addressing with pointer arithmetic */
        /* This creates MEM with non-trivial address expression */
        int idx = (i * 7 + global_index) & 0x3;
        volatile uint32_t *ptr = ptr_array[idx] + (i & 0xF);
        
        /* 4. Mixed bitwise and arithmetic operations */
        *ptr = (*ptr & global_mask) | (temp << 8);
        
        /* 5. Additional bit manipulation on memory location */
        /* May generate ZERO_EXTRACT when combined with volatile */
        uint32_t masked = *ptr & 0x0000FFFF;
        *ptr = (*ptr & 0xFFFF0000) | ((masked + i) & 0xFFFF);
        
        /* 6. Array access with computed index - complex MEM address */
        int array_idx = (i * 13 + global_index) % 256;
        mem_array[array_idx] = mem_array[array_idx] ^ temp;
        
        /* 7. Update accumulator to prevent optimization */
        accumulator ^= bf.full_word ^ *ptr ^ mem_array[array_idx];
        
        /* 8. Modify global_index to create data dependencies */
        global_index = (global_index + accumulator) & 0xFF;
    }
    
    /* Final computation to use all variables */
    uint32_t result = accumulator + bf.full_word + global_index;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %u\n", (unsigned int)result);
    
    return (int)(result & 0x7FFFFFFF);
}
