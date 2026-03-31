/* test_resources.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent optimization and ensure RTL generation */
volatile unsigned int global_val = 0x12345678;
volatile int global_idx = 5;
volatile short global_short;
volatile int global_src = 0x89ABCDEF;
volatile int arr[100];

/* Function to force generation of ZERO_EXTRACT RTL */
unsigned int zero_extract_example(volatile unsigned int val, int start, int width) {
    /* Non-constant shift and mask to prevent folding */
    unsigned int shifted = val >> start;
    unsigned int mask = (1U << width) - 1;
    return shifted & mask;
}

/* Function to force generation of STRICT_LOW_PART RTL */
void strict_low_part_example(volatile int src) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    volatile short dest;
    dest = (short)src;
    global_short = dest;
}

/* Function to force generation of SUBREG RTL */
int32_t subreg_example(int32_t a) {
    /* Cast between different integer sizes */
    int16_t b = (int16_t)a;
    return (int32_t)b;  /* Sign-extend back */
}

/* Function with complex MEM addressing */
int mem_complex_address(volatile int *array, int idx) {
    /* Variable index with offset creates complex addressing */
    return array[idx + global_idx];
}

/* Combined test function with all patterns */
unsigned int test_all_patterns(int seed) {
    unsigned int result = 0;
    
    /* Initialize array with pseudo-random but reproducible values */
    for (int i = 0; i < 100; i++) {
        arr[i] = (i * 37 + seed) & 0xFF;
    }
    
    /* 1. ZERO_EXTRACT pattern */
    /* Use non-constant parameters to prevent optimization */
    int start = (seed % 16) + 1;
    int width = (seed % 8) + 1;
    unsigned int extract_result = zero_extract_example(global_val, start, width);
    result ^= extract_result;
    
    /* 2. STRICT_LOW_PART pattern */
    strict_low_part_example(global_src + seed);
    result ^= (unsigned int)global_short;
    
    /* 3. SUBREG pattern */
    int32_t subreg_input = global_val ^ seed;
    int32_t subreg_result = subreg_example(subreg_input);
    result ^= (unsigned int)subreg_result;
    
    /* 4. Complex MEM addressing pattern */
    int mem_idx = (seed * 7) % 50;
    int mem_result = mem_complex_address(arr, mem_idx);
    result ^= (unsigned int)mem_result;
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Use command line argument for reproducibility with variation */
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Run the test multiple times to increase coverage probability */
    unsigned int final_result = 0;
    for (int i = 0; i < 10; i++) {
        final_result += test_all_patterns(seed + i);
    }
    
    printf("Result: 0x%08X\n", final_result);
    return 0;
}
