/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile uint32_t global_val = 0x12345678;
volatile uint16_t global_short;
volatile uint8_t global_byte;
volatile int32_t global_array[256];
volatile int32_t *global_ptr = (int32_t*)global_array;

/* Function to generate ZERO_EXTRACT pattern */
static uint32_t generate_zero_extract(volatile uint32_t val, int start, int width) {
    /* Non-constant shift and mask to prevent folding */
    /* Should generate ZERO_EXTRACT RTL on architectures like ARM */
    return (val >> start) & ((1U << width) - 1);
}

/* Function to generate STRICT_LOW_PART pattern */
static void generate_strict_low_part(volatile uint32_t src) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    global_short = (uint16_t)src;
    global_byte = (uint8_t)src;
}

/* Function to generate SUBREG pattern */
static uint16_t generate_subreg(uint32_t val) {
    /* Cast to smaller type generates SUBREG */
    uint16_t result = (uint16_t)val;
    return result;
}

/* Function to generate complex MEM address pattern */
static int32_t generate_complex_mem(volatile int32_t *arr, int idx) {
    /* Complex addressing: arr[idx + offset] */
    /* XEXP(x, 0) will be the address expression (idx + offset) */
    int offset = 7;
    return arr[idx + offset];
}

/* Main function combining all patterns */
int main(int argc, char *argv[]) {
    uint32_t result = 0;
    int seed = 0;
    
    /* Use command line argument for variability */
    if (argc > 1) {
        seed = atoi(argv[1]) % 100;
    }
    
    /* Initialize array with pseudo-random values */
    for (int i = 0; i < 256; i++) {
        global_array[i] = (i * 37 + seed) & 0xFF;
    }
    
    /* 1. Generate ZERO_EXTRACT pattern */
    /* Use variable parameters to prevent constant folding */
    int start = (seed % 16) + 1;
    int width = (seed % 8) + 1;
    uint32_t extract_result = generate_zero_extract(global_val, start, width);
    result ^= extract_result;
    
    /* 2. Generate STRICT_LOW_PART pattern */
    generate_strict_low_part(global_val + seed);
    result ^= (global_short << 8) | global_byte;
    
    /* 3. Generate SUBREG pattern */
    uint16_t subreg_result = generate_subreg(global_val ^ seed);
    result ^= subreg_result;
    
    /* 4. Generate complex MEM address pattern */
    int idx = (seed * 13) % 200;  /* Ensure within bounds */
    int32_t mem_result = generate_complex_mem(global_array, idx);
    result ^= mem_result;
    
    /* Additional patterns in same basic block */
    
    /* Another ZERO_EXTRACT with different parameters */
    uint32_t val2 = global_val ^ 0x87654321;
    uint32_t extract2 = (val2 >> (width + 1)) & ((1U << (width + 2)) - 1);
    result += extract2;
    
    /* Another STRICT_LOW_PART via pointer */
    volatile uint32_t *ptr32 = &global_val;
    volatile uint16_t *ptr16 = (volatile uint16_t*)ptr32;
    *ptr16 = (uint16_t)(result & 0xFFFF);
    
    /* Another complex MEM access with pointer arithmetic */
    int32_t mem2 = global_ptr[idx * 2 + 3];
    result += mem2;
    
    /* Force all operations to be observable */
    printf("Result: 0x%08x\n", result);
    
    return (int)(result & 0xFF);
}
