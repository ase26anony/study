/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

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
int strict_low_part_operation(void) {
    volatile int src = global_val;
    volatile short dest;
    
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    dest = (short)src;
    
    /* Use the result to prevent elimination */
    return (int)dest;
}

/* Function to force generation of SUBREG pattern */
int32_t subreg_operation(void) {
    int32_t a = global_val;
    
    /* Cast to smaller type should generate SUBREG */
    int16_t b = (int16_t)a;
    
    /* Cast back through different size */
    int32_t c = (int32_t)b;
    
    return c;
}

/* Function to force generation of complex MEM address pattern */
int complex_mem_operation(void) {
    int idx = global_idx;
    
    /* Complex addressing: array + variable index + offset */
    /* This should generate MEM with non-simple address expression */
    int value = mem_array[idx * 3 + 7];
    
    /* Another complex memory access with different pattern */
    int value2 = short_array[idx + 10] + mem_array[idx - 2];
    
    return value + value2;
}

/* Main function combining all patterns */
int main(int argc, char *argv[]) {
    unsigned int seed = 0;
    unsigned int final_result = 0;
    
    /* Use command line argument for reproducible but variable input */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = 12345;
    }
    
    /* Initialize with pseudo-random but reproducible values */
    srand(seed);
    global_val = rand();
    global_idx = (rand() % 50) + 1;
    global_start = (rand() % 16) + 1;
    global_width = (rand() % 16) + 1;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        mem_array[i] = rand();
        short_array[i] = (short)(rand() & 0xFFFF);
    }
    
    /* Execute operations in sequence - all in same basic block */
    final_result ^= zero_extract_operation();
    final_result += strict_low_part_operation();
    final_result ^= subreg_operation();
    final_result += complex_mem_operation();
    
    /* Additional mixed operations to increase coverage probability */
    for (int i = 0; i < 3; i++) {
        /* Mixed bitfield and memory operations */
        volatile unsigned int temp = global_val;
        unsigned int extract = (temp >> (i * 3)) & ((1U << 8) - 1);
        
        /* Memory with complex addressing */
        int mem_val = mem_array[global_idx + i * 2];
        
        /* Assignment to smaller type */
        volatile short short_val = (short)mem_val;
        
        final_result ^= extract + (unsigned int)short_val;
    }
    
    /* Force all operations to be observable */
    printf("Result: %u (seed: %u)\n", final_result, seed);
    
    return (int)(final_result & 0x7FFFFFFF);
}
