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
    return (val >> start) & ((1U << width) - 1);
}

/* Function to generate STRICT_LOW_PART pattern */
void gen_strict_low_part(volatile int src) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    volatile short dest;
    dest = (short)src;
    global_short = dest;
}

/* Function to generate SUBREG pattern */
int16_t gen_subreg(int32_t val) {
    /* Cast between different integer sizes generates SUBREG */
    return (int16_t)val;
}

/* Function to generate complex MEM address pattern */
int gen_complex_mem(volatile int *arr, int idx) {
    /* Variable index with offset creates complex addressing */
    return arr[idx + 5];
}

/* Main function combining all patterns */
int main(int argc, char *argv[]) {
    int seed = 42;
    unsigned int result = 0;
    
    /* Use command-line argument for variability if provided */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize with pseudo-random but reproducible values */
    srand(seed);
    
    /* 1. Generate ZERO_EXTRACT pattern */
    int start = (rand() % 16) + 1;  /* Non-constant start position */
    int width = (rand() % 8) + 1;   /* Non-constant width */
    
    /* Use volatile to force actual computation */
    volatile unsigned int extract_result = gen_zero_extract(global_val, start, width);
    result ^= extract_result;
    
    /* 2. Generate STRICT_LOW_PART pattern */
    volatile int src_val = rand();
    gen_strict_low_part(src_val);
    result ^= (unsigned int)global_short;
    
    /* 3. Generate SUBREG pattern */
    int32_t subreg_src = rand();
    int16_t subreg_result = gen_subreg(subreg_src);
    result ^= (unsigned int)subreg_result;
    
    /* 4. Generate complex MEM address pattern */
    /* Initialize array with values */
    for (int i = 0; i < 256; i++) {
        global_array[i] = rand();
    }
    
    int idx = rand() % 200;  /* Variable index */
    int mem_result = gen_complex_mem(global_array, idx);
    result ^= (unsigned int)mem_result;
    
    /* 5. Additional patterns in same basic block */
    /* Combined operation that might generate multiple patterns */
    volatile struct {
        unsigned int field1 : 8;
        unsigned int field2 : 8;
        unsigned int field3 : 8;
        unsigned int field4 : 8;
    } bitfield;
    
    bitfield.field1 = (global_val >> 0) & 0xFF;
    bitfield.field2 = (global_val >> 8) & 0xFF;
    result ^= bitfield.field1 + bitfield.field2;
    
    /* Pointer arithmetic for complex MEM addresses */
    volatile int *ptr = global_ptr + idx;
    result ^= *ptr;
    
    /* Mixed-size operations */
    volatile int64_t large_val = (int64_t)global_val * (int64_t)seed;
    volatile int32_t truncated = (int32_t)large_val;
    result ^= truncated;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %u (seed: %d)\n", result, seed);
    
    return (int)(result & 0x7FFFFFFF);
}
