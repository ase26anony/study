/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile unsigned int global_val = 0x12345678;
volatile short global_short;
volatile int global_int;
volatile int global_array[256];

/* Force generation of ZERO_EXTRACT RTL pattern */
unsigned int zero_extract_example(volatile unsigned int val, 
                                  volatile unsigned int start, 
                                  volatile unsigned int width) {
    /* Non-constant shift and mask to prevent folding */
    unsigned int shifted = val >> start;
    unsigned int mask = (1U << width) - 1;
    return shifted & mask;
}

/* Force generation of STRICT_LOW_PART RTL pattern */
void strict_low_part_example(volatile int src) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    global_short = (short)src;
}

/* Force generation of SUBREG RTL pattern */
int32_t subreg_example(volatile int32_t a) {
    /* Cast to smaller type generates SUBREG */
    int16_t b = (int16_t)a;
    /* Cast back to generate another SUBREG */
    return (int32_t)b;
}

/* Force generation of complex MEM address RTL pattern */
int mem_complex_address_example(volatile int *arr, 
                                volatile int idx, 
                                volatile int offset) {
    /* Complex addressing: arr[idx + offset] */
    return arr[idx + offset];
}

/* Combined test that uses all patterns */
unsigned int combined_test(volatile unsigned int seed) {
    unsigned int result = 0;
    
    /* 1. ZERO_EXTRACT pattern */
    volatile unsigned int extract_start = (seed % 16) + 1;
    volatile unsigned int extract_width = (seed % 8) + 1;
    result ^= zero_extract_example(global_val, extract_start, extract_width);
    
    /* 2. STRICT_LOW_PART pattern */
    volatile int src_val = (int)seed * 0xABCD;
    strict_low_part_example(src_val);
    result ^= (unsigned int)global_short;
    
    /* 3. SUBREG pattern */
    volatile int32_t subreg_val = (int32_t)seed * 0x1234;
    result ^= (unsigned int)subreg_example(subreg_val);
    
    /* 4. Complex MEM address pattern */
    volatile int idx = (seed % 200) + 10;
    volatile int offset = (seed % 20) + 5;
    
    /* Initialize array with some values */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + seed;
    }
    
    result ^= (unsigned int)mem_complex_address_example(
        (volatile int *)global_array, idx, offset);
    
    return result;
}

int main(int argc, char *argv[]) {
    unsigned int seed = 42;  /* Default seed */
    
    /* Use command line argument for seed if provided */
    if (argc > 1) {
        seed = (unsigned int)atoi(argv[1]);
    }
    
    /* Run the combined test multiple times to ensure
       the patterns are processed in scheduling passes */
    unsigned int final_result = 0;
    
    for (int i = 0; i < 100; i++) {
        volatile unsigned int iter_seed = seed + i * 0x12345;
        final_result += combined_test(iter_seed);
    }
    
    printf("Result: %u (0x%08x)\n", final_result, final_result);
    
    return 0;
}
