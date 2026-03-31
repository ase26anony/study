/* test_resource_cc.c
 * 
 * This program is designed to trigger specific uncovered lines in GCC's
 * resource.cc file (lines 282-290) by generating RTL patterns containing
 * ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM expressions.
 * 
 * Compilation recommendations:
 *   - For general RTL generation: gcc -O1 -fdump-rtl-all test_resource_cc.c
 *   - For scheduling passes: gcc -O2 -fschedule-insns test_resource_cc.c
 *   - For BPF target: gcc -target bpf -O2 -fdump-rtl-all test_resource_cc.c
 */

#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT generation */
struct __attribute__((packed)) bitfield_struct {
    volatile uint32_t full_word;
    uint16_t low_part : 8;
    uint16_t mid_part : 12;
    uint16_t high_part : 12;
};

/* Global variables to create complex addressing modes */
static volatile int global_array[32];
static volatile struct bitfield_struct global_bf;

int main(void) {
    volatile int result = 0;
    volatile int temp;
    
    /* Local packed structure with bit-fields */
    struct __attribute__((packed)) local_bf {
        volatile uint32_t base;
        uint16_t field_a : 4;
        uint16_t field_b : 6;
        uint16_t field_c : 10;
        uint16_t field_d : 12;
    } local;
    
    /* Initialize structures */
    local.base = 0x12345678;
    local.field_a = 0xA;
    local.field_b = 0x15;
    local.field_c = 0x1FF;
    local.field_d = 0xABC;
    
    global_bf.full_word = 0x87654321;
    global_bf.low_part = 0x42;
    global_bf.mid_part = 0x7FF;
    global_bf.high_part = 0xABC;
    
    /* Loop to create data dependencies and complex RTL patterns */
    for (volatile int i = 0; i < 16; i++) {
        /* 1. Arithmetic operations that may generate SUBREG in RTL */
        temp = i * 3 + 7;
        
        /* 2. Bit-field assignments - may generate ZERO_EXTRACT/STRICT_LOW_PART */
        /* Modify specific bits of local.base using bitwise operations */
        local.base = (local.base & ~(0xF << 8)) | ((temp & 0xF) << 8);
        
        /* Direct bit-field assignment - common source of ZERO_EXTRACT */
        local.field_b = (temp >> 1) & 0x3F;
        
        /* 3. Complex memory addressing - may generate MEM with non-trivial address */
        /* Array access with computed index using arithmetic and bit operations */
        int idx = (i + (local.field_a & 0x3)) & 0x1F;
        global_array[idx] = temp * 2;
        
        /* Pointer arithmetic to create complex addressing */
        volatile int *ptr = &global_array[0];
        ptr += (idx * 3) / 2;
        *ptr += local.field_c;
        
        /* 4. Mixed operations on global bit-field structure */
        /* This may generate both ZERO_EXTRACT and complex MEM patterns */
        global_bf.mid_part = (global_bf.mid_part + i) & 0x7FF;
        
        /* 5. Access through pointer with offset calculation */
        volatile struct bitfield_struct *bf_ptr = &global_bf;
        bf_ptr->low_part = (bf_ptr->low_part ^ temp) & 0xFF;
        
        /* Accumulate results to prevent optimization */
        result += local.base + global_array[idx] + bf_ptr->full_word;
        
        /* Additional bit manipulation to encourage ZERO_EXTRACT */
        uint32_t mask = 1 << (i & 0x1F);
        local.base ^= mask;  /* Toggle specific bit */
        
        /* Structure field access with bit masking */
        local.field_d = (local.field_d + (result & 0xFFF)) & 0xFFF;
    }
    
    /* Final computation using all modified values */
    result += local.field_a + local.field_b + local.field_c + local.field_d;
    result += global_bf.low_part + global_bf.mid_part + global_bf.high_part;
    
    /* Print result to ensure all operations are observable */
    printf("Result: %d\n", (int)result);
    
    return 0;
}
