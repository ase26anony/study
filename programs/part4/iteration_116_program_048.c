/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific uncovered lines in GCC's
 * resource.cc file (lines 282-290) by generating RTL patterns containing
 * ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM expressions.
 * 
 * Compilation recommendations:
 *   For RTL expansion: gcc -O1 -fdump-rtl-all -fdump-rtl-expand test_resource_coverage.c
 *   For scheduling:    gcc -O2 -fschedule-insns -freschedule-modulo-scheduled-loops test_resource_coverage.c
 *   For BPF target:   gcc -target bpf -O2 -fdump-rtl-all test_resource_coverage.c
 */

#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT/STRICT_LOW_PART */
struct __attribute__((packed)) packed_data {
    unsigned int field1 : 4;   /* 4-bit field */
    unsigned int field2 : 12;  /* 12-bit field */
    unsigned int field3 : 8;   /* 8-bit field */
    unsigned int field4 : 8;   /* 8-bit field */
};

/* Volatile variables to prevent optimization */
static volatile struct packed_data global_packed;
static volatile uint32_t global_array[16];
static volatile uint32_t global_result = 0;

int main(void) {
    /* Local volatile variables to force memory operations */
    volatile uint32_t base_value = 0x12345678;
    volatile uint32_t temp_result = 0;
    volatile uint32_t index_reg = 0;
    
    /* Pointer to manipulate for complex MEM addresses */
    volatile uint32_t *ptr = &global_array[0];
    
    /* Loop to create data dependencies and inhibit simple addressing */
    for (int i = 0; i < 100; i++) {
        /* 1. Arithmetic operation that may generate SUBREG in RTL */
        base_value = base_value + (i * 3);
        
        /* 2. Bit-field assignment to packed structure - may generate ZERO_EXTRACT/STRICT_LOW_PART */
        /* Access through pointer to create MEM with complex address */
        struct packed_data *packed_ptr = (struct packed_data *)&global_packed;
        
        /* Modify specific bits of field2 (12-bit field) */
        packed_ptr->field2 = (packed_ptr->field2 & 0xF0F) | ((i & 0xF) << 4);
        
        /* 3. Complex memory addressing with pointer arithmetic */
        /* Create index with arithmetic and bit manipulation */
        index_reg = (i * 7 + (base_value & 0x3F)) % 16;
        
        /* Array access with computed index - may generate complex MEM expression */
        temp_result = global_array[index_reg];
        
        /* 4. Bitwise operation on memory location through pointer */
        /* This may generate ZERO_EXTRACT for bit-field-like access */
        uint32_t *int_ptr = (uint32_t *)packed_ptr;
        *int_ptr = (*int_ptr & 0xFFFF0000) | (temp_result & 0x0000FFFF);
        
        /* 5. Additional pointer manipulation for complex addressing */
        ptr = &global_array[(i + (base_value >> 8)) & 0xF];
        *ptr = *ptr + (i & 0xFF);
        
        /* 6. Mixed operations to create dependencies */
        /* Modify specific bits of base_value (simulating bit-field on integer) */
        base_value = (base_value & 0xFFFFFF00) | ((base_value + i) & 0x000000FF);
        
        /* 7. Accumulate results to prevent dead code elimination */
        global_result += base_value + temp_result + packed_ptr->field2 + *ptr;
        
        /* 8. Additional bit-field manipulation */
        /* STRICT_LOW_PART may be generated for this kind of partial update */
        packed_ptr->field3 = (packed_ptr->field3 ^ i) & 0x7F;
    }
    
    /* Final output to ensure all operations are observable */
    printf("Result: %u\n", (unsigned int)global_result);
    printf("Base value: 0x%08x\n", (unsigned int)base_value);
    printf("Packed data fields: %u, %u, %u\n", 
           global_packed.field1, global_packed.field2, global_packed.field3);
    
    return (int)(global_result & 0x7FFFFFFF);
}
