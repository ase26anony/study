/* test_resource_cc.c
 * 
 * This program is designed to trigger specific uncovered lines in GCC's
 * resource.cc file (lines 282-290) by generating RTL patterns containing
 * ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM expressions.
 *
 * Compilation recommendations:
 *   For RTL analysis: gcc -O1 -fdump-rtl-all -fdump-rtl-expand test_resource_cc.c
 *   For scheduling passes: gcc -O2 -fschedule-insns test_resource_cc.c
 *   For BPF target: clang -target bpf -O2 -c test_resource_cc.c
 */

#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT generation */
struct __attribute__((packed)) bitfield_struct {
    volatile uint32_t full_word;
    uint16_t low_part : 4;
    uint16_t mid_part : 8;
    uint16_t high_part : 4;
    volatile uint8_t byte_field;
};

/* Global array to create complex MEM addresses */
static volatile uint32_t global_array[32];

int main(void) {
    struct bitfield_struct bf = {0};
    volatile uint32_t accumulator = 0;
    volatile uint32_t index_reg = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 32; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    /* Main loop to create data dependencies and complex RTL patterns */
    for (volatile int i = 0; i < 100; i++) {
        /* 1. Arithmetic operation that may generate SUBREG in RTL */
        bf.full_word = bf.full_word + i * 7;
        
        /* 2. Bit-field assignments - may generate ZERO_EXTRACT/STRICT_LOW_PART */
        bf.low_part = (bf.low_part + 1) & 0x0F;      /* 4-bit field */
        bf.mid_part = (bf.mid_part ^ i) & 0xFF;      /* 8-bit field */
        bf.high_part = (bf.high_part | 0x5) & 0x0F;  /* 4-bit field */
        
        /* 3. Bitwise operation on full word - may create ZERO_EXTRACT */
        bf.full_word = (bf.full_word & 0xFFFF00FF) | ((i & 0xFF) << 8);
        
        /* 4. Complex memory addressing with pointer arithmetic */
        uint32_t idx = (i * 13 + bf.low_part) % 32;
        
        /* Multiple operations to create addressing complexity */
        volatile uint32_t *ptr = &global_array[0];
        ptr += idx;                     /* Pointer arithmetic */
        ptr += (bf.mid_part >> 4);      /* More complex addressing */
        
        /* 5. Memory access with complex address - may generate MEM_P(x) path */
        accumulator ^= *ptr;
        accumulator += bf.full_word;
        
        /* 6. Additional bit manipulation to encourage ZERO_EXTRACT */
        bf.byte_field = (bf.byte_field << 1) | ((accumulator >> 3) & 1);
        
        /* 7. Mixed operations to prevent optimization */
        index_reg = (index_reg * 3 + 1) & 0x1F;
        global_array[index_reg] = accumulator;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %u\n", accumulator);
    printf("Bitfield word: %u\n", bf.full_word);
    printf("Byte field: %u\n", (uint32_t)bf.byte_field);
    
    return (int)(accumulator & 0xFF);
}
