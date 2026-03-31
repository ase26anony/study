/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile unsigned int global_val = 0x12345678;
volatile int global_idx = 5;
volatile unsigned int global_start = 4;
volatile unsigned int global_width = 8;

/* Volatile arrays for memory addressing */
volatile int mem_array[100];
volatile short short_array[100];

/* Function to force generation of ZERO_EXTRACT pattern */
unsigned int zero_extract_operation(void) {
    /* Non-constant shift and mask to prevent folding */
    unsigned int val = global_val;
    unsigned int start = global_start;
    unsigned int width = global_width;
    
    /* This should generate ZERO_EXTRACT RTL on architectures that support it */
    unsigned int result = (val >> start) & ((1U << width) - 1);
    
    return result;
}

/* Function to force generation of STRICT_LOW_PART pattern */
void strict_low_part_operation(void) {
    volatile int src = global_val;
    volatile short dest;
    
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    dest = (short)src;
    
    /* Use the result to prevent dead code elimination */
    short_array[0] = dest;
}

/* Function to force generation of SUBREG pattern */
int32_t subreg_operation(int32_t a) {
    /* Cast between different integer sizes generates SUBREG */
    int16_t b = (int16_t)a;
    int32_t c = (int32_t)b;  /* Another SUBREG */
    
    return c;
}

/* Function to force complex MEM addressing */
int complex_mem_operation(void) {
    volatile int idx = global_idx;
    
    /* Complex addressing: array + variable index + offset */
    int value = mem_array[idx + 3];
    
    /* Even more complex: pointer arithmetic */
    volatile int *ptr = &mem_array[10];
    int value2 = ptr[idx * 2 - 1];
    
    return value + value2;
}

/* Combined operations in main to hit all patterns */
int main(int argc, char *argv[]) {
    int i;
    
    /* Initialize with pseudo-random but reproducible values */
    unsigned int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Initialize arrays */
    for (i = 0; i < 100; i++) {
        mem_array[i] = rand() % 1000;
        short_array[i] = (short)(rand() % 1000);
    }
    
    /* Set global variables */
    global_val = rand();
    global_idx = rand() % 50;
    global_start = rand() % 16;
    global_width = (rand() % 8) + 1;  /* width 1-8 */
    
    /* Execute all pattern-generating operations */
    unsigned int result1 = zero_extract_operation();
    strict_low_part_operation();
    int32_t result2 = subreg_operation(global_val);
    int result3 = complex_mem_operation();
    
    /* Combine results to ensure all operations are used */
    unsigned int final_result = result1 + result2 + result3;
    
    /* Print to create observable side effect */
    printf("Result: %u (seed: %u)\n", final_result, seed);
    
    return 0;
}
