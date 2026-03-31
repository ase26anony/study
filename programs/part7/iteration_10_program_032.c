/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile unsigned int global_val = 0xDEADBEEF;
volatile int global_idx = 0;
volatile short global_short;
volatile int global_int;
volatile int global_array[256];

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
    unsigned int shift = start % 32;
    unsigned int mask_width = width % 16 + 1;  /* Ensure width 1-16 */
    unsigned int mask = (1U << mask_width) - 1;
    
    /* This should generate ZERO_EXTRACT RTL */
    unsigned int result = (val >> shift) & mask;
    
    /* Use volatile to force generation */
    volatile unsigned int vol_result = result;
    return vol_result;
}

/* Generate STRICT_LOW_PART pattern */
static short gen_strict_low_part(volatile int src) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    volatile short dest;
    dest = (short)src;  /* Potential STRICT_LOW_PART */
    return dest;
}

/* Generate SUBREG pattern */
static int16_t gen_subreg(int32_t val) {
    /* Cast between different sizes generates SUBREG */
    int16_t result = (int16_t)val;
    volatile int16_t vol_result = result;
    return vol_result;
}

/* Generate complex MEM address pattern */
static int gen_complex_mem(volatile int *arr, int idx) {
    /* Complex addressing: arr[idx + offset] */
    int offset = 7;
    volatile int result;
    
    /* Variable index with offset creates non-simple MEM address */
    result = arr[idx + offset];  /* MEM with complex address */
    
    return result;
}

/* Combined test function */
static int test_resources_combined(int seed) {
    int result = 0;
    
    /* Initialize with seed-dependent values */
    volatile unsigned int val = (unsigned int)seed ^ 0x12345678;
    volatile int idx = (seed * 1103515245 + 12345) & 255;
    
    /* 1. ZERO_EXTRACT pattern */
    unsigned int extract_start = (seed >> 3) & 31;
    unsigned int extract_width = (seed >> 8) & 15;
    if (extract_width == 0) extract_width = 8;
    
    result ^= gen_zero_extract(val, extract_start, extract_width);
    
    /* 2. STRICT_LOW_PART pattern */
    volatile int src_int = seed * 0xABCDEF;
    result += gen_strict_low_part(src_int);
    
    /* 3. SUBREG pattern */
    int32_t subreg_val = seed * 0x87654321;
    result ^= gen_subreg(subreg_val);
    
    /* 4. Complex MEM address pattern */
    /* Initialize array with pattern */
    for (int i = 0; i < 256; i++) {
        global_array[i] = (i * seed) ^ 0xF0F0F0F0;
    }
    
    result += gen_complex_mem(global_array, idx);
    
    /* 5. Additional patterns in same basic block */
    /* Mixed operations to keep scheduler busy */
    volatile int mixed = 0;
    mixed = global_val & 0xFFFF;          /* Potential SUBREG */
    mixed = (short)mixed;                 /* Potential STRICT_LOW_PART */
    result ^= mixed;
    
    /* Another memory access with different addressing */
    volatile int *ptr = &global_array[0];
    ptr += (seed & 31);
    result += *ptr;                       /* MEM with register address */
    
    return use_result(result);
}

/* Main function with command-line control */
int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Run multiple iterations to increase coverage chance */
    int final_result = 0;
    for (int i = 0; i < 10; i++) {
        final_result ^= test_resources_combined(seed + i);
    }
    
    printf("Result: %d (0x%08x)\n", final_result, final_result);
    
    return 0;
}
