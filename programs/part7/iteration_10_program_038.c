/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile unsigned int global_val = 0x12345678;
volatile int global_idx = 5;
volatile unsigned int global_start = 4;
volatile unsigned int global_width = 8;
volatile int global_array[100];
volatile short global_short;
volatile int global_src_int = 0x89ABCDEF;

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
short strict_low_part_operation(void) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    int src = global_src_int;
    short dest;
    
    /* Explicit cast to smaller type */
    dest = (short)src;
    global_short = dest;  /* Volatile store */
    
    return dest;
}

/* Function with SUBREG patterns */
int32_t subreg_operations(void) {
    int32_t a = global_src_int;
    int16_t b;
    int8_t c;
    
    /* Multiple casts between different sizes */
    b = (int16_t)a;      /* Should generate SUBREG */
    c = (int8_t)b;       /* Another SUBREG */
    
    return (int32_t)c;   /* Sign extension with SUBREG */
}

/* Function with complex MEM addressing */
int memory_operation(void) {
    volatile int* volatile_ptr = (volatile int*)global_array;
    int idx = global_idx;
    
    /* Complex addressing: base + (index * scale) + offset */
    /* Should generate MEM with non-simple address expression */
    int value = ptr[idx * 2 + 3];
    
    return value;
}

/* Main function combining all patterns */
int main(int argc, char *argv[]) {
    int i;
    unsigned int final_result = 0;
    
    /* Initialize with pseudo-random but reproducible values */
    unsigned int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Initialize array with some values */
    for (i = 0; i < 100; i++) {
        global_array[i] = rand() % 1000;
    }
    
    /* Set up parameters for extract operation */
    global_start = (rand() % 16) + 1;  /* 1-16 */
    global_width = (rand() % 8) + 1;   /* 1-8 */
    
    /* Execute all operations in sequence */
    unsigned int extract_result = zero_extract_operation();
    short low_part_result = strict_low_part_operation();
    int32_t subreg_result = subreg_operations();
    int mem_result = memory_operation();
    
    /* Combine results to ensure all computations are used */
    final_result = extract_result 
                   + (unsigned int)low_part_result 
                   + (unsigned int)subreg_result 
                   + (unsigned int)mem_result;
    
    /* Print result to create observable side effect */
    printf("Final result: %u (seed: %u)\n", final_result, seed);
    
    return (int)(final_result & 0xFF);
}
