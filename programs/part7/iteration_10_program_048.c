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
volatile int global_array[256];
volatile int *global_ptr = &global_array[0];

/* Function to force generation of ZERO_EXTRACT pattern */
unsigned int generate_zero_extract(void) {
    /* Non-constant shift and mask to prevent folding */
    unsigned int val = global_val;
    unsigned int start = global_start;
    unsigned int width = global_width;
    
    /* This should generate ZERO_EXTRACT RTL on architectures that support it */
    unsigned int result = (val >> start) & ((1U << width) - 1);
    
    return result;
}

/* Function to force generation of STRICT_LOW_PART pattern */
short generate_strict_low_part(void) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    int src = global_src;
    short dest;
    
    /* Explicit cast and assignment to volatile */
    dest = (short)src;
    global_dest = dest;  /* Store to volatile to ensure side effect */
    
    return dest;
}

/* Function to force generation of SUBREG pattern */
int16_t generate_subreg(void) {
    int32_t a = global_src;
    int16_t b;
    
    /* Cast between different sizes generates SUBREG */
    b = (int16_t)a;
    
    return b;
}

/* Function to force generation of complex MEM address pattern */
int generate_complex_mem(void) {
    volatile int arr[100];
    volatile int idx = 25;  /* Non-constant index */
    int value;
    
    /* Initialize array with some values */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 3;
    }
    
    /* Complex addressing: register + offset */
    value = arr[idx + 5];
    
    /* Even more complex: array with scaled index */
    value += global_array[idx * 2];
    
    /* Pointer arithmetic with variable offset */
    value += *(global_ptr + idx);
    
    return value;
}

/* Main function combining all patterns */
int main(int argc, char *argv[]) {
    unsigned int result = 0;
    
    /* Use command line argument for variability but reproducibility */
    int seed = 1;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize with seed-dependent values */
    global_val = 0x12345678 ^ seed;
    global_start = (4 + seed) % 16;
    global_width = (8 + seed) % 16 + 1;  /* Ensure width > 0 */
    global_src = 0x89ABCDEF ^ seed;
    
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 7 + seed;
    }
    
    /* Execute all pattern-generating operations */
    result ^= generate_zero_extract();
    result ^= (unsigned int)generate_strict_low_part();
    result ^= (unsigned int)generate_subreg();
    result ^= (unsigned int)generate_complex_mem();
    
    /* Combine results in a data-dependent way */
    result = result * 1103515245 + 12345;
    
    /* Print result to ensure all computations are observable */
    printf("Result: 0x%08X\n", result);
    
    return 0;
}
