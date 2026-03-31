/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile unsigned int global_val = 0xDEADBEEF;
volatile short global_short;
volatile int global_int;
volatile int global_array[256];

/* Function to ensure operations aren't optimized away */
static int use_result(int val) {
    volatile int sink = val;
    return sink;
}

/* Generate ZERO_EXTRACT pattern */
static unsigned int gen_zero_extract(volatile unsigned int val, 
                                     unsigned int start, 
                                     unsigned int width) {
    /* Non-constant shift and mask to prevent folding */
    unsigned int mask = (1U << width) - 1;
    return (val >> start) & mask;
}

/* Generate STRICT_LOW_PART pattern */
static void gen_strict_low_part(volatile int src) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    global_short = (short)src;
}

/* Generate SUBREG pattern */
static int16_t gen_subreg(int32_t val) {
    /* Cast between different-sized types generates SUBREG */
    return (int16_t)val;
}

/* Generate complex MEM address pattern */
static int gen_complex_mem(volatile int *arr, int idx) {
    /* Variable index with offset creates complex addressing */
    return arr[idx + 5];
}

/* Main test function */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use command-line argument for variability */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Initialize with pseudo-random but reproducible values */
    srand(seed);
    
    /* Initialize array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = rand();
    }
    
    /* 1. Generate ZERO_EXTRACT pattern */
    /* Use variable width and shift to prevent constant folding */
    unsigned int width = (seed % 16) + 1;  /* 1-16 bits */
    unsigned int shift = (seed % 16);      /* 0-15 bits */
    unsigned int extract_result = gen_zero_extract(global_val, shift, width);
    result ^= extract_result;
    
    /* 2. Generate STRICT_LOW_PART pattern */
    gen_strict_low_part(seed * 1000);
    result ^= global_short;
    
    /* 3. Generate SUBREG pattern */
    int32_t subreg_src = seed * 12345;
    int16_t subreg_result = gen_subreg(subreg_src);
    result ^= subreg_result;
    
    /* 4. Generate complex MEM address pattern */
    int mem_idx = (seed * 3) % 200;  /* Ensure within bounds */
    int mem_result = gen_complex_mem(global_array, mem_idx);
    result ^= mem_result;
    
    /* Combine all results with data dependencies */
    result = use_result(result);
    
    /* Additional patterns in a loop to increase coverage probability */
    for (int i = 0; i < 10; i++) {
        /* Mixed operations in same basic block */
        volatile int temp = global_array[i];
        
        /* Another ZERO_EXTRACT with different parameters */
        unsigned int ext = gen_zero_extract(temp, i % 8, (i % 7) + 1);
        result += ext;
        
        /* Another STRICT_LOW_PART */
        global_short = (short)(temp >> 8);
        result += global_short;
        
        /* Another complex MEM access */
        int idx = (i * 7) % 100;
        result += global_array[idx + i];
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
