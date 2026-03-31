/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent optimization and ensure RTL generation */
volatile unsigned int global_val = 0x12345678;
volatile int global_idx = 5;
volatile short global_short;
volatile int global_src = 0x89ABCDEF;
volatile int arr[100];

/* Function to generate ZERO_EXTRACT pattern */
unsigned int generate_zero_extract(volatile unsigned int val, int start, int width) {
    /* Non-constant shift and mask to prevent folding */
    unsigned int shifted = val >> start;
    unsigned int mask = (1U << width) - 1;
    return shifted & mask;
}

/* Function to generate STRICT_LOW_PART pattern */
void generate_strict_low_part(volatile int src) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    global_short = (short)src;
}

/* Function to generate SUBREG pattern */
int16_t generate_subreg(int32_t a) {
    /* Cast between different integer sizes */
    return (int16_t)a;
}

/* Function to generate complex MEM address pattern */
int generate_complex_mem(volatile int *array, int idx) {
    /* Variable index with offset creates complex addressing */
    return array[idx + global_idx];
}

/* Main function combining all patterns */
int main(int argc, char *argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize with pseudo-random but reproducible values */
    srand(seed);
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = rand() % 1000;
    }
    
    /* Use non-constant parameters to prevent optimization */
    int start = (seed % 16) + 1;
    int width = (seed % 8) + 1;
    
    int result = 0;
    
    /* 1. Generate ZERO_EXTRACT pattern */
    unsigned int extract_result = generate_zero_extract(global_val, start, width);
    result ^= extract_result;
    
    /* 2. Generate STRICT_LOW_PART pattern */
    generate_strict_low_part(global_src);
    result ^= global_short;
    
    /* 3. Generate SUBREG pattern */
    int32_t src_val = global_src ^ seed;
    int16_t subreg_result = generate_subreg(src_val);
    result ^= subreg_result;
    
    /* 4. Generate complex MEM address pattern */
    int mem_result = generate_complex_mem(arr, seed % 50);
    result ^= mem_result;
    
    /* Additional patterns in same basic block */
    /* Another SUBREG pattern */
    volatile int64_t large_val = 0x1122334455667788ULL;
    volatile int32_t truncated = (int32_t)large_val;
    result ^= truncated;
    
    /* Another complex MEM with different addressing */
    volatile int *ptr = arr + (seed % 20);
    result ^= ptr[global_idx * 2];
    
    /* Bitfield operations that might generate ZERO_EXTRACT */
    struct bitfield {
        unsigned int field1 : 4;
        unsigned int field2 : 8;
        unsigned int field3 : 4;
    };
    
    volatile struct bitfield bf;
    bf.field1 = (seed >> 4) & 0xF;
    bf.field2 = seed & 0xFF;
    bf.field3 = (seed >> 8) & 0xF;
    
    result ^= bf.field2;
    
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
