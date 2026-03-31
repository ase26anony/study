/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Use volatile to prevent optimization and ensure RTL generation */
volatile unsigned int global_val = 0xDEADBEEF;
volatile int global_idx = 5;
volatile unsigned short global_short;
volatile int global_arr[100];

/* Function to generate ZERO_EXTRACT pattern */
unsigned int zero_extract_example(volatile unsigned int val, int start, int width) {
    /* Non-constant shift and mask to prevent folding */
    unsigned int shifted = val >> start;
    unsigned int mask = (1U << width) - 1;
    return shifted & mask;
}

/* Function to generate STRICT_LOW_PART pattern */
void strict_low_part_example(volatile int src) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    volatile short dest;
    dest = (short)src;
    global_short = dest;  /* Ensure side effect */
}

/* Function to generate SUBREG pattern */
int32_t subreg_example(int32_t a) {
    /* Cast between different integer sizes */
    int16_t b = (int16_t)a;
    int32_t c = (int32_t)b;  /* Another cast back */
    return c;
}

/* Function to generate complex MEM address pattern */
int mem_complex_address(volatile int *arr, int idx) {
    /* Complex addressing: arr[idx + offset] */
    int offset = 3;
    return arr[idx + offset];
}

/* Main function combining all patterns */
int main(int argc, char *argv[]) {
    int seed = 0;
    unsigned int result = 0;
    
    /* Use command line argument for seed to prevent constant propagation */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = 12345;
    }
    
    /* Initialize array with pseudo-random but reproducible values */
    srand(seed);
    for (int i = 0; i < 100; i++) {
        global_arr[i] = rand();
    }
    
    /* 1. Generate ZERO_EXTRACT pattern */
    /* Use non-constant parameters to prevent optimization */
    int start = (seed % 16) + 1;      /* 1-16 */
    int width = (seed % 8) + 1;       /* 1-8 */
    unsigned int extract_result = zero_extract_example(global_val, start, width);
    result ^= extract_result;
    
    /* 2. Generate STRICT_LOW_PART pattern */
    strict_low_part_example(seed * 1000);
    result ^= global_short;
    
    /* 3. Generate SUBREG pattern */
    int32_t subreg_input = seed * 0xABCD;
    int32_t subreg_output = subreg_example(subreg_input);
    result ^= subreg_output;
    
    /* 4. Generate complex MEM address pattern */
    /* Use variable index to prevent simple addressing */
    int idx = (seed % 90) + 5;  /* 5-94 to stay within bounds */
    int mem_result = mem_complex_address(global_arr, idx);
    result ^= mem_result;
    
    /* Additional patterns in a loop to increase coverage probability */
    for (int i = 0; i < 3; i++) {
        /* Mixed operations in same basic block */
        volatile int temp = global_arr[global_idx + i];
        volatile short temp_short = (short)temp;
        result += temp_short;
        
        /* Another ZERO_EXTRACT with different parameters */
        unsigned int temp_extract = zero_extract_example(temp, i + 1, 4);
        result += temp_extract;
    }
    
    /* Print result to ensure all computations are observable */
    printf("Result: %u (0x%08X)\n", result, result);
    
    return 0;
}
