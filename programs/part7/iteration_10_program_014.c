/* test_resources.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent optimization and ensure RTL generation */
volatile unsigned int global_val = 0x12345678;
volatile int global_idx = 5;
volatile unsigned int global_start = 4;
volatile unsigned int global_width = 8;

/* Arrays for memory access patterns */
volatile int mem_array[256];
volatile short short_array[256];

/* Function to force generation of ZERO_EXTRACT pattern */
unsigned int zero_extract_operation(void) {
    /* This should generate ZERO_EXTRACT RTL when compiled with optimization.
     * The volatile variables prevent constant folding.
     * Pattern: (val >> start) & ((1U << width) - 1)
     */
    unsigned int val = global_val;
    unsigned int start = global_start;
    unsigned int width = global_width;
    
    /* Non-constant width and start force extract pattern */
    unsigned int mask = (1U << width) - 1;
    unsigned int result = (val >> start) & mask;
    
    return result;
}

/* Function to force generation of STRICT_LOW_PART pattern */
short strict_low_part_operation(void) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    volatile int src = global_val + 0x100;
    volatile short dest;
    
    /* This assignment might use STRICT_LOW_PART to preserve high bits
     * when writing to a sub-register */
    dest = (short)src;
    
    return dest;
}

/* Function to force generation of SUBREG pattern */
int16_t subreg_operation(void) {
    /* Cast between different-sized integers generates SUBREG */
    int32_t a = global_val;
    int16_t b = (int16_t)a;  /* This should generate SUBREG */
    
    return b;
}

/* Function to force generation of complex MEM address pattern */
int complex_mem_operation(void) {
    /* Complex addressing mode: array with variable index and offset */
    int idx = global_idx;
    
    /* This should generate MEM with complex address: &mem_array[idx + 10] */
    int value = mem_array[idx + 10];
    
    return value;
}

/* Combined function that uses all patterns in sequence */
unsigned int combined_operations(int seed) {
    unsigned int result = 0;
    
    /* Initialize arrays with pseudo-random but reproducible values */
    for (int i = 0; i < 256; i++) {
        mem_array[i] = (i * 37 + seed) & 0xFF;
        short_array[i] = (i * 19 + seed) & 0xFFFF;
    }
    
    /* 1. ZERO_EXTRACT pattern */
    result ^= zero_extract_operation();
    
    /* 2. STRICT_LOW_PART pattern */
    result += strict_low_part_operation();
    
    /* 3. SUBREG pattern */
    result |= subreg_operation();
    
    /* 4. Complex MEM address pattern */
    result ^= complex_mem_operation();
    
    /* Additional complex memory access with pointer arithmetic */
    volatile int *ptr = &mem_array[50];
    result += ptr[global_idx * 2];  /* MEM with scaled index */
    
    /* Another bit-field extract with different parameters */
    volatile unsigned int val2 = result;
    volatile unsigned int shift = (seed % 16) + 1;
    volatile unsigned int width2 = (seed % 8) + 1;
    unsigned int extract2 = (val2 >> shift) & ((1U << width2) - 1);
    
    return result + extract2;
}

/* Main function with command-line argument for variability */
int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize global variables with seed-dependent values */
    global_val = 0x89ABCDEF ^ seed;
    global_idx = (seed % 100) + 1;
    global_start = (seed % 24) + 1;
    global_width = (seed % 16) + 1;
    
    /* Perform combined operations */
    unsigned int final_result = combined_operations(seed);
    
    /* Print result to ensure all computations are observable */
    printf("Result: 0x%08X\n", final_result);
    
    /* Additional loop to increase chance of scheduling analysis */
    for (int i = 0; i < 10; i++) {
        /* Mix in more operations that could generate target RTL */
        volatile short s = (short)(final_result + i);
        volatile int arr_val = mem_array[(global_idx + i) % 256];
        final_result ^= (s + arr_val);
    }
    
    printf("Final result: 0x%08X\n", final_result);
    
    return (final_result & 0xFF);
}
