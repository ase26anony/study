/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific uncovered lines in GCC's
 * resource.cc file (lines 282-290) by generating RTL patterns containing
 * ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM expressions.
 * 
 * Compilation recommendations:
 *   - For general RTL expansion: gcc -O1 -fdump-rtl-all test_resource_coverage.c
 *   - For scheduling passes: gcc -O2 -fschedule-insns test_resource_coverage.c
 *   - For BPF target: gcc -target bpf -O2 -fdump-rtl-all test_resource_coverage.c
 */

#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT/STRICT_LOW_PART */
struct __attribute__((packed)) bitfield_struct {
    volatile uint32_t full_word;
    uint16_t low_part : 5;
    uint16_t mid_part : 7;
    uint16_t high_part : 4;
    volatile uint8_t byte_field;
};

/* Global variables to force memory operations */
volatile int global_index = 0;
static int static_array[8] = {1, 2, 3, 4, 5, 6, 7, 8};

int main(void) {
    struct bitfield_struct bf = {0};
    volatile int result = 0;
    volatile int temp;
    
    /* Array with pointer arithmetic to create complex MEM addresses */
    int local_array[16];
    for (int i = 0; i < 16; i++) {
        local_array[i] = i * 3;
    }
    
    /* Loop to create data dependencies and prevent optimization */
    for (int i = 0; i < 100; i++) {
        /* 1. Create SUBREG potential through arithmetic on mixed-size data */
        bf.full_word = (bf.full_word + i) & 0xFFFF;
        
        /* 2. Bit-field assignments - may generate ZERO_EXTRACT/STRICT_LOW_PART */
        bf.low_part = (bf.low_part + 1) & 0x1F;      /* 5-bit field */
        bf.mid_part = (bf.mid_part ^ i) & 0x7F;      /* 7-bit field */
        bf.high_part = (bf.high_part | 0x3) & 0xF;   /* 4-bit field */
        
        /* 3. Direct bit manipulation on volatile - may generate ZERO_EXTRACT */
        bf.byte_field = (bf.byte_field & 0xF0) | (i & 0x0F);
        
        /* 4. Complex MEM addressing with array indexing */
        int idx = (i + global_index) & 0x7;
        int *ptr = &local_array[idx * 2];
        
        /* Pointer arithmetic followed by dereference */
        temp = *(ptr + (i & 0x1));
        
        /* 5. Mixed operations to create dependencies */
        bf.full_word = bf.full_word ^ (temp << 3);
        
        /* 6. Access global through computed index */
        idx = (bf.low_part + bf.mid_part) & 0x7;
        temp = static_array[idx];
        
        /* 7. Bitwise assignment on result - may use ZERO_EXTRACT */
        result = (result & 0xFF00FF) | ((temp + i) & 0xFF);
        
        /* 8. Another complex MEM access with multiple components */
        int * volatile volatile_ptr = &local_array[0];
        volatile_ptr += (bf.high_part * 2 + 1) & 0xF;
        temp = *volatile_ptr;
        
        /* Accumulate results to prevent optimization */
        result += temp + bf.byte_field;
        
        /* Modify global to create loop-carried dependency */
        global_index = (global_index + 1) & 0x3;
    }
    
    /* Additional bit-field manipulation outside loop */
    struct bitfield_struct bf2 = {0};
    bf2.low_part = result & 0x1F;
    bf2.mid_part = (result >> 5) & 0x7F;
    bf2.high_part = (result >> 12) & 0xF;
    
    /* Force memory write with complex address */
    int *final_ptr = &local_array[(result + global_index) & 0xF];
    *final_ptr = bf2.full_word;
    
    /* Print result to ensure all operations are observable */
    printf("Result: %d\n", result + bf2.byte_field + *final_ptr);
    
    return 0;
}
