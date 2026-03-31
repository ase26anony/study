/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent optimization and ensure RTL generation */
volatile unsigned int global_val = 0x12345678;
volatile int global_idx = 5;
volatile short global_short;
volatile int global_src = 0xABCDEF99;
volatile int global_result = 0;

/* Array for complex memory addressing */
volatile int mem_array[256];

/* Function to force generation of ZERO_EXTRACT pattern */
unsigned int generate_zero_extract(volatile unsigned int val, int start, int width) {
    /* Non-constant shift and mask to prevent folding */
    /* This should generate ZERO_EXTRACT RTL on architectures that support it */
    unsigned int mask = (1U << width) - 1;
    return (val >> start) & mask;
}

/* Function to force STRICT_LOW_PART pattern */
void generate_strict_low_part(volatile short *dest, volatile int src) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    *dest = (short)src;
}

/* Function to force SUBREG pattern */
int16_t generate_subreg(int32_t value) {
    /* Cast between different integer sizes generates SUBREG */
    return (int16_t)value;
}

/* Function with complex memory addressing for MEM_P pattern */
int generate_complex_mem_access(volatile int *arr, int idx) {
    /* Complex addressing: array + variable offset + constant */
    /* XEXP(x, 0) will be the address expression */
    return arr[idx + global_idx + 3];
}

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
        mem_array[i] = rand() % 1000;
    }
    
    /* 1. Generate ZERO_EXTRACT pattern */
    /* Use non-constant parameters to prevent constant folding */
    int start_pos = (rand() % 16) + 1;  /* 1-16 */
    int width_val = (rand() % 8) + 1;   /* 1-8 */
    
    extract_result = generate_zero_extract(global_val, start_pos, width_val);
    final_result ^= extract_result;
    
    /* 2. Generate STRICT_LOW_PART pattern */
    generate_strict_low_part(&global_short, global_src);
    final_result ^= global_short;
    
    /* 3. Generate SUBREG pattern */
    subreg_result = generate_subreg(global_src);
    final_result ^= subreg_result;
    
    /* 4. Generate complex MEM access pattern */
    int array_idx = rand() % 200;  /* 0-199 */
    mem_result = generate_complex_mem_access(mem_array, array_idx);
    final_result ^= mem_result;
    
    /* Combine all results in a way that can't be optimized away */
    global_result = final_result;
    
    /* Print result to ensure side effects */
    printf("Result: %d (seed: %d)\n", global_result, seed);
    printf("Extract: %u, Subreg: %d, Mem: %d, Short: %d\n", 
           extract_result, subreg_result, mem_result, global_short);
    
    return 0;
}
