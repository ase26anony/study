/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent optimization and ensure RTL generation */
volatile unsigned int global_val = 0xDEADBEEF;
volatile int global_idx = 0;
volatile int global_start = 4;
volatile int global_width = 8;

/* Arrays for memory access patterns */
volatile int mem_array[256];
volatile short short_array[256];

/* Function to force generation of ZERO_EXTRACT pattern */
unsigned int zero_extract_operation(volatile unsigned int val, 
                                   volatile int start, 
                                   volatile int width) {
    /* Non-constant shift and mask to prevent folding */
    unsigned int shifted = val >> start;
    unsigned int mask = ((1U << width) - 1);
    return shifted & mask;
}

/* Function to force generation of STRICT_LOW_PART pattern */
void strict_low_part_operation(volatile int src, volatile short *dest) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    *dest = (short)src;
}

/* Function to force generation of SUBREG pattern */
int32_t subreg_operation(volatile int32_t a) {
    /* Cast to smaller type generates SUBREG */
    int16_t b = (int16_t)a;
    /* Cast back with sign extension */
    return (int32_t)b;
}

/* Function with complex memory addressing */
int complex_mem_access(volatile int *arr, volatile int idx) {
    /* Complex addressing: arr[idx + offset] */
    int offset = 7;
    return arr[idx + offset];
}

/* Main function combining all patterns */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Initialize with pseudo-random but reproducible values */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        mem_array[i] = rand() % 1000;
        short_array[i] = (short)(rand() % 1000);
    }
    
    /* 1. Generate ZERO_EXTRACT pattern */
    volatile unsigned int extract_src = 0xABCD1234;
    volatile int start_pos = (rand() % 16) + 1;  /* Non-constant */
    volatile int field_width = (rand() % 8) + 1; /* Non-constant */
    
    unsigned int extract_result = zero_extract_operation(extract_src, 
                                                        start_pos, 
                                                        field_width);
    result ^= extract_result;
    
    /* 2. Generate STRICT_LOW_PART pattern */
    volatile int src_val = rand();
    volatile short dest_val;
    
    strict_low_part_operation(src_val, &dest_val);
    result ^= dest_val;
    
    /* 3. Generate SUBREG pattern */
    volatile int32_t subreg_src = rand();
    int32_t subreg_result = subreg_operation(subreg_src);
    result ^= subreg_result;
    
    /* 4. Generate complex MEM access pattern */
    volatile int mem_idx = rand() % 200;  /* Non-constant index */
    int mem_result = complex_mem_access((int*)mem_array, mem_idx);
    result ^= mem_result;
    
    /* Additional patterns in a loop to increase coverage probability */
    for (int i = 0; i < 10; i++) {
        /* Mixed operations in same basic block */
        volatile int temp = rand();
        
        /* Another ZERO_EXTRACT with different parameters */
        unsigned int ext = zero_extract_operation(temp, i + 1, (i % 7) + 1);
        result += ext;
        
        /* Another STRICT_LOW_PART */
        volatile short s;
        strict_low_part_operation(temp, &s);
        result += s;
        
        /* Another complex memory access */
        int mem_val = complex_mem_access((int*)mem_array, (mem_idx + i) % 200);
        result += mem_val;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
