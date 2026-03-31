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
    volatile unsigned int field1 : 4;   /* Volatile to prevent optimization */
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
    
    /* Local array with volatile elements to create complex MEM addresses */
    volatile int local_array[32];
    for (int i = 0; i < 32; i++) {
        local_array[i] = i * 3;
    }
    
    /* Loop to create data dependencies and inhibit simple addressing */
    for (int i = 0; i < 100; i++) {
        /* 1. Arithmetic operations that may generate SUBREG in RTL */
        int base = i * 7 + global_counter;
        
        /* 2. Bit-field assignments (potential ZERO_EXTRACT/STRICT_LOW_PART) */
        bf.field1 = (base >> 0) & 0xF;   /* Direct bit-field assignment */
        bf.field2 = (base >> 4) & 0xFF;
        bf.field3 = (base >> 8) & 0xFFF;
        
        /* 3. Bitwise operations on the whole structure (more ZERO_EXTRACT) */
        *(volatile unsigned int*)&bf |= (1 << 3);  /* Set specific bit */
        *(volatile unsigned int*)&bf &= ~(1 << 7); /* Clear specific bit */
        
        /* 4. Complex memory addressing with pointer arithmetic */
        int idx = (i * 13 + bf.field1) % 32;
        
        /* Array access with computed index (complex MEM address) */
        temp = local_array[idx];
        
        /* Further pointer manipulation */
        volatile int *ptr = &local_array[0];
        ptr += (bf.field2 % 16);
        
        /* Memory store through manipulated pointer */
        *ptr = temp + bf.field3;
        
        /* 5. Mixed operations to create SUBREG patterns */
        long long big_val = (long long)temp * (long long)bf.field3;
        
        /* Extract parts of the big value (potential for SUBREG) */
        int lower_part = (int)(big_val & 0xFFFFFFFF);
        int upper_part = (int)((big_val >> 32) & 0xFFFFFFFF);
        
        /* More bit-field manipulation */
        bf.field4 = (lower_part ^ upper_part) & 0xFF;
        
        /* 6. Global array access with complex index */
        int global_idx = (bf.field4 + i) % 16;
        global_array[global_idx] += bf.field1;
        
        /* Accumulate results to prevent optimization */
        result += temp + bf.field2 + *ptr + global_array[global_idx];
        
        /* Update global counter to create loop-carried dependency */
        global_counter += (result & 1);
    }
    
    /* Additional volatile bit-field operations outside loop */
    struct bitfield_struct bf2 = {0};
    for (int i = 0; i < 10; i++) {
        /* Sequential bit-field assignments */
        bf2.field1 = i & 0xF;
        bf2.field2 = (i * 3) & 0xFF;
        bf2.field3 = (i * 5) & 0xFFF;
        bf2.field4 = (i * 7) & 0xFF;
        
        /* Mixed arithmetic and bitwise */
        int mixed = bf2.field1 + bf2.field2;
        bf2.field3 = mixed & 0xFFF;
        
        result += *(volatile unsigned int*)&bf2;
    }
    
    /* Print result to ensure all operations are observable */
    printf("Result: %d\n", result);
    printf("Global counter: %d\n", global_counter);
    
    /* Also print some array values to prevent dead store elimination */
    printf("Array samples: %d, %d, %d\n", 
           local_array[0], local_array[16], global_array[0]);
    
    return 0;
}
