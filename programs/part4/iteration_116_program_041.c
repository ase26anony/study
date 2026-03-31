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
struct __attribute__((packed)) packed_data {
    volatile uint32_t full_word;
    struct {
        uint32_t low_bits : 8;
        uint32_t mid_bits : 12;
        uint32_t high_bits : 10;
        uint32_t spare : 2;
    } bitfield;
    volatile uint8_t byte_array[16];
};

/* Global variables to force memory operations */
static volatile struct packed_data global_data;
static volatile int32_t global_index = 0;

int main(void) {
    struct packed_data local_data;
    volatile uint32_t accumulator = 0;
    volatile int32_t temp_index;
    
    /* Initialize data */
    global_data.full_word = 0xDEADBEEF;
    local_data.full_word = 0xCAFEBABE;
    
    for (int i = 0; i < 100; i++) {
        /* 1. Create arithmetic operations that may generate SUBREG in RTL */
        temp_index = (global_index + i * 3) & 0xF;
        
        /* 2. Bit-field assignments to encourage ZERO_EXTRACT/STRICT_LOW_PART */
        /* Modify specific bits of the full_word using bitwise operations */
        global_data.full_word &= ~(0xFF << 8);      /* Clear bits 8-15 */
        global_data.full_word |= (i & 0xFF) << 8;   /* Set bits 8-15 with loop counter */
        
        /* Direct bit-field manipulation (common source of ZERO_EXTRACT) */
        local_data.bitfield.mid_bits = (i * 7) & 0xFFF;
        local_data.bitfield.low_bits = (i + temp_index) & 0xFF;
        
        /* 3. Complex memory addressing to reach MEM_P(x) path */
        /* Array access with computed index creates complex address expressions */
        uint8_t value = local_data.byte_array[temp_index];
        local_data.byte_array[(temp_index + 1) & 0xF] = value ^ (i & 0xFF);
        
        /* Pointer arithmetic for additional MEM complexity */
        volatile uint8_t *ptr = &local_data.byte_array[0];
        ptr += (temp_index * 2) & 0xF;
        *ptr = (*ptr + i) & 0x7F;
        
        /* 4. Mixed operations to create data dependencies */
        /* Combine arithmetic with bitwise operations */
        uint32_t mixed = local_data.full_word;
        mixed += (temp_index << 3);          /* Arithmetic - may use SUBREG */
        mixed &= ~(0x3FF << 5);              /* Bitwise - may use ZERO_EXTRACT */
        mixed |= (accumulator & 0x3FF) << 5; /* Another potential ZERO_EXTRACT */
        local_data.full_word = mixed;
        
        /* 5. Accumulate results to prevent optimization */
        accumulator += global_data.full_word;
        accumulator += local_data.full_word;
        accumulator += temp_index;
        
        /* Update global index with complex expression */
        global_index = (global_index * 13 + i) & 0xFF;
    }
    
    /* Final output to ensure all operations are observable */
    printf("Result: %u\n", (unsigned int)accumulator);
    printf("Global word: 0x%08X\n", (unsigned int)global_data.full_word);
    printf("Local word: 0x%08X\n", (unsigned int)local_data.full_word);
    
    return 0;
}
