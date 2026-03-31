/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile unsigned int global_val = 0xDEADBEEF;
volatile unsigned short global_short = 0;
volatile int global_array[256];
volatile int global_result = 0;

/* Force generation of ZERO_EXTRACT pattern */
unsigned int zero_extract_example(volatile unsigned int val, 
                                  volatile unsigned int start,
                                  volatile unsigned int width) {
    /* Non-constant shift and mask to prevent folding */
    unsigned int shifted = val >> start;
    unsigned int mask = (1U << width) - 1;
    return shifted & mask;
}

/* Force generation of STRICT_LOW_PART pattern */
void strict_low_part_example(volatile int src) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    volatile short dest;
    dest = (short)src;
    global_short = dest;
}

/* Force generation of SUBREG pattern */
int32_t subreg_example(volatile int32_t a) {
    /* Cast to smaller type generates SUBREG */
    int16_t b = (int16_t)a;
    /* Cast back to generate another SUBREG */
    return (int32_t)b;
}

/* Force generation of complex MEM address pattern */
int mem_complex_address(volatile int* arr, volatile int idx) {
    /* Complex addressing: array + variable index + offset */
    return arr[idx + 5];
}

/* Combined test function with all patterns */
unsigned int test_all_patterns(volatile unsigned int seed) {
    unsigned int result = 0;
    
    /* 1. ZERO_EXTRACT pattern */
    volatile unsigned int extract_start = (seed % 16) + 1;
    volatile unsigned int extract_width = (seed % 8) + 1;
    unsigned int extracted = zero_extract_example(global_val, 
                                                  extract_start, 
                                                  extract_width);
    result ^= extracted;
    
    /* 2. STRICT_LOW_PART pattern */
    volatile int src_val = (int)(seed * 12345);
    strict_low_part_example(src_val);
    result ^= (unsigned int)global_short;
    
    /* 3. SUBREG pattern */
    volatile int32_t subreg_input = (int32_t)(seed ^ 0xABCD1234);
    int32_t subreg_output = subreg_example(subreg_input);
    result ^= (unsigned int)subreg_output;
    
    /* 4. Complex MEM address pattern */
    /* Initialize array with pseudo-random values */
    for (int i = 0; i < 256; i++) {
        global_array[i] = (int)(seed + i * 7919);
    }
    
    volatile int idx = (seed % 200) + 10;
    int mem_value = mem_complex_address(global_array, idx);
    result ^= (unsigned int)mem_value;
    
    return result;
}

int main(int argc, char *argv[]) {
    unsigned int seed = 42; /* Default seed */
    
    /* Use command line argument for seed if provided */
    if (argc > 1) {
        seed = (unsigned int)atoi(argv[1]);
    }
    
    /* Run the test multiple times to increase coverage probability */
    unsigned int final_result = 0;
    for (int i = 0; i < 100; i++) {
        volatile unsigned int iter_seed = seed + i * 9973;
        final_result += test_all_patterns(iter_seed);
    }
    
    /* Use the result to prevent optimization */
    global_result = (int)final_result;
    
    printf("Result: %u\n", final_result);
    return (int)(final_result & 0x7FFFFFFF);
}
