/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent optimization and ensure RTL generation */
volatile unsigned int global_val = 0xDEADBEEF;
volatile unsigned int global_start = 8;
volatile unsigned int global_width = 12;
volatile int global_src = 0x12345678;
volatile short global_dest;
volatile int global_arr[256];
volatile int *global_ptr = (int*)global_arr;

/* Function to generate ZERO_EXTRACT pattern */
unsigned int gen_zero_extract(void) {
    /* Non-constant shift and mask to prevent folding */
    unsigned int val = global_val;
    unsigned int start = global_start;
    unsigned int width = global_width;
    
    /* This should generate ZERO_EXTRACT RTL on architectures that support it */
    unsigned int result = (val >> start) & ((1U << width) - 1);
    
    return result;
}

/* Function to generate STRICT_LOW_PART pattern */
short gen_strict_low_part(void) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    int src = global_src;
    short dest;
    
    /* Explicit cast to smaller type */
    dest = (short)src;
    global_dest = dest;  /* Volatile store to force generation */
    
    return dest;
}

/* Function to generate SUBREG pattern */
int16_t gen_subreg(void) {
    int32_t a = global_src;
    int16_t b;
    
    /* Cast between different-sized integers generates SUBREG */
    b = (int16_t)a;
    
    return b;
}

/* Function to generate complex MEM address pattern */
int gen_complex_mem(void) {
    volatile int arr[100];
    int idx = global_start;
    int value;
    
    /* Initialize array with some values */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 3;
    }
    
    /* Complex addressing: arr[idx + 5] with variable index */
    value = arr[idx + 5];
    
    return value;
}

/* Function combining all patterns in one basic block */
unsigned int combine_patterns(int seed) {
    volatile unsigned int result = 0;
    volatile int temp;
    
    /* 1. ZERO_EXTRACT pattern */
    result ^= gen_zero_extract();
    
    /* 2. STRICT_LOW_PART pattern */
    result += (unsigned int)gen_strict_low_part();
    
    /* 3. SUBREG pattern */
    result ^= (unsigned int)gen_subreg();
    
    /* 4. Complex MEM address pattern */
    result += (unsigned int)gen_complex_mem();
    
    /* Additional direct patterns in same basic block */
    
    /* Direct bitfield extract with volatile */
    volatile unsigned int v = 0xABCD1234;
    volatile unsigned int s = 4;
    volatile unsigned int w = 10;
    unsigned int direct_extract = (v >> s) & ((1U << w) - 1);
    result ^= direct_extract;
    
    /* Direct memory with complex addressing */
    volatile int mem_arr[64];
    volatile int *mem_ptr = mem_arr + seed;
    temp = *mem_ptr;  /* MEM with register+offset addressing */
    result += temp;
    
    /* Another SUBREG example */
    volatile long long big = 0x1122334455667788LL;
    volatile int medium = (int)big;  /* SUBREG from 64 to 32 bits */
    result ^= medium;
    
    return result;
}

int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Use command line argument for seed to create variable but reproducible data */
    if (argc > 1) {
        seed = atoi(argv[1]) % 100;
    }
    
    /* Initialize global array with pseudo-random values */
    srand(seed);
    for (int i = 0; i < 256; i++) {
        global_arr[i] = rand();
    }
    
    /* Call function that combines all patterns */
    unsigned int final_result = combine_patterns(seed);
    
    /* Print result to ensure all computations are observable */
    printf("Result: 0x%08X\n", final_result);
    
    return 0;
}
