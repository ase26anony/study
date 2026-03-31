/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent optimization and ensure RTL generation */
volatile unsigned int global_val = 0x12345678;
volatile int global_idx = 0;
volatile int global_start = 4;
volatile int global_width = 8;

/* Arrays for memory access patterns */
volatile int mem_array[256];
volatile short short_array[256];

/* Force generation of ZERO_EXTRACT pattern */
static unsigned int zero_extract_operation(void) {
    /* Non-constant shift and mask to prevent folding */
    volatile unsigned int val = global_val;
    volatile int start = global_start;
    volatile int width = global_width;
    
    /* This should generate ZERO_EXTRACT RTL on architectures that support it */
    unsigned int result = (val >> start) & ((1U << width) - 1);
    
    /* Make result dependent on all inputs */
    return result + (start * width);
}

/* Force generation of STRICT_LOW_PART pattern */
static int strict_low_part_operation(void) {
    volatile int src = global_val;
    volatile short dest;
    
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    dest = (short)src;
    
    /* Also try with explicit bitmask */
    volatile int masked = src & 0xFFFF;
    dest = (short)masked;
    
    return (int)dest;
}

/* Force generation of SUBREG pattern */
static int subreg_operation(void) {
    volatile int32_t full_reg = global_val;
    volatile int16_t half_reg;
    
    /* Cast between different sizes generates SUBREG */
    half_reg = (int16_t)full_reg;
    
    /* Another SUBREG through pointer cast */
    volatile int8_t byte_reg = (int8_t)(full_reg >> 16);
    
    return (int)half_reg + byte_reg;
}

/* Force generation of complex MEM address pattern */
static int complex_mem_operation(void) {
    volatile int idx = global_idx;
    volatile int offset = 5;
    
    /* Complex addressing: array base + variable index + constant offset */
    /* XEXP(x, 0) will be the address expression (idx + offset) */
    int value = mem_array[idx + offset];
    
    /* More complex addressing with scaling */
    int scaled_idx = idx * 2;
    value += mem_array[scaled_idx + 3];
    
    /* Pointer arithmetic version */
    volatile int *ptr = (volatile int *)mem_array;
    value += ptr[idx + 7];
    
    return value;
}

/* Combine all operations to ensure they're in same compilation unit */
static int combine_operations(int seed) {
    int result = 0;
    
    /* Use seed to make operations data-dependent */
    global_val = (seed * 1103515245 + 12345) & 0xFFFFFFFF;
    global_idx = (seed * 31) % 200;
    global_start = (seed % 24) + 1;
    global_width = (seed % 16) + 1;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        mem_array[i] = (i * seed) & 0xFF;
        short_array[i] = (short)((i * seed * 3) & 0xFFFF);
    }
    
    /* Execute all pattern-generating operations */
    result ^= zero_extract_operation();
    result += strict_low_part_operation();
    result ^= subreg_operation();
    result += complex_mem_operation();
    
    return result;
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    int iterations = 10;
    
    /* Use command line argument for seed if provided */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    int final_result = 0;
    
    /* Loop to increase chance of hitting the code paths */
    for (int i = 0; i < iterations; i++) {
        int current_seed = seed + i;
        final_result ^= combine_operations(current_seed);
        
        /* Force side effects to prevent dead code elimination */
        printf("Iteration %d: intermediate = %d\n", i, final_result & 0xFF);
    }
    
    /* Final result depends on all operations */
    printf("Final result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
