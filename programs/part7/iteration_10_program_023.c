/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile unsigned int global_val = 0xDEADBEEF;
volatile unsigned short global_short = 0;
volatile int global_array[256];
volatile int global_result = 0;

/* Function to generate ZERO_EXTRACT pattern */
unsigned int generate_zero_extract(volatile unsigned int val, int start, int width) {
    /* Non-constant shift and mask to prevent folding */
    unsigned int shifted = val >> start;
    unsigned int mask = (1U << width) - 1;
    return shifted & mask;
}

/* Function to generate STRICT_LOW_PART pattern */
void generate_strict_low_part(volatile unsigned int src) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    volatile unsigned short dest;
    dest = (unsigned short)src;
    global_short = dest;
}

/* Function to generate SUBREG pattern */
int16_t generate_subreg(int32_t val) {
    /* Cast between different integer sizes generates SUBREG */
    int16_t result = (int16_t)val;
    return result;
}

/* Function to generate complex MEM address pattern */
int generate_complex_mem(volatile int *arr, int idx) {
    /* Variable index with offset creates complex addressing */
    int offset = 7;
    return arr[idx + offset];
}

int main(int argc, char *argv[]) {
    int i, seed = 0;
    
    /* Use command line argument for seed to create variable but reproducible data */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = 42; /* Default seed */
    }
    
    /* Initialize array with pseudo-random values */
    srand(seed);
    for (i = 0; i < 256; i++) {
        global_array[i] = rand();
    }
    
    /* Generate ZERO_EXTRACT pattern */
    /* Use non-constant parameters to prevent optimization */
    int start = (seed % 16) + 1;
    int width = (seed % 8) + 1;
    unsigned int extract_result = generate_zero_extract(global_val, start, width);
    
    /* Generate STRICT_LOW_PART pattern */
    generate_strict_low_part(global_val + seed);
    
    /* Generate SUBREG pattern */
    int32_t subreg_src = global_val ^ seed;
    int16_t subreg_result = generate_subreg(subreg_src);
    
    /* Generate complex MEM address pattern */
    int idx = (seed * 3) % 200; /* Ensure within bounds with offset */
    int mem_result = generate_complex_mem(global_array, idx);
    
    /* Combine all results to ensure all computations are used */
    global_result = extract_result + global_short + subreg_result + mem_result;
    
    /* Print result to create observable side effect */
    printf("Result: %d (seed: %d)\n", global_result, seed);
    
    return 0;
}
