/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent optimization and ensure RTL generation */
volatile unsigned int global_val = 0x12345678;
volatile int global_idx = 5;
volatile unsigned int global_start = 4;
volatile unsigned int global_width = 8;

/* Arrays for memory access patterns */
volatile int mem_array[256];
volatile short short_array[256];

/* Function to force generation of ZERO_EXTRACT pattern */
unsigned int generate_zero_extract(volatile unsigned int val, 
                                   volatile unsigned int start, 
                                   volatile unsigned int width) {
    /* This should generate ZERO_EXTRACT RTL:
     * Extract width bits starting at position start, zero-extend result */
    unsigned int mask = ((1U << width) - 1);
    return (val >> start) & mask;
}

/* Function to force generation of STRICT_LOW_PART pattern */
void generate_strict_low_part(volatile int src) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    volatile short dest;
    dest = (short)src;  /* Low 16 bits only, high bits preserved in register */
    
    /* Use the result to prevent dead code elimination */
    short_array[0] = dest;
}

/* Function to force generation of SUBREG pattern */
int32_t generate_subreg(int32_t a) {
    /* Cast to smaller type generates SUBREG */
    int16_t b = (int16_t)a;
    /* Cast back generates another SUBREG */
    return (int32_t)b;
}

/* Function with complex memory addressing for MEM_P pattern */
int generate_complex_mem_access(volatile int idx) {
    /* Complex addressing: base + scaled index + offset */
    int value = mem_array[idx * 2 + 3];
    
    /* Even more complex: pointer arithmetic */
    volatile int *ptr = &mem_array[0];
    ptr += idx;
    value += *ptr;
    
    return value;
}

/* Main function combining all patterns */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Initialize with pseudo-random but reproducible values */
    unsigned int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        mem_array[i] = rand() % 1000;
        short_array[i] = (short)(rand() % 1000);
    }
    
    /* 1. Generate ZERO_EXTRACT pattern */
    unsigned int extract_result = generate_zero_extract(
        global_val ^ seed, 
        global_start + (seed & 0x3),  /* Vary start position */
        global_width + (seed & 0x7)    /* Vary width */
    );
    result += extract_result;
    
    /* 2. Generate STRICT_LOW_PART pattern */
    generate_strict_low_part(global_val + seed);
    
    /* 3. Generate SUBREG pattern */
    int32_t subreg_result = generate_subreg(global_val * seed);
    result += subreg_result;
    
    /* 4. Generate complex MEM access pattern */
    int mem_result = generate_complex_mem_access(
        global_idx + (seed % 50)
    );
    result += mem_result;
    
    /* Additional patterns in a loop to increase coverage probability */
    for (int i = 0; i < 10; i++) {
        /* Mixed operations in single basic block */
        volatile int temp = mem_array[i];
        
        /* Another ZERO_EXTRACT with different parameters */
        unsigned int ext = (temp >> (i & 0x7)) & ((1U << 8) - 1);
        result += ext;
        
        /* Another STRICT_LOW_PART */
        volatile short s = (short)temp;
        short_array[i] = s;
        
        /* Another complex MEM access */
        result += mem_array[temp & 0xFF];
    }
    
    /* Use result to prevent optimization */
    printf("Result: %d (seed: %u)\n", result, seed);
    
    return result != 0 ? 0 : 1;
}
