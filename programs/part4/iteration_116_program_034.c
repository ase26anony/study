/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific uncovered lines in GCC's
 * resource.cc file (lines 282-290) by generating RTL patterns containing
 * ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM expressions.
 * 
 * Compilation recommendations:
 *   gcc -O1 -fdump-rtl-all -fdump-rtl-expand test_resource_coverage.c -o test
 *   gcc -O2 -fschedule-insns -freschedule-modulo-scheduled-loops test_resource_coverage.c -o test
 *   For BPF: clang -target bpf -O2 -fdump-rtl-all test_resource_coverage.c -o test.bpf
 */

#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT generation */
struct __attribute__((packed)) bitfield_struct {
    volatile unsigned int field1 : 4;   /* volatile prevents optimization */
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 12;
    volatile unsigned int field4 : 8;
};

/* Global variables to force memory operations */
static volatile int global_base = 0x12345678;
static volatile int global_array[16] = {0};

int main(void) {
    struct bitfield_struct bf = {0};
    volatile int result = 0;  /* volatile to prevent optimization */
    int i, j;
    
    /* Initialize array with pattern */
    for (i = 0; i < 16; i++) {
        global_array[i] = i * 0x1111;
    }
    
    /* Main loop to create complex RTL patterns */
    for (i = 0; i < 100; i++) {
        volatile int temp = global_base + i;  /* Creates arithmetic dependency */
        
        /* 1. Bit-field assignments - may generate ZERO_EXTRACT/STRICT_LOW_PART */
        bf.field1 = (temp >> 4) & 0xF;        /* 4-bit field */
        bf.field2 = (temp >> 8) & 0xFF;       /* 8-bit field */
        bf.field3 = (temp >> 12) & 0xFFF;     /* 12-bit field */
        
        /* 2. Mixed arithmetic and bitwise operations */
        temp = temp + (bf.field1 * 3);        /* Arithmetic creates SUBREG potential */
        
        /* 3. Complex memory addressing with pointer arithmetic */
        int idx = (temp ^ bf.field2) & 0xF;   /* Non-trivial index calculation */
        
        /* This creates MEM with complex addressing: [global_array + idx*4] */
        volatile int* ptr = &global_array[idx];
        
        /* 4. Bitwise assignment to memory location - may generate ZERO_EXTRACT */
        *ptr = (*ptr & 0xFFFF00FF) | ((temp & 0xFF) << 8);
        
        /* 5. Additional bit manipulation on the bitfield structure */
        /* Access through pointer with offset to create complex MEM address */
        volatile char* byte_ptr = (volatile char*)&bf;
        byte_ptr[1] = byte_ptr[1] ^ 0xAA;     /* XOR second byte */
        
        /* 6. Accumulate results to prevent dead code elimination */
        result += temp + *ptr + bf.field3 + idx;
        
        /* 7. Nested loop for additional complexity */
        for (j = 0; j < 4; j++) {
            /* Array access with computed index inside loop */
            int inner_idx = (idx + j) & 0xF;
            global_array[inner_idx] += (bf.field2 >> j) & 1;
        }
        
        /* 8. Update global_base to create data dependency across iterations */
        global_base = (global_base * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Final computation using all modified values */
    result += bf.field1 + bf.field2 + bf.field3;
    
    /* Print result to prevent optimization and verify execution */
    printf("Result: %d\n", result);
    
    /* Additional verification print */
    printf("Bitfield values: %u, %u, %u\n", 
           bf.field1, bf.field2, bf.field3);
    
    return 0;
}
