/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific uncovered lines in GCC's
 * resource.cc file (lines 282-290) by generating RTL patterns containing
 * ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM expressions.
 * 
 * Compilation options for coverage analysis:
 *   -O1 -fdump-rtl-all -fdump-rtl-expand
 *   -O2 -fschedule-insns -freschedule-modulo-scheduled-loops
 *   For BPF: -target bpf -O2 -fdump-rtl-all
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

/* Global variables to force memory operations with complex addressing */
static volatile int global_array[32];
static volatile struct bitfield_struct global_bf;

int main(void) {
    volatile int result = 0;
    volatile int temp;
    struct bitfield_struct local_bf;
    volatile int *ptr_array[8];
    
    /* Initialize pointer array with addresses at different offsets */
    for (int i = 0; i < 8; i++) {
        ptr_array[i] = &global_array[i * 3];
    }
    
    /* Initialize bit-field structure */
    local_bf.full_word = 0x12345678;
    local_bf.low_part = 0xA;
    local_bf.mid_part = 0xBC;
    local_bf.high_part = 0xD;
    local_bf.byte_field = 0xEF;
    
    global_bf = local_bf;
    
    /* Main loop to create complex RTL patterns */
    for (volatile int i = 0; i < 100; i++) {
        /* 1. Arithmetic operation that may generate SUBREG in RTL */
        volatile int32_t arithmetic_var = i * 3 + 7;
        
        /* 2. Bit-field assignment - may generate ZERO_EXTRACT/STRICT_LOW_PART */
        /* Modify specific bits of the full_word using bitwise operations */
        local_bf.full_word = (local_bf.full_word & 0xFFFF0000) | 
                            (arithmetic_var & 0x0000FFFF);
        
        /* Direct bit-field assignment (common source of ZERO_EXTRACT) */
        local_bf.mid_part = (i & 0xFF);  /* This often becomes ZERO_EXTRACT */
        
        /* 3. Complex memory addressing with pointer arithmetic */
        /* Create a computed index with data dependencies */
        int idx = (i * 7 + 3) & 0x1F;  /* Ensure within array bounds */
        
        /* Array access with complex addressing - may generate MEM with non-trivial address */
        temp = global_array[idx] + ptr_array[i & 0x7][(i >> 3) & 0x3];
        
        /* 4. Mixed operations to create data dependencies */
        /* Bit manipulation on memory location */
        global_array[idx] = (global_array[idx] & 0x00FFFFFF) | 
                           ((temp & 0xFF) << 24);
        
        /* 5. Structure field access with bit-field operations */
        /* This combination often produces the target RTL patterns */
        global_bf.byte_field = (global_bf.byte_field ^ temp) & 0x7F;
        
        /* 6. Additional bit-field manipulation */
        /* Mask specific bits - potential for ZERO_EXTRACT */
        local_bf.high_part = (local_bf.full_word >> 28) & 0xF;
        
        /* 7. Accumulate results to prevent optimization */
        result += arithmetic_var + temp + local_bf.full_word + 
                 global_bf.byte_field + idx;
        
        /* Force memory barrier-like behavior with volatile */
        asm volatile("" : : "r"(result) : "memory");
    }
    
    /* Additional complex memory operation outside loop */
    volatile int *complex_ptr = &global_array[0];
    for (int j = 0; j < 16; j++) {
        /* Pointer arithmetic creating complex addressing modes */
        complex_ptr[j * 2] = result & (0xFF << (j & 0x7));
        
        /* Bit-field extraction from memory */
        local_bf.low_part = (complex_ptr[j] >> 4) & 0xF;
    }
    
    /* Final output to prevent dead code elimination */
    printf("Result: %d\n", result);
    printf("Bitfield word: 0x%08X\n", local_bf.full_word);
    printf("Array[10]: %d\n", global_array[10]);
    
    return result & 0xFF;
}
