/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent optimization and ensure RTL generation */
volatile uint32_t global_val = 0x12345678;
volatile uint32_t global_array[256];
volatile uint16_t global_short;
volatile uint8_t global_byte;

/* Function to generate ZERO_EXTRACT pattern */
uint32_t generate_zero_extract(volatile uint32_t val, int start, int width) {
    /* Non-constant shift and mask to prevent folding */
    uint32_t mask = (1U << width) - 1;
    return (val >> start) & mask;
}

/* Function to generate STRICT_LOW_PART pattern */
void generate_strict_low_part(volatile uint32_t src) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    global_short = (uint16_t)src;
    global_byte = (uint8_t)src;
}

/* Function to generate SUBREG pattern */
uint16_t generate_subreg(volatile uint32_t val) {
    /* Cast to smaller type generates SUBREG */
    return (uint16_t)val;
}

/* Function to generate complex MEM address pattern */
uint32_t generate_complex_mem(volatile uint32_t *arr, int idx) {
    /* Complex addressing: arr[idx + offset] */
    int offset = idx * 2;
    return arr[idx + offset + 3];  /* Non-simple address expression */
}

int main(int argc, char *argv[]) {
    /* Use command-line argument for variability but reproducibility */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    srand(seed);
    
    uint32_t result = 0;
    
    /* 1. Generate ZERO_EXTRACT pattern */
    /* Use variable shift and width to prevent constant folding */
    int shift = (rand() % 16) + 1;      /* 1-16 */
    int width = (rand() % 8) + 1;       /* 1-8 */
    
    result ^= generate_zero_extract(global_val, shift, width);
    
    /* 2. Generate STRICT_LOW_PART pattern */
    generate_strict_low_part(global_val);
    result ^= global_short;
    result ^= global_byte;
    
    /* 3. Generate SUBREG pattern */
    result ^= generate_subreg(global_val);
    
    /* 4. Generate complex MEM address pattern */
    /* Initialize array with pseudo-random values */
    for (int i = 0; i < 256; i++) {
        global_array[i] = rand();
    }
    
    int index = rand() % 100;  /* Variable index */
    result ^= generate_complex_mem(global_array, index);
    
    /* Additional patterns that might help on specific architectures */
    
    /* ARM-style bitfield operations (may generate ZERO_EXTRACT) */
    #if defined(__arm__) || defined(__aarch64__)
    volatile uint32_t arm_val = 0xABCD1234;
    uint32_t arm_extract = (arm_val >> 8) & 0xFFF;  /* 12-bit extract */
    result ^= arm_extract;
    #endif
    
    /* x86 BMI2 style (may generate ZERO_EXTRACT with bzhi) */
    #if defined(__x86_64__) || defined(__i386__)
    volatile uint64_t x86_val = 0x123456789ABCDEF0ULL;
    uint64_t x86_extract = x86_val & ((1ULL << 32) - 1);
    result ^= (uint32_t)x86_extract;
    #endif
    
    /* Force multiple SUBREGs through type punning */
    union {
        uint32_t full;
        struct {
            uint16_t low;
            uint16_t high;
        } parts;
    } pun;
    
    pun.full = global_val;
    result ^= pun.parts.low;   /* May generate SUBREG */
    result ^= pun.parts.high;  /* May generate SUBREG */
    
    /* Complex memory addressing with multiple components */
    volatile uint32_t *ptr = global_array + 16;
    result ^= ptr[index * 3 - 5];  /* Complex address calculation */
    
    /* Print result to prevent dead code elimination */
    printf("Result: 0x%08x\n", result);
    
    return 0;
}
