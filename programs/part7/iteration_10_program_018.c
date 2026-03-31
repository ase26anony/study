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

/* Function to ensure operations aren't optimized away */
static int use_result(int val) {
    volatile int sink = val;
    return sink;
}

/* Function containing the target RTL patterns */
int generate_rtl_patterns(int seed) {
    volatile int result = 0;
    
    /* 1. Generate ZERO_EXTRACT pattern */
    /* Bit-field extraction with variable parameters */
    volatile unsigned int extract_src = global_val ^ seed;
    volatile unsigned int start = global_start + (seed & 0x3);  /* 4-7 */
    volatile unsigned int width = global_width + (seed & 0x7);  /* 8-15 */
    
    /* This should generate ZERO_EXTRACT RTL on architectures that support it */
    unsigned int extracted = (extract_src >> start) & ((1U << width) - 1);
    result ^= extracted;
    
    /* 2. Generate STRICT_LOW_PART pattern */
    /* Assignment to smaller type - may generate strict_low_part */
    volatile int32_t src32 = seed * 0xABCD;
    volatile int16_t dest16;
    
    /* Explicit cast and assignment to volatile smaller type */
    dest16 = (int16_t)src32;
    result += dest16;
    
    /* 3. Generate SUBREG pattern */
    /* Multiple casts between different integer sizes */
    int64_t wide_val = (int64_t)seed * 0x123456789ABCDEFLL;
    int32_t narrowed32 = (int32_t)wide_val;      /* May generate SUBREG */
    int16_t narrowed16 = (int16_t)narrowed32;    /* Another SUBREG */
    int8_t narrowed8 = (int8_t)narrowed16;       /* Another SUBREG */
    
    result += narrowed32 + narrowed16 + narrowed8;
    
    /* 4. Generate complex MEM address pattern */
    /* Array access with variable, computed index */
    volatile int* volatile ptr = (volatile int*)mem_array;
    
    /* Complex addressing: base + (index * scale) + offset */
    int complex_idx = (global_idx * seed) & 0xFF;
    
    /* This should generate MEM with complex address expression */
    int mem_value = ptr[complex_idx + 3];
    result ^= mem_value;
    
    /* Additional memory pattern with pointer arithmetic */
    volatile short* short_ptr = (volatile short*)short_array;
    int offset_idx = (seed * 7) & 0xFF;
    
    /* Pointer arithmetic in the subscript */
    short short_value = short_ptr[offset_idx * 2];
    result += short_value;
    
    /* Mix in some arithmetic to create data dependencies */
    result = (result * 0x9E3779B9) ^ seed;
    
    return use_result(result);
}

/* Main function with initialization and repeated pattern generation */
int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize arrays with pseudo-random but deterministic values */
    for (int i = 0; i < 256; i++) {
        mem_array[i] = (i * 0x1234 + seed) ^ 0xABCDEF;
        short_array[i] = (short)((i * 0x5678 + seed) & 0xFFFF);
    }
    
    /* Generate patterns multiple times to increase coverage probability */
    int final_result = 0;
    for (int i = 0; i < 100; i++) {
        int iteration_seed = seed + i * 0x1000;
        int partial = generate_rtl_patterns(iteration_seed);
        final_result ^= partial;
        
        /* Modify globals slightly each iteration */
        global_val = (global_val * 0x1234567) ^ iteration_seed;
        global_idx = (global_idx + 1) & 0xF;
    }
    
    /* Ensure result is used */
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
