/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent optimization and force RTL generation */
volatile unsigned int global_val = 0xDEADBEEF;
volatile int global_idx = 0;
volatile int global_start = 4;
volatile int global_width = 8;

/* Arrays for memory access patterns */
volatile int mem_array[256];
volatile short short_array[256];

/* Function to force generation of ZERO_EXTRACT RTL */
static unsigned int force_zero_extract(volatile unsigned int val, 
                                       volatile int start, 
                                       volatile int width) {
    /* Non-constant shift and mask to prevent folding */
    unsigned int shifted = val >> start;
    unsigned int mask = (1U << width) - 1;
    return shifted & mask;
}

/* Function to force generation of STRICT_LOW_PART RTL */
static void force_strict_low_part(volatile int src) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    volatile short dest;
    dest = (short)src;
    short_array[0] = dest;  /* Use result to prevent elimination */
}

/* Function with complex memory addressing */
static int force_complex_mem(volatile int idx) {
    /* Complex addressing: array base + variable index + constant offset */
    return mem_array[idx + 5] + mem_array[idx * 2];
}

/* Function demonstrating SUBREG usage */
static int32_t force_subreg(volatile int32_t a) {
    /* Multiple casts between different sizes */
    int16_t b = (int16_t)a;
    int8_t c = (int8_t)b;
    return (int32_t)c;  /* Sign extension back to 32-bit */
}

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Initialize with pseudo-random but reproducible values */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        mem_array[i] = rand() % 1000;
        short_array[i] = (short)(rand() % 1000);
    }
    
    /* 1. Generate ZERO_EXTRACT pattern */
    global_start = (rand() % 16) + 1;  /* Non-zero, non-constant */
    global_width = (rand() % 8) + 1;   /* Non-zero, non-constant */
    unsigned int extract_result = force_zero_extract(global_val, 
                                                     global_start, 
                                                     global_width);
    result ^= extract_result;
    
    /* 2. Generate STRICT_LOW_PART pattern */
    force_strict_low_part(rand());
    result ^= short_array[0];
    
    /* 3. Generate SUBREG patterns through multiple casts */
    int32_t subreg_result = force_subreg(rand());
    result ^= subreg_result;
    
    /* 4. Generate complex MEM addressing pattern */
    global_idx = rand() % 100;  /* Variable index */
    int mem_result = force_complex_mem(global_idx);
    result ^= mem_result;
    
    /* 5. Additional patterns in same basic block */
    /* Combined operation that might generate multiple RTL patterns */
    volatile uint64_t big_val = 0x123456789ABCDEF0ULL;
    volatile uint32_t part1 = (uint32_t)(big_val >> 32);  /* High part */
    volatile uint16_t part2 = (uint16_t)part1;            /* Strict low part */
    volatile uint8_t part3 = (uint8_t)part2;              /* Another strict low part */
    
    result ^= part1 ^ part2 ^ part3;
    
    /* 6. Bitfield operations that might generate ZERO_EXTRACT */
    struct bitfield {
        unsigned int a : 5;
        unsigned int b : 7;
        unsigned int c : 10;
    } bf;
    
    volatile unsigned int *ptr = (unsigned int*)&bf;
    *ptr = rand();
    
    /* Extract bitfield with variable position */
    volatile int shift_a = 0;
    volatile int shift_b = 5;
    unsigned int bf_extract = (bf.b << shift_a) | (bf.c << shift_b);
    result ^= bf_extract;
    
    /* 7. Pointer arithmetic for complex MEM addresses */
    volatile int *mem_ptr = (volatile int*)mem_array;
    mem_ptr += global_idx * 3 + 7;  /* Complex address calculation */
    result ^= *mem_ptr;
    
    printf("Result: %d\n", result);
    return result & 1;  /* Return non-zero to ensure all code is used */
}
