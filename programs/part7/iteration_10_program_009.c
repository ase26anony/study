/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile uint32_t global_val = 0x12345678;
volatile uint16_t global_short;
volatile uint8_t global_byte;
volatile int32_t global_array[256];
volatile int32_t *global_ptr = global_array;

/* Function to force generation of ZERO_EXTRACT RTL */
uint32_t zero_extract_operation(uint32_t val, int start, int width) {
    /* Non-constant shift and mask to prevent folding */
    volatile uint32_t temp = val;
    /* This should generate ZERO_EXTRACT when compiled to RTL */
    uint32_t result = (temp >> start) & ((1U << width) - 1);
    return result;
}

/* Function to force generation of STRICT_LOW_PART RTL */
void strict_low_part_operation(uint32_t src) {
    volatile uint16_t dest;
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    dest = (uint16_t)src;
    global_short = dest;
}

/* Function to force generation of SUBREG RTL */
int32_t subreg_operation(int64_t large_val) {
    /* Cast between different sizes generates SUBREG */
    int32_t truncated = (int32_t)large_val;
    return truncated;
}

/* Function with complex memory addressing (MEM_P with non-simple address) */
int32_t complex_mem_access(int idx) {
    /* Variable index with offset creates complex addressing mode */
    volatile int32_t value = global_array[idx * 2 + 5];
    
    /* Pointer arithmetic also creates complex addresses */
    volatile int32_t *ptr = global_ptr + idx + 3;
    return value + *ptr;
}

/* Main function combining all patterns */
int main(int argc, char *argv[]) {
    uint32_t result = 0;
    int seed = 0;
    
    /* Use command line argument for variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = 12345;
    }
    
    /* Initialize with pseudo-random but reproducible values */
    srand(seed);
    
    /* Initialize array with some data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = rand() % 1000;
    }
    
    /* 1. Generate ZERO_EXTRACT pattern */
    /* Use non-constant parameters to prevent optimization */
    int start_bit = (seed % 16) + 1;
    int width = (seed % 8) + 1;
    
    uint32_t extract_result = zero_extract_operation(global_val, start_bit, width);
    result ^= extract_result;
    
    /* 2. Generate STRICT_LOW_PART pattern */
    uint32_t src_val = (uint32_t)rand();
    strict_low_part_operation(src_val);
    result += global_short;
    
    /* 3. Generate SUBREG pattern */
    int64_t large_val = ((int64_t)rand() << 32) | rand();
    int32_t subreg_result = subreg_operation(large_val);
    result += subreg_result;
    
    /* 4. Generate complex MEM access pattern */
    int idx = rand() % 100;
    int32_t mem_result = complex_mem_access(idx);
    result += mem_result;
    
    /* Additional combination to ensure all operations are used */
    /* Mix in some bit-field operations on memory */
    volatile struct {
        uint32_t field1 : 8;
        uint32_t field2 : 12;
        uint32_t field3 : 12;
    } bitfield;
    
    bitfield.field1 = extract_result & 0xFF;
    bitfield.field2 = (mem_result >> 4) & 0xFFF;
    bitfield.field3 = (subreg_result >> 8) & 0xFFF;
    
    result += bitfield.field1 + bitfield.field2 + bitfield.field3;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %u (0x%08x)\n", result, result);
    
    return (int)(result % 256);
}
