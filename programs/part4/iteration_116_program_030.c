/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific uncovered lines in GCC's
 * resource.cc file (lines 282-290) by generating RTL patterns containing
 * ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM expressions.
 * 
 * Compilation recommendations:
 *   - For general RTL generation: gcc -O1 -fdump-rtl-all -c test_resource_coverage.c
 *   - For scheduling passes: gcc -O2 -fschedule-insns -c test_resource_coverage.c
 *   - For BPF target: gcc -target bpf -O2 -fdump-rtl-all -c test_resource_coverage.c
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

/* Function to create data dependencies and complex addressing */
static volatile int compute_index(volatile int a, volatile int b) {
    return (a ^ b) & 0xF;  /* Force non-trivial computation */
}

int main(void) {
    volatile int result = 0;
    volatile int base = 0x12345678;
    volatile int temp;
    volatile int *ptr_array[4];
    
    /* Initialize pointer array with different offsets into global_array */
    for (int i = 0; i < 4; i++) {
        ptr_array[i] = &global_array[i * 3];  /* Non-linear addressing */
    }
    
    /* Initialize bitfield structure */
    global_bitfield.field1 = 0xA;
    global_bitfield.field2 = 0xBC;
    global_bitfield.field3 = 0xDEF;
    global_bitfield.field4 = 0x12;
    
    /* Main loop to generate complex RTL patterns */
    for (volatile int i = 0; i < 100; i++) {
        /* 1. Arithmetic operation that may generate SUBREG in RTL */
        base = base + i * 0x1111;
        
        /* 2. Bitwise assignment to specific bits - may generate ZERO_EXTRACT */
        /* Clear bits 8-15, then set bits 4-7 */
        base = (base & ~0xFF00) | ((i & 0xF) << 4);
        
        /* 3. Complex memory access with computed index */
        int idx = compute_index(base, i);
        
        /* Array access with variable index - creates complex MEM address */
        temp = global_array[idx];
        
        /* 4. Pointer arithmetic and dereference - another complex MEM */
        volatile int *ptr = ptr_array[i % 4] + (base & 0x3);
        temp += *ptr;
        
        /* 5. Bit-field assignments - likely to generate ZERO_EXTRACT/STRICT_LOW_PART */
        global_bitfield.field2 = (temp >> 4) & 0xFF;      /* ZERO_EXTRACT pattern */
        global_bitfield.field3 = (global_bitfield.field3 + i) & 0xFFF;
        
        /* 6. Mixed operation: modify specific bits of memory location */
        /* This should generate ZERO_EXTRACT as destination */
        global_array[idx] = (global_array[idx] & ~0xF0) | ((i & 0xF) << 4);
        
        /* 7. Accumulate results with bit manipulation */
        result ^= base;
        result += temp;
        result += global_bitfield.field2;
        result += global_bitfield.field3;
        
        /* 8. Additional complex addressing through multiple indirections */
        if (i & 1) {
            volatile int **pptr = &ptr_array[(i >> 1) & 3];
            result += **pptr;
        }
    }
    
    /* Final output to prevent dead code elimination */
    printf("Result: %d\n", (int)result);
    printf("Base: 0x%x\n", (unsigned int)base);
    printf("Bitfield: %u %u %u %u\n", 
           global_bitfield.field1,
           global_bitfield.field2,
           global_bitfield.field3,
           global_bitfield.field4);
    
    return 0;
}
