/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile unsigned int global_val = 0x12345678;
volatile unsigned int global_start = 4;
volatile unsigned int global_width = 8;
volatile int global_src = 0x89ABCDEF;
volatile short global_dest;
volatile int global_arr[256];
volatile int *global_ptr = &global_arr[0];

/* Function to force generation of ZERO_EXTRACT pattern */
unsigned int zero_extract_pattern(void) {
    /* Non-constant shift and mask to prevent folding */
    unsigned int val = global_val;
    unsigned int start = global_start;
    unsigned int width = global_width;
    
    /* This should generate ZERO_EXTRACT RTL on architectures 
       that support bit-field extract instructions */
    unsigned int result = (val >> start) & ((1U << width) - 1);
    
    return result;
}

/* Function to force generation of STRICT_LOW_PART pattern */
short strict_low_part_pattern(void) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    int src = global_src;
    short dest;
    
    /* Explicit cast to smaller type */
    dest = (short)src;
    global_dest = dest;  /* Volatile store to ensure side effect */
    
    return dest;
}

/* Function to force generation of SUBREG pattern */
int16_t subreg_pattern(int32_t a) {
    /* Cast between different integer sizes generates SUBREG */
    int16_t b = (int16_t)a;
    return b;
}

/* Function to force generation of complex MEM address pattern */
int mem_complex_address_pattern(void) {
    /* Use variable index with offset for complex addressing */
    volatile int arr[256];
    int idx = global_start;
    int value;
    
    /* Complex address: base + (index * scale) + offset */
    value = arr[idx * 2 + 5];
    
    /* Another complex address: pointer arithmetic */
    value += global_ptr[idx + 3];
    
    return value;
}

/* Combined function that uses all patterns */
unsigned int combined_patterns(int seed) {
    unsigned int result = 0;
    
    /* 1. ZERO_EXTRACT pattern */
    result ^= zero_extract_pattern();
    
    /* 2. STRICT_LOW_PART pattern */
    result ^= (unsigned int)strict_low_part_pattern();
    
    /* 3. SUBREG pattern */
    int32_t a = seed * 0x12345;
    result ^= (unsigned int)subreg_pattern(a);
    
    /* 4. Complex MEM address pattern */
    result ^= (unsigned int)mem_complex_address_pattern();
    
    return result;
}

/* Main function with data-dependent operations */
int main(int argc, char *argv[]) {
    int seed = 1;
    
    /* Use command line argument for variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize array with pseudo-random but reproducible values */
    for (int i = 0; i < 256; i++) {
        global_arr[i] = (i * seed) ^ 0xDEADBEEF;
    }
    
    /* Set global variables based on seed */
    global_val = 0x87654321 ^ seed;
    global_start = (seed % 24) + 1;
    global_width = (seed % 16) + 1;
    global_src = 0x12345678 ^ seed;
    
    /* Execute combined patterns multiple times in a loop
       to increase chance of hitting the code paths */
    unsigned int final_result = 0;
    for (int i = 0; i < 10; i++) {
        final_result += combined_patterns(seed + i);
    }
    
    /* Print result to ensure side effects */
    printf("Result: 0x%08X\n", final_result);
    
    return 0;
}
