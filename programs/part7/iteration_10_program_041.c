/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent optimization and ensure RTL generation */
volatile unsigned int global_val = 0xDEADBEEF;
volatile int global_idx = 0;
volatile short global_short;
volatile int global_int;

/* Arrays for memory access patterns */
volatile int mem_array[256];
volatile short short_array[256];

/* Function to generate ZERO_EXTRACT pattern */
unsigned int generate_zero_extract(volatile unsigned int val, 
                                   volatile unsigned int start, 
                                   volatile unsigned int width) {
    /* Non-constant shift and mask to prevent folding */
    unsigned int shifted = val >> start;
    unsigned int mask = (1U << width) - 1;
    return shifted & mask;
}

/* Function to generate STRICT_LOW_PART pattern */
void generate_strict_low_part(volatile int src) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    global_short = (short)src;
}

/* Function to generate SUBREG pattern */
int16_t generate_subreg(int32_t val) {
    /* Cast between different integer sizes generates SUBREG */
    return (int16_t)val;
}

/* Function to generate complex MEM address pattern */
int generate_complex_mem(volatile int *arr, volatile int idx) {
    /* Complex addressing: array base + variable index + offset */
    return arr[idx + 5];
}

/* Function combining all patterns in a hot loop */
unsigned int combine_patterns(volatile unsigned int seed) {
    unsigned int result = 0;
    
    /* Initialize arrays with pseudo-random but deterministic values */
    for (int i = 0; i < 256; i++) {
        mem_array[i] = (i * 37 + seed) & 0xFF;
        short_array[i] = (i * 73 + seed) & 0xFFFF;
    }
    
    /* Generate each target pattern in sequence */
    
    /* 1. ZERO_EXTRACT pattern - extract 8 bits starting at position 4 */
    volatile unsigned int extract_start = (seed % 16) + 1;
    volatile unsigned int extract_width = (seed % 8) + 1;
    unsigned int extracted = generate_zero_extract(global_val, 
                                                   extract_start, 
                                                   extract_width);
    result ^= extracted;
    
    /* 2. STRICT_LOW_PART pattern - assign int to short */
    volatile int src_val = (seed * 7919) & 0xFFFFFFFF;
    generate_strict_low_part(src_val);
    result ^= (unsigned int)global_short;
    
    /* 3. SUBREG pattern - cast int32 to int16 */
    int32_t subreg_src = (seed * 1337) & 0xFFFFFFFF;
    int16_t subreg_result = generate_subreg(subreg_src);
    result ^= (unsigned int)subreg_result;
    
    /* 4. Complex MEM address pattern - array with variable index */
    volatile int mem_idx = (seed % 200) + 10;  /* Ensure in bounds */
    int mem_result = generate_complex_mem(mem_array, mem_idx);
    result ^= (unsigned int)mem_result;
    
    /* Additional: MEM with even more complex addressing */
    int complex_idx = (seed * 17) % 200;
    /* MEM with register + scaled index + offset */
    int complex_mem_result = mem_array[complex_idx * 2 + 3];
    result ^= (unsigned int)complex_mem_result;
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Use command line argument for reproducible but variable input */
    unsigned int seed = 42;  /* Default seed */
    if (argc > 1) {
        seed = (unsigned int)atoi(argv[1]);
    }
    
    /* Initialize global variables with seed-dependent values */
    global_val = (seed * 0x1234567) ^ 0x89ABCDEF;
    global_idx = (seed % 100) + 1;
    
    /* Run the pattern generator multiple times to increase coverage chance */
    unsigned int final_result = 0;
    for (int i = 0; i < 10; i++) {
        unsigned int iteration_seed = seed + i * 10007;
        final_result += combine_patterns(iteration_seed);
    }
    
    /* Print result to ensure all computations are observable */
    printf("Result: 0x%08X\n", final_result);
    
    return 0;
}
