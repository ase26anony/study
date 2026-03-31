/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific uncovered lines in GCC's
 * resource.cc file (lines 282-290) by generating RTL patterns containing
 * ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM expressions.
 * 
 * Compilation recommendations:
 *   - For RTL expansion analysis: gcc -O1 -fdump-rtl-all -fdump-rtl-expand test_resource_coverage.c
 *   - For scheduling passes: gcc -O2 -fschedule-insns -freschedule-modulo-scheduled-loops test_resource_coverage.c
 *   - For BPF target: gcc -target bpf -O2 -fdump-rtl-all test_resource_coverage.c
 */

#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT/STRICT_LOW_PART */
struct __attribute__((packed)) bitfield_struct {
    volatile unsigned int field1 : 4;   /* Volatile to prevent optimization */
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 12;
    volatile unsigned int field4 : 8;
};

/* Global variables to force memory operations */
static volatile int global_array[16];
static volatile struct bitfield_struct global_bitfield;

int main(void) {
    volatile int result = 0;           /* Volatile to ensure observability */
    volatile int temp = 0;
    int i, j;
    
    /* Initialize global structures */
    global_bitfield.field1 = 1;
    global_bitfield.field2 = 2;
    global_bitfield.field3 = 3;
    global_bitfield.field4 = 4;
    
    for (i = 0; i < 16; i++) {
        global_array[i] = i * 3;
    }
    
    /* Main loop creating complex data dependencies */
    for (i = 0; i < 100; i++) {
        /* 1. Arithmetic operations that may generate SUBREG in RTL */
        volatile int base = i * 7 + 3;
        
        /* 2. Bit-field assignments - likely to generate ZERO_EXTRACT/STRICT_LOW_PART */
        /* Modify specific bits of base using bitwise operations */
        base = (base & ~0xF) | (i & 0xF);          /* Modify lower 4 bits */
        base = (base & ~(0x3F << 4)) | ((i * 2) << 4); /* Modify bits 4-9 */
        
        /* 3. Complex memory addressing with pointer arithmetic */
        volatile int *ptr = (volatile int *)global_array;
        
        /* Create non-trivial address expression: array + (base & mask) */
        int index = (base + i) & 0xF;  /* Ensure index stays in bounds */
        
        /* This should generate MEM with complex addressing */
        temp = ptr[index];
        
        /* 4. More bit manipulation on the loaded value */
        /* Clear specific bits then set others */
        temp = (temp & ~0xFF) | (base & 0xFF);
        
        /* 5. Access bit-field structure members */
        /* These assignments often generate ZERO_EXTRACT for packed bit-fields */
        global_bitfield.field2 = (temp >> 4) & 0xFF;
        global_bitfield.field3 = (temp >> 8) & 0xFFF;
        
        /* 6. Mixed operations to create data dependencies */
        volatile int *complex_ptr = (volatile int *)&global_bitfield;
        
        /* Cast to char pointer and back to force unusual memory access */
        volatile char *char_ptr = (volatile char *)complex_ptr;
        char_ptr[(i & 3)] = (temp & 0xFF);
        
        /* 7. Accumulate results with bitwise operations */
        result ^= temp;                /* XOR accumulation */
        result += global_bitfield.field2;
        result -= global_bitfield.field3;
        
        /* 8. Additional pointer manipulation for complex MEM addresses */
        volatile int **ptr_to_ptr = &ptr;
        (*ptr_to_ptr)[(i + 1) & 0xF] = result & 0x7F;
    }
    
    /* Final computation using all modified values */
    result += global_bitfield.field1;
    result += global_bitfield.field4;
    
    for (j = 0; j < 16; j++) {
        result += global_array[j];
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", (int)result);
    
    return 0;
}
