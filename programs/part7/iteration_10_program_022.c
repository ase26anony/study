/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent optimization and ensure RTL generation */
volatile unsigned int global_val = 0xDEADBEEF;
volatile int global_idx = 0;
volatile short global_short;
volatile int global_src = 0x12345678;
volatile int global_dest;

/* Array for complex memory addressing */
volatile int mem_array[256];

/* Function to ensure operations aren't optimized away */
__attribute__((noinline))
int perform_operations(int seed) {
    volatile int result = 0;
    
    /* 1. Generate ZERO_EXTRACT pattern */
    /* Non-constant shift and width to prevent folding */
    volatile unsigned int val = global_val ^ seed;
    volatile int start = (seed % 16) + 1;
    volatile int width = (seed % 8) + 1;
    
    /* This should generate ZERO_EXTRACT RTL on architectures that support it */
    unsigned int extracted = (val >> start) & ((1U << width) - 1);
    result ^= extracted;
    
    /* 2. Generate STRICT_LOW_PART pattern */
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    volatile short dest_short;
    volatile int src_int = global_src + seed;
    
    /* This assignment might generate STRICT_LOW_PART for the low 16 bits */
    dest_short = (short)src_int;
    global_short = dest_short;
    result += dest_short;
    
    /* 3. Generate SUBREG pattern (very common) */
    /* Cast between different sized integers generates SUBREG */
    int32_t a = src_int;
    int16_t b = (int16_t)a;  /* This generates SUBREG */
    result += b;
    
    /* 4. Generate MEM_P with complex addressing */
    /* Variable index with offset creates complex memory address */
    volatile int idx = global_idx + seed;
    
    /* Complex addressing: array base + variable index + constant offset */
    int mem_value = mem_array[idx + 5];  /* MEM with complex address */
    result ^= mem_value;
    
    /* Additional complex memory access with pointer arithmetic */
    volatile int *ptr = &mem_array[100];
    int mem_value2 = ptr[idx & 0x3F];    /* Another complex MEM address */
    result += mem_value2;
    
    /* Mix in some more bit-field operations for good measure */
    /* Another ZERO_EXTRACT candidate with different parameters */
    volatile unsigned int val2 = ~val;
    volatile int start2 = (seed % 12) + 4;
    volatile int width2 = (seed % 6) + 2;
    unsigned int extracted2 = (val2 >> start2) & ((1U << width2) - 1);
    result ^= extracted2;
    
    return result;
}

/* Helper function to initialize array with pseudo-random values */
void init_array(int seed) {
    for (int i = 0; i < 256; i++) {
        mem_array[i] = (i * 1103515245 + seed) & 0x7FFFFFFF;
    }
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    /* Use command line argument for seed if provided */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize with seed for reproducibility */
    init_array(seed);
    global_idx = seed % 128;
    
    /* Perform operations multiple times to increase coverage chances */
    int final_result = 0;
    for (int i = 0; i < 10; i++) {
        final_result ^= perform_operations(seed + i);
    }
    
    /* Print result to prevent optimization and verify execution */
    printf("Result: %d (0x%08X)\n", final_result, final_result);
    
    return 0;
}
