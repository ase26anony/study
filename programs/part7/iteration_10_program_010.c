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
static uint32_t generate_zero_extract(volatile uint32_t val, int shift, int width) {
    /* Non-constant shift and width to prevent folding */
    return (val >> shift) & ((1U << width) - 1);
}

/* Function to generate STRICT_LOW_PART pattern */
static void generate_strict_low_part(volatile uint32_t src) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    global_short = (uint16_t)src;
    global_byte = (uint8_t)src;
}

/* Function to generate SUBREG pattern */
static uint16_t generate_subreg(volatile uint32_t val) {
    /* Cast between different sizes generates SUBREG */
    uint16_t low = (uint16_t)val;
    uint16_t high = (uint16_t)(val >> 16);
    return low + high; /* Combine to prevent dead code elimination */
}

/* Function to generate complex MEM address pattern */
static int32_t generate_complex_mem(volatile int32_t *arr, int idx) {
    /* Variable index with offset creates complex addressing */
    return arr[idx + 5] + arr[idx * 2] + arr[idx % 16];
}

int main(int argc, char *argv[]) {
    int i, result = 0;
    uint32_t temp;
    
    /* Use command line argument for variability */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Initialize array with pseudo-random values */
    for (i = 0; i < 256; i++) {
        global_array[i] = rand() % 1000;
    }
    
    /* 1. Generate ZERO_EXTRACT pattern */
    /* Use variable shift/width to prevent constant folding */
    int shift = (seed % 8) + 1;
    int width = (seed % 16) + 1;
    temp = generate_zero_extract(global_val, shift, width);
    result += temp;
    
    /* 2. Generate STRICT_LOW_PART pattern */
    generate_strict_low_part(global_val + seed);
    result += global_short + global_byte;
    
    /* 3. Generate SUBREG pattern */
    temp = generate_subreg(global_val ^ seed);
    result += temp;
    
    /* 4. Generate complex MEM address pattern */
    int idx = (seed * 3) % 200;
    temp = generate_complex_mem(global_array, idx);
    result += temp;
    
    /* Additional patterns in a loop to increase coverage probability */
    for (i = 0; i < 10; i++) {
        /* Mixed operations in same basic block */
        uint32_t val = global_val + i;
        
        /* ZERO_EXTRACT with different parameters */
        int s = (i % 7) + 1;
        int w = (i % 8) + 8;
        result += generate_zero_extract(val, s, w);
        
        /* STRICT_LOW_PART assignment */
        global_short = (uint16_t)(val * 3);
        
        /* SUBREG through pointer casting */
        uint32_t *ptr = &global_val;
        uint16_t *short_ptr = (uint16_t *)ptr;
        result += *short_ptr;
        
        /* Complex memory access with pointer arithmetic */
        int offset = (i * 7) % 100;
        result += global_ptr[offset + 3];
    }
    
    /* Use bitfields for additional ZERO_EXTRACT opportunities */
    struct bitfield_struct {
        volatile unsigned int field1 : 4;
        volatile unsigned int field2 : 8;
        volatile unsigned int field3 : 12;
    } bf;
    
    bf.field1 = (seed >> 4) & 0xF;
    bf.field2 = (seed >> 8) & 0xFF;
    bf.field3 = (seed >> 12) & 0xFFF;
    result += bf.field1 + bf.field2 + bf.field3;
    
    /* Final output to prevent optimization */
    printf("Result: %d (seed: %d)\n", result, seed);
    
    return result != 0 ? 0 : 1;
}
