/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent optimization and ensure RTL generation */
volatile unsigned int global_val = 0x12345678;
volatile int global_idx = 5;
volatile unsigned int global_start = 4;
volatile unsigned int global_width = 8;

/* Array for complex memory addressing */
volatile int mem_array[256];

/* Function to force generation of ZERO_EXTRACT pattern */
unsigned int zero_extract_pattern(volatile unsigned int val, 
                                  volatile unsigned int start, 
                                  volatile unsigned int width) {
    /* Non-constant shift and mask to prevent folding */
    /* This should generate ZERO_EXTRACT RTL on architectures like ARM */
    unsigned int shifted = val >> start;
    unsigned int mask = (1U << width) - 1;
    return shifted & mask;
}

/* Function to force STRICT_LOW_PART pattern */
void strict_low_part_pattern(volatile int32_t src) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    volatile int16_t dest;
    dest = (int16_t)src;
    
    /* Use the result to prevent dead code elimination */
    mem_array[0] = (int)dest;
}

/* Function to force SUBREG pattern */
int32_t subreg_pattern(volatile int32_t a) {
    /* Cast between different integer sizes generates SUBREG */
    int16_t b = (int16_t)a;
    int32_t c = (int32_t)b;  /* Sign extension may also be interesting */
    return c;
}

/* Function with complex memory addressing (MEM_P with non-simple address) */
int complex_mem_access(volatile int idx) {
    /* Variable index with offset creates complex addressing mode */
    int value = mem_array[idx + 10];
    
    /* More complex addressing: array[array[idx]] */
    int idx2 = mem_array[idx];
    return mem_array[idx2 * 2];
}

/* Combined function that uses all patterns */
unsigned int combined_patterns(volatile unsigned int seed) {
    unsigned int result = 0;
    
    /* 1. ZERO_EXTRACT pattern */
    result ^= zero_extract_pattern(global_val ^ seed, 
                                   global_start + (seed & 0x3), 
                                   global_width + (seed & 0x7));
    
    /* 2. STRICT_LOW_PART pattern */
    strict_low_part_pattern((int32_t)(result + seed));
    
    /* 3. SUBREG pattern */
    result += subreg_pattern((int32_t)result);
    
    /* 4. Complex memory access pattern */
    result += complex_mem_access(global_idx + (seed & 0xFF));
    
    return result;
}

int main(int argc, char *argv[]) {
    unsigned int seed = 0;
    
    /* Use command line argument for reproducibility but variability */
    if (argc > 1) {
        seed = (unsigned int)atoi(argv[1]);
    } else {
        seed = 12345;
    }
    
    /* Initialize array with pseudo-random but deterministic values */
    for (int i = 0; i < 256; i++) {
        mem_array[i] = (i * 37 + seed) & 0xFF;
    }
    
    /* Execute the combined patterns multiple times */
    unsigned int final_result = 0;
    for (int i = 0; i < 100; i++) {
        final_result ^= combined_patterns(seed + i);
    }
    
    /* Print result to ensure all computations are observable */
    printf("Result: 0x%08x\n", final_result);
    
    return 0;
}
