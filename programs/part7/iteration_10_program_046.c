/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent optimization and ensure RTL generation */
volatile unsigned int global_val = 0xDEADBEEF;
volatile int global_idx = 0;
volatile short global_short;
volatile int global_src = 0x12345678;
volatile int global_result = 0;

/* Array for complex memory addressing */
volatile int mem_array[256];

/* Function to generate ZERO_EXTRACT pattern */
unsigned int generate_zero_extract(volatile unsigned int val, int start, int width) {
    /* Non-constant shift and mask to prevent folding */
    /* This should generate ZERO_EXTRACT RTL on architectures like ARM */
    unsigned int mask = ((1U << width) - 1);
    return (val >> start) & mask;
}

/* Function to generate STRICT_LOW_PART pattern */
void generate_strict_low_part(volatile int src) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    global_short = (short)src;
}

/* Function to generate SUBREG pattern */
int16_t generate_subreg(int32_t a) {
    /* Cast between different integer sizes generates SUBREG */
    return (int16_t)a;
}

/* Function to generate complex MEM address pattern */
int generate_complex_mem(volatile int *arr, int idx) {
    /* Complex addressing: arr[idx + offset] */
    /* XEXP(x, 0) will be the address expression (idx + offset) */
    int offset = 7;
    return arr[idx + offset];
}

/* Main function combining all patterns */
int main(int argc, char *argv[]) {
    int i;
    unsigned int extract_result;
    int16_t subreg_result;
    int mem_result;
    int final_result = 0;
    
    /* Initialize with pseudo-random but reproducible values */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Initialize array */
    for (i = 0; i < 256; i++) {
        mem_array[i] = rand();
    }
    
    global_idx = rand() % 200;
    
    /* 1. Generate ZERO_EXTRACT pattern */
    /* Use non-constant width and shift to prevent optimization */
    int width = (rand() % 16) + 1;  /* 1-16 bits */
    int start = (rand() % 16) + 1;  /* 1-16 bits */
    extract_result = generate_zero_extract(global_val, start, width);
    
    /* 2. Generate STRICT_LOW_PART pattern */
    generate_strict_low_part(global_src);
    
    /* 3. Generate SUBREG pattern */
    subreg_result = generate_subreg(global_src);
    
    /* 4. Generate complex MEM address pattern */
    mem_result = generate_complex_mem(mem_array, global_idx);
    
    /* Combine all results to ensure all operations are used */
    final_result = extract_result + subreg_result + mem_result + global_short;
    
    /* Print result to create observable side effect */
    printf("Final result: %d (0x%08x)\n", final_result, final_result);
    
    /* Additional loop to increase chance of scheduling analysis */
    for (i = 0; i < 100; i++) {
        /* Mix operations in a loop to create scheduling pressure */
        int temp_idx = (global_idx + i) % 200;
        int temp = generate_complex_mem(mem_array, temp_idx);
        global_result ^= temp;
        
        /* More bitfield operations */
        unsigned int temp_extract = generate_zero_extract(temp, i % 8, 4);
        global_result += temp_extract;
    }
    
    printf("Global result: %d\n", global_result);
    
    return 0;
}
