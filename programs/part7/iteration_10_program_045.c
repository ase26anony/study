/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile uint32_t global_val = 0xDEADBEEF;
volatile uint16_t global_short;
volatile uint8_t global_byte;
volatile int32_t global_array[256];
volatile int32_t *global_ptr = (int32_t*)global_array;

/* Force generation of ZERO_EXTRACT pattern */
static uint32_t zero_extract_pattern(volatile uint32_t val, int start, int width) {
    /* Non-constant shift and mask to prevent folding */
    uint32_t mask = (1U << width) - 1;
    return (val >> start) & mask;
}

/* Force generation of STRICT_LOW_PART pattern */
static void strict_low_part_pattern(volatile uint32_t src) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    global_short = (uint16_t)src;
    global_byte = (uint8_t)src;
}

/* Force generation of SUBREG pattern */
static uint16_t subreg_pattern(uint32_t a) {
    /* Cast to smaller type generates SUBREG */
    uint16_t b = (uint16_t)a;
    uint8_t c = (uint8_t)a;
    return b + c;
}

/* Force generation of complex MEM address pattern */
static int32_t complex_mem_pattern(volatile int32_t *arr, int idx) {
    /* Variable index with offset creates complex addressing */
    return arr[idx + 5] + arr[idx * 2];
}

/* Combined function with all patterns */
static uint32_t generate_all_patterns(int seed) {
    uint32_t result = 0;
    
    /* Initialize with seed to create variable but reproducible data */
    volatile uint32_t base_val = seed * 0x1234567;
    
    /* 1. ZERO_EXTRACT pattern */
    int start = (seed % 16) + 1;      /* Non-constant start position */
    int width = (seed % 8) + 1;       /* Non-constant width */
    result ^= zero_extract_pattern(base_val, start, width);
    
    /* 2. STRICT_LOW_PART pattern */
    strict_low_part_pattern(base_val ^ 0x87654321);
    
    /* 3. SUBREG pattern */
    result += subreg_pattern(base_val);
    
    /* 4. Complex MEM pattern with variable addressing */
    /* Initialize array with some data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = (i * seed) & 0xFF;
    }
    
    int idx = (seed * 3) % 200;  /* Variable index */
    result += complex_mem_pattern(global_array, idx);
    
    /* 5. Additional MEM pattern with pointer arithmetic */
    int offset = (seed % 50) * 4;  /* Variable offset in bytes */
    result += *(global_ptr + offset / sizeof(int32_t));
    
    /* 6. Mixed patterns in a loop to increase coverage probability */
    for (int i = 0; i < 3; i++) {
        /* More ZERO_EXTRACT with different parameters */
        result ^= zero_extract_pattern(global_val, (i * 3) % 16, (i * 2) % 8 + 1);
        
        /* More STRICT_LOW_PART assignments */
        volatile uint16_t temp_short = (uint16_t)result;
        global_short = temp_short;
        
        /* More complex MEM accesses */
        int temp_idx = (idx + i * 7) % 200;
        result += global_array[temp_idx * 2 + 1];
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Generate all RTL patterns multiple times */
    uint32_t final_result = 0;
    for (int iteration = 0; iteration < 5; iteration++) {
        final_result ^= generate_all_patterns(seed + iteration);
        
        /* Force some register pressure */
        volatile uint32_t temp = final_result;
        final_result = (final_result << 1) | (final_result >> 31);
        final_result ^= temp;
    }
    
    /* Print result to ensure all computations are observable */
    printf("Result: 0x%08X\n", final_result);
    
    return 0;
}
