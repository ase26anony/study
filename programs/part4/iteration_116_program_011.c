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

/* Packed structure with bit-fields to encourage ZERO_EXTRACT/STRICT_LOW_PART */
struct __attribute__((packed)) bitfield_struct {
    volatile unsigned int field1 : 4;   /* volatile to prevent optimization */
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 12;
    volatile unsigned int field4 : 8;
};

/* Global variables to force memory operations */
static volatile int global_counter = 0;
static volatile int global_array[16] = {0};

int main(void) {
    struct bitfield_struct bf = {0};
    volatile int result = 0;
    volatile int temp;
    int i, j;
    
    /* Array with pointer arithmetic to create complex MEM addresses */
    volatile int mem_array[32];
    volatile int *ptr;
    
    /* Initialize arrays */
    for (i = 0; i < 32; i++) {
        mem_array[i] = i * 3;
    }
    
    for (i = 0; i < 16; i++) {
        global_array[i] = i * 2;
    }
    
    /* Main loop to generate complex RTL patterns */
    for (i = 0; i < 100; i++) {
        /* 1. Arithmetic operation that may generate SUBREG in RTL */
        temp = global_counter + i * 7;
        
        /* 2. Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
        /* These assignments to specific bit ranges should generate ZERO_EXTRACT */
        bf.field1 = (temp & 0xF);                /* First 4 bits */
        bf.field2 = ((temp >> 4) & 0xFF);        /* Next 8 bits */
        bf.field3 = ((temp >> 12) & 0xFFF);      /* Next 12 bits */
        
        /* 3. Complex memory addressing with pointer arithmetic */
        /* This creates MEM with non-trivial address expressions */
        ptr = &mem_array[(i * 3) % 32];
        
        /* Combine with bitwise operations on the dereferenced value */
        *ptr = (*ptr & ~0xFF) | (temp & 0xFF);
        
        /* 4. Array access with computed index (more complex addressing) */
        j = (i * 5 + 3) % 16;
        global_array[j] = global_array[j] ^ (temp << (i % 8));
        
        /* 5. Additional bit manipulation on the bitfield structure */
        /* Cast to volatile int* and use bitwise operations */
        *(volatile unsigned int*)&bf = 
            (*(volatile unsigned int*)&bf & 0xFFFF0000) | 
            (temp & 0x0000FFFF);
        
        /* 6. Accumulate results to prevent optimization */
        result += bf.field1 + bf.field2 + bf.field3 + *ptr + global_array[j];
        
        /* Update global counter with mixed operations */
        global_counter = (global_counter * 13 + i) & 0x3FFFFFFF;
    }
    
    /* Additional loop with nested bit operations */
    for (i = 0; i < 50; i++) {
        /* Create data dependencies that inhibit simple addressing */
        volatile int idx1 = (result + i) % 32;
        volatile int idx2 = (result * 3 + i * 7) % 32;
        
        /* Memory operations with complex addressing modes */
        mem_array[idx1] = mem_array[idx2] | (1 << (i % 16));
        
        /* More bit-field manipulation */
        bf.field4 = (mem_array[idx1] >> 4) & 0xFF;
        
        /* Update result with all operations */
        result ^= mem_array[idx1] + bf.field4 + idx1 + idx2;
    }
    
    /* Final output to ensure all operations are observable */
    printf("Result: %d\n", result);
    printf("Global counter: %d\n", global_counter);
    printf("Bitfield values: %u, %u, %u, %u\n", 
           bf.field1, bf.field2, bf.field3, bf.field4);
    
    /* Sample array output to verify memory operations */
    printf("Array samples: %d, %d, %d\n", 
           mem_array[0], mem_array[10], mem_array[20]);
    
    return 0;
}
