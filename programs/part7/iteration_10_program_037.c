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
    int src = global_src;
    
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    global_dest = (short)src;
    
    return global_dest;
}

/* Function to generate SUBREG pattern */
int16_t gen_subreg(void) {
    int32_t a = global_src;
    
    /* Cast to smaller type generates SUBREG */
    int16_t b = (int16_t)a;
    
    return b;
}

/* Function to generate complex MEM address pattern */
int gen_complex_mem(void) {
    volatile int idx = 42;
    volatile int offset = 7;
    
    /* Complex addressing: array + variable index + offset */
    /* XEXP(x, 0) will be (plus (reg) (const_int)) or similar */
    int value = global_arr[idx + offset];
    
    return value;
}

/* Function with all patterns in one basic block */
unsigned int combined_patterns(int seed) {
    volatile unsigned int result = 0;
    
    /* Initialize array with pseudo-random values */
    for (int i = 0; i < 256; i++) {
        global_arr[i] = (i * seed) ^ 0xABCD;
    }
    
    /* 1. ZERO_EXTRACT pattern */
    unsigned int extract_result = gen_zero_extract();
    result ^= extract_result;
    
    /* 2. STRICT_LOW_PART pattern */
    short low_part = gen_strict_low_part();
    result += (unsigned int)low_part;
    
    /* 3. SUBREG pattern */
    int16_t subreg_val = gen_subreg();
    result ^= (unsigned int)subreg_val;
    
    /* 4. Complex MEM address pattern */
    int mem_val = gen_complex_mem();
    result += (unsigned int)mem_val;
    
    /* Additional: Pointer arithmetic with variable offset */
    volatile int offset2 = seed % 64;
    int ptr_val = *(global_ptr + offset2);
    result ^= (unsigned int)ptr_val;
    
    /* Another MEM pattern: register + scaled index */
    volatile int idx2 = seed % 128;
    int scaled_val = global_arr[idx2 * 2 + 3];
    result += scaled_val;
    
    return result;
}

/* Alternative: All operations in a tight loop */
void loop_patterns(int iterations) {
    volatile unsigned int accumulator = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix different patterns in loop body */
        volatile int temp = i;
        
        /* ZERO_EXTRACT-like operation */
        unsigned int val = global_val + i;
        unsigned int start = (temp & 0xF) + 1;
        unsigned int width = (temp & 0x7) + 1;
        unsigned int extracted = (val >> start) & ((1U << width) - 1);
        accumulator ^= extracted;
        
        /* STRICT_LOW_PART-like assignment */
        volatile short short_var;
        int int_var = accumulator + i;
        short_var = (short)int_var;
        accumulator += short_var;
        
        /* SUBREG through cast */
        int32_t src32 = accumulator;
        int16_t dst16 = (int16_t)src32;
        accumulator ^= dst16;
        
        /* Complex MEM access */
        int mem_idx = (temp * 3) % 256;
        int mem_val = global_arr[mem_idx + (i & 0xF)];
        accumulator += mem_val;
    }
    
    /* Use result to prevent dead code elimination */
    global_val = accumulator;
}

int main(int argc, char *argv[]) {
    int seed = 12345;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize with seed for reproducibility */
    srand(seed);
    global_val = rand();
    global_start = (rand() % 24) + 1;
    global_width = (rand() % 16) + 1;
    global_src = rand();
    
    /* Initialize array */
    for (int i = 0; i < 256; i++) {
        global_arr[i] = rand();
    }
    
    /* Generate patterns */
    unsigned int result1 = combined_patterns(seed);
    
    /* Generate more patterns in loop */
    loop_patterns(100);
    
    /* Combine results */
    unsigned int final_result = result1 ^ global_val;
    
    printf("Result: 0x%08X\n", final_result);
    
    return (int)(final_result & 0xFF);
}
