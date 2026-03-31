/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile unsigned int global_val = 0x12345678;
volatile int global_idx = 5;
volatile short global_short;
volatile int global_int;
volatile int global_array[100];

/* Force generation of ZERO_EXTRACT pattern */
unsigned int zero_extract_example(volatile unsigned int val, int start, int width) {
    /* Non-constant shift and mask to prevent folding */
    unsigned int mask = (1U << width) - 1;
    return (val >> start) & mask;
}

/* Force generation of STRICT_LOW_PART pattern */
void strict_low_part_example(volatile int src) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    global_short = (short)src;
    
    /* Another potential STRICT_LOW_PART case */
    volatile char *ptr = (volatile char *)&global_int;
    *ptr = (char)src;
}

/* Force generation of SUBREG pattern */
int32_t subreg_example(int32_t a) {
    /* Casts between different sizes generate SUBREG */
    int16_t b = (int16_t)a;
    int8_t c = (int8_t)a;
    return (int32_t)b + (int32_t)c;
}

/* Force generation of complex MEM address pattern */
int mem_complex_address(volatile int *arr, int idx) {
    /* Complex addressing: arr[idx + global_idx * 2] */
    return arr[idx + global_idx * 2];
}

/* Combined function with all patterns */
unsigned int combined_patterns(int seed) {
    unsigned int result = 0;
    
    /* 1. ZERO_EXTRACT pattern */
    /* Use non-constant parameters to prevent optimization */
    int start = (seed % 16) + 1;
    int width = (seed % 8) + 1;
    result ^= zero_extract_example(global_val, start, width);
    
    /* 2. STRICT_LOW_PART pattern */
    strict_low_part_example(seed * 0xABCD);
    
    /* 3. SUBREG pattern */
    result += subreg_example(seed * 0x1234);
    
    /* 4. Complex MEM address pattern */
    /* Initialize array with some values */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * seed;
    }
    
    /* Use variable index for complex addressing */
    int idx = (seed * 37) % 50;
    result += mem_complex_address(global_array, idx);
    
    /* Additional MEM pattern with pointer arithmetic */
    volatile int *ptr = global_array + idx + global_idx;
    result += *ptr;
    
    /* Another potential ZERO_EXTRACT with bitfields */
    struct bitfield_struct {
        unsigned int field1 : 4;
        unsigned int field2 : 8;
        unsigned int field3 : 12;
    };
    
    volatile struct bitfield_struct bf;
    bf.field1 = (seed >> 4) & 0xF;
    bf.field2 = (seed >> 8) & 0xFF;
    bf.field3 = (seed >> 12) & 0xFFF;
    
    /* Access bitfield - may generate ZERO_EXTRACT */
    result += bf.field2;
    
    return result;
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    /* Use command line argument for variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize global variables with seed-dependent values */
    global_val = seed * 0x98765432;
    global_idx = (seed % 20) + 1;
    
    /* Execute combined patterns multiple times to increase coverage chance */
    unsigned int final_result = 0;
    for (int i = 0; i < 10; i++) {
        final_result += combined_patterns(seed + i);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: 0x%08X\n", final_result);
    
    return 0;
}
