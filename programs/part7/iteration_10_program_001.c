/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile unsigned int global_val = 0xDEADBEEF;
volatile short global_short;
volatile int global_int;
volatile int global_array[256];
volatile int *global_ptr = &global_array[0];

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
    volatile short dest;
    dest = (short)src;
    global_short = dest;
}

/* Generate SUBREG pattern */
static int16_t gen_subreg(int32_t val) {
    /* Cast between different-sized types generates SUBREG */
    int16_t result = (int16_t)val;
    return result;
}

/* Generate complex MEM address pattern */
static int gen_complex_mem(volatile int *arr, int idx) {
    /* Variable index with offset creates complex addressing */
    return arr[idx + 3];
}

int main(int argc, char *argv[]) {
    unsigned int result = 0;
    unsigned int seed = 12345;
    
    /* Use command line argument for variability if provided */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize with pseudo-random but reproducible values */
    srand(seed);
    
    /* Initialize array with non-zero values */
    for (int i = 0; i < 256; i++) {
        global_array[i] = rand() % 1000;
    }
    
    /* 1. Generate ZERO_EXTRACT pattern */
    /* Use variable shift and width to prevent constant folding */
    unsigned int shift = (seed % 16) + 1;
    unsigned int width = (seed % 8) + 1;
    unsigned int extract_result = gen_zero_extract(global_val, shift, width);
    result ^= extract_result;
    
    /* 2. Generate STRICT_LOW_PART pattern */
    gen_strict_low_part(seed * 37);
    result ^= global_short;
    
    /* 3. Generate SUBREG pattern */
    int32_t subreg_src = seed * 73;
    int16_t subreg_result = gen_subreg(subreg_src);
    result ^= subreg_result;
    
    /* 4. Generate complex MEM address pattern */
    int mem_idx = (seed * 97) % 200;  /* Ensure within bounds */
    int mem_result = gen_complex_mem(global_array, mem_idx);
    result ^= mem_result;
    
    /* Additional combinations to increase coverage probability */
    
    /* Nested operations that might generate multiple patterns */
    volatile struct {
        unsigned int field1 : 8;
        unsigned int field2 : 16;
        unsigned int field3 : 8;
    } bitfield = {0};
    
    /* Bitfield operations may generate ZERO_EXTRACT/STRICT_LOW_PART */
    bitfield.field1 = (global_val >> 8) & 0xFF;
    bitfield.field2 = global_val & 0xFFFF;
    result ^= bitfield.field1;
    result ^= bitfield.field2;
    
    /* Pointer arithmetic with different types */
    volatile char *char_ptr = (volatile char *)global_array;
    int char_offset = seed % 100;
    volatile char char_val = char_ptr[char_offset * sizeof(int)];
    result ^= char_val;
    
    /* Mixed-size operations */
    volatile int64_t large_val = (int64_t)global_val * seed;
    volatile int32_t truncated = (int32_t)large_val;
    result ^= truncated;
    
    /* Ensure all operations contribute to output */
    result = use_result(result);
    
    printf("Result: %u (seed: %u)\n", result, seed);
    
    return 0;
}
