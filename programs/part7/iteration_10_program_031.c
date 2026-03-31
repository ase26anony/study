/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent optimization and force RTL generation */
volatile unsigned int global_val = 0x12345678;
volatile int global_idx = 5;
volatile short global_short;
volatile int global_int = 0x89ABCDEF;
volatile int arr[100];

/* Function to ensure operations aren't optimized away */
static int use_result(int val) {
    volatile int sink = val;
    return sink;
}

/* Generate ZERO_EXTRACT pattern */
static unsigned int gen_zero_extract(volatile unsigned int val, 
                                     volatile unsigned int start, 
                                     volatile unsigned int width) {
    /* Non-constant shift and mask to prevent folding */
    unsigned int shifted = val >> start;
    unsigned int mask = (1U << width) - 1;
    return shifted & mask;
}

/* Generate STRICT_LOW_PART pattern */
static void gen_strict_low_part(volatile short *dest, volatile int src) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    *dest = (short)src;
}

/* Generate SUBREG pattern */
static int16_t gen_subreg(int32_t val) {
    /* Cast between different sizes generates SUBREG */
    return (int16_t)val;
}

/* Generate complex MEM address pattern */
static int gen_complex_mem(volatile int *array, volatile int idx) {
    /* Complex addressing: array[idx + offset] */
    int offset = 3;
    return array[idx + offset];
}

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Initialize with pseudo-random but reproducible values */
    unsigned int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = rand() % 1000;
    }
    
    /* 1. Generate ZERO_EXTRACT RTL */
    volatile unsigned int extract_start = (seed % 16) + 1;
    volatile unsigned int extract_width = (seed % 8) + 1;
    unsigned int extract_result = gen_zero_extract(global_val, 
                                                   extract_start, 
                                                   extract_width);
    result ^= extract_result;
    
    /* 2. Generate STRICT_LOW_PART RTL */
    gen_strict_low_part(&global_short, global_int);
    result ^= global_short;
    
    /* 3. Generate SUBREG RTL */
    int32_t subreg_src = seed * 137;
    int16_t subreg_result = gen_subreg(subreg_src);
    result ^= subreg_result;
    
    /* 4. Generate complex MEM address RTL */
    volatile int mem_idx = (seed % 90) + 1;  /* Ensure in bounds */
    int mem_result = gen_complex_mem(arr, mem_idx);
    result ^= mem_result;
    
    /* Combine all results to ensure all operations are used */
    result = use_result(result);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d (seed: %u)\n", result, seed);
    
    return 0;
}
