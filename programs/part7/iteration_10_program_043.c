/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent optimization and ensure RTL generation */
volatile unsigned int global_val = 0x12345678;
volatile unsigned int global_start = 4;
volatile unsigned int global_width = 8;
volatile int global_src = 0x89ABCDEF;
volatile short global_dest;
volatile int global_arr[256];
volatile int *global_ptr = (int*)global_arr;

/* Function to force generation of ZERO_EXTRACT pattern */
unsigned int zero_extract_operation(void) {
    /* Non-constant shift and mask to prevent folding */
    unsigned int val = global_val;
    unsigned int start = global_start;
    unsigned int width = global_width;
    
    /* This should generate ZERO_EXTRACT RTL on architectures that support it */
    unsigned int result = (val >> start) & ((1U << width) - 1);
    
    /* Add dependency to prevent dead code elimination */
    global_val = result;
    return result;
}

/* Function to force generation of STRICT_LOW_PART pattern */
short strict_low_part_operation(void) {
    int src = global_src;
    
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    short dest = (short)src;
    
    /* Store to volatile to ensure the operation isn't optimized away */
    global_dest = dest;
    return dest;
}

/* Function to force generation of SUBREG pattern */
int16_t subreg_operation(int32_t input) {
    /* Explicit cast between different-sized types generates SUBREG */
    int16_t result = (int16_t)input;
    
    /* Use result to prevent optimization */
    return result + 1;
}

/* Function with complex memory addressing for MEM_P pattern */
int memory_operation(int idx) {
    /* Complex addressing: base + (index * scale) + offset */
    /* Use volatile pointer to ensure memory access isn't optimized */
    int value = global_ptr[idx * 2 + 3];
    
    /* Additional complexity: pointer arithmetic */
    volatile int *ptr2 = global_ptr + idx;
    value += *(ptr2 - 1);
    
    return value;
}

/* Main function combining all patterns */
int main(int argc, char *argv[]) {
    int i, result = 0;
    
    /* Use command-line argument for variability but reproducibility */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Initialize array with pseudo-random values */
    for (i = 0; i < 256; i++) {
        global_arr[i] = rand();
    }
    
    /* Set up parameters for operations */
    global_start = (rand() % 16) + 1;  /* 1-16 */
    global_width = (rand() % 8) + 1;   /* 1-8 */
    global_src = rand();
    
    /* Execute operations in sequence to create basic block */
    
    /* 1. ZERO_EXTRACT pattern */
    result ^= zero_extract_operation();
    
    /* 2. STRICT_LOW_PART pattern */
    result += strict_low_part_operation();
    
    /* 3. SUBREG pattern */
    result += subreg_operation(global_src);
    
    /* 4. Complex MEM_P pattern with addressing mode */
    int idx = rand() % 100;
    result += memory_operation(idx);
    
    /* Additional: Combined operation that might generate multiple patterns */
    {
        volatile struct {
            unsigned int field : 8;
        } bitfield;
        
        /* Bitfield assignment might generate ZERO_EXTRACT or STRICT_LOW_PART */
        bitfield.field = (global_val >> 4) & 0xFF;
        result += bitfield.field;
    }
    
    /* Print result to ensure all computations are observable */
    printf("Result: %d (seed: %d)\n", result, seed);
    
    return 0;
}
