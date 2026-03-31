/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile unsigned int global_val = 0x12345678;
volatile short global_short;
volatile int global_int;
volatile int global_array[256];
volatile int *global_ptr = &global_array[0];

/* Function to generate ZERO_EXTRACT pattern */
unsigned int gen_zero_extract(volatile unsigned int val, int start, int width) {
    /* Non-constant shift and mask to prevent folding */
    unsigned int shifted = val >> start;
    unsigned int mask = (1U << width) - 1;
    return shifted & mask;
}

/* Function to generate STRICT_LOW_PART pattern */
void gen_strict_low_part(volatile short *dest, volatile int src) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    *dest = (short)src;
}

/* Function to generate SUBREG pattern */
int16_t gen_subreg(int32_t a) {
    /* Cast between different-sized integers generates SUBREG */
    return (int16_t)a;
}

/* Function to generate complex MEM address pattern */
int gen_complex_mem(volatile int *arr, int idx, int offset) {
    /* Variable index with offset creates complex addressing */
    return arr[idx + offset];
}

/* Main function combining all patterns */
int main(int argc, char *argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize with pseudo-random but reproducible values */
    srand(seed);
    
    /* 1. Generate ZERO_EXTRACT pattern */
    int start = (rand() % 16) + 1;  /* Non-constant between 1-16 */
    int width = (rand() % 8) + 1;   /* Non-constant between 1-8 */
    unsigned int extract_result = gen_zero_extract(global_val, start, width);
    
    /* 2. Generate STRICT_LOW_PART pattern */
    gen_strict_low_part(&global_short, global_val);
    
    /* 3. Generate SUBREG pattern */
    int32_t src_val = (int32_t)global_val;
    int16_t subreg_result = gen_subreg(src_val);
    
    /* 4. Generate complex MEM address pattern */
    /* Initialize array with some values */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + seed;
    }
    
    int idx = rand() % 200;  /* Non-constant index */
    int offset = 5;          /* Constant offset */
    int mem_result = gen_complex_mem(global_array, idx, offset);
    
    /* 5. Combine all results to ensure all computations are used */
    int final_result = (int)extract_result + 
                      (int)global_short + 
                      (int)subreg_result + 
                      mem_result;
    
    /* Print result to create observable side effect */
    printf("Result: %d (seed: %d)\n", final_result, seed);
    printf("  extract: %u, short: %d, subreg: %d, mem: %d\n",
           extract_result, (int)global_short, (int)subreg_result, mem_result);
    
    return final_result != 0 ? 0 : 1;
}
