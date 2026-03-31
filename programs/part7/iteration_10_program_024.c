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

/* Function to ensure operations aren't optimized away */
static int use_result(int val) {
    volatile int sink = val;
    return sink;
}

/* Generate ZERO_EXTRACT pattern */
static unsigned int gen_zero_extract(unsigned int val, int start, int width) {
    /* Non-constant shift and mask to prevent folding */
    volatile unsigned int v = val;
    volatile int s = start;
    volatile int w = width;
    
    /* This should generate ZERO_EXTRACT RTL */
    unsigned int result = (v >> s) & ((1U << w) - 1);
    
    /* Add dependency to prevent dead code elimination */
    global_val = result;
    return result;
}

/* Generate STRICT_LOW_PART pattern */
static short gen_strict_low_part(int src) {
    volatile int v = src;
    
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    short result = (short)v;
    
    /* Store to volatile to ensure the operation happens */
    global_short = result;
    return result;
}

/* Generate SUBREG pattern */
static int16_t gen_subreg(int32_t val) {
    volatile int32_t v = val;
    
    /* Cast to smaller type generates SUBREG */
    int16_t result = (int16_t)v;
    
    /* Use result to prevent elimination */
    global_int = (int)result;
    return result;
}

/* Generate complex MEM address pattern */
static int gen_complex_mem(int idx) {
    volatile int index = idx;
    
    /* Complex addressing: array base + variable index + offset */
    /* Should generate MEM with non-simple address expression */
    int result = global_array[index + 5];
    
    /* Also try pointer arithmetic version */
    int result2 = *(global_ptr + index * 2);
    
    return result + result2;
}

/* Combined test function with all patterns */
static int test_all_patterns(int seed) {
    int result = 0;
    
    /* 1. ZERO_EXTRACT pattern */
    /* Use seed-dependent parameters to prevent constant folding */
    int start = (seed % 8) + 1;
    int width = (seed % 16) + 1;
    unsigned int extract_result = gen_zero_extract(0xABCDEF12, start, width);
    result ^= extract_result;
    
    /* 2. STRICT_LOW_PART pattern */
    short low_part = gen_strict_low_part(seed * 0x1234);
    result += (int)low_part;
    
    /* 3. SUBREG pattern */
    int16_t subreg_val = gen_subreg(seed * 0x5678);
    result += (int)subreg_val;
    
    /* 4. Complex MEM pattern */
    int mem_result = gen_complex_mem((seed % 100) + 10);
    result += mem_result;
    
    /* Additional: Combined operation that might generate multiple patterns */
    volatile struct {
        unsigned int full;
        unsigned short half;
    } combined;
    
    combined.full = seed * 0x9ABCDEF;
    combined.half = (unsigned short)(combined.full >> 8);
    result += combined.half;
    
    /* Array with complex indexing - another MEM pattern */
    volatile int arr2d[10][10];
    for (int i = 0; i < 10; i++) {
        arr2d[i][seed % 10] = i * seed;
        result += arr2d[i][(seed + i) % 10];
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    /* Use command line argument for variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize array with pseudo-random values */
    for (int i = 0; i < 256; i++) {
        global_array[i] = (i * seed) ^ 0xDEADBEEF;
    }
    
    /* Run the test multiple times to increase coverage probability */
    int final_result = 0;
    for (int i = 0; i < 10; i++) {
        final_result += test_all_patterns(seed + i);
        final_result = use_result(final_result);
    }
    
    /* Print result to ensure all computations are observable */
    printf("Result: %d\n", final_result);
    
    return 0;
}
