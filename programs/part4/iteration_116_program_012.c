/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific uncovered lines in GCC's
 * resource.cc file (lines 282-290) by generating RTL patterns containing
 * ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM expressions.
 * 
 * Compilation recommendations:
 *   For RTL analysis: gcc -O1 -fdump-rtl-all -fdump-rtl-expand test_resource_coverage.c
 *   For scheduling:   gcc -O2 -fschedule-insns -freschedule-modulo-scheduled-loops test_resource_coverage.c
 *   For BPF target:   clang -target bpf -O2 -fdump-rtl-all test_resource_coverage.c
 */

#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT/STRICT_LOW_PART */
struct __attribute__((packed)) packed_bitfield {
    volatile uint32_t full;
    struct {
        volatile uint16_t low : 4;
        volatile uint16_t mid : 8;
        volatile uint16_t high : 4;
    } bits;
    volatile uint8_t array[8];
};

/* Global variables to force memory operations */
static volatile int global_index = 1;
static volatile int global_mask = 0x0F;

int main(void) {
    struct packed_bitfield data;
    volatile uint32_t accumulator = 0;
    volatile int *ptr_array[4];
    volatile int dummy;
    
    /* Initialize data */
    data.full = 0x12345678;
    data.bits.low = 0x5;
    data.bits.mid = 0xAB;
    data.bits.high = 0x3;
    
    for (int i = 0; i < 8; i++) {
        data.array[i] = i * 3;
    }
    
    /* Setup pointer array with complex addressing */
    for (int i = 0; i < 4; i++) {
        ptr_array[i] = &data.array[i] + global_index;
    }
    
    /* Main loop to generate complex RTL patterns */
    for (volatile int i = 0; i < 100; i++) {
        /* 1. Arithmetic operation that may generate SUBREG in RTL */
        uint32_t temp = data.full;
        temp = temp + (i * 0x1111);
        
        /* 2. Bit-field assignment - may generate ZERO_EXTRACT/STRICT_LOW_PART */
        /* Modify specific bits using bitwise operations */
        data.bits.mid = (data.bits.mid & 0xF0) | (temp & 0x0F);
        
        /* 3. Complex memory addressing - may generate MEM with non-trivial address */
        /* Array access with computed index */
        int idx = (i + global_index) & 0x7;
        volatile uint8_t *addr = &data.array[idx] + (temp & 0x3);
        
        /* 4. Mixed operation: arithmetic then bitwise on result */
        *addr = (*addr + 1) & global_mask;
        
        /* 5. Another bit-field manipulation */
        /* This assignment to a specific bit range may generate STRICT_LOW_PART */
        data.bits.low = (temp >> 4) & 0x0F;
        
        /* 6. Pointer arithmetic with dereference - complex MEM address */
        int ptr_idx = (i * 3) % 4;
        volatile int *complex_ptr = ptr_array[ptr_idx] + (i & 0x1);
        *complex_ptr = *complex_ptr ^ (temp & 0xFF);
        
        /* 7. Update accumulator to prevent optimization */
        accumulator += data.full + *addr + data.bits.low + data.bits.mid;
        
        /* 8. Additional bit manipulation on the full word */
        /* This may generate ZERO_EXTRACT for partial word updates */
        data.full = (data.full & 0xFFFF0000) | (temp & 0x0000FFFF);
    }
    
    /* Final computation to use all modified data */
    volatile uint32_t result = accumulator;
    
    /* Add bit-field values */
    result += (data.bits.low << 0);
    result += (data.bits.mid << 4);
    result += (data.bits.high << 12);
    
    /* Add array values with complex indexing */
    for (int i = 0; i < 8; i++) {
        int idx = (i + global_index) & 0x7;
        result += data.array[idx];
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %u\n", (unsigned int)result);
    
    return 0;
}
