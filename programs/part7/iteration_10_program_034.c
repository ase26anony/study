/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent optimization and ensure RTL generation */
volatile unsigned int global_val = 0xDEADBEEF;
volatile unsigned int global_start = 8;
volatile unsigned int global_width = 12;
volatile int global_src = 0x12345678;
volatile short global_dest;
volatile int global_arr[256];
volatile int *global_ptr = (int*)global_arr;

/* Function to force generation of ZERO_EXTRACT pattern */
unsigned int zero_extract_operation(void) {
    /* Non-constant shift and mask to prevent folding */
    unsigned int val = global_val;
    unsigned int start = global_start;
    unsigned int width = global_width;
    
    /* This should generate ZERO_EXTRACT RTL on architectures that support it */
    unsigned int result = (val >> start) & ((1U << width) - 1);
    
    return result;
}

/* Function to force generation of STRICT_LOW_PART pattern */
short strict_low_part_operation(void) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    int src = global_src;
    short dest;
    
    /* Explicit cast to smaller type */
    dest = (short)src;
    global_dest = dest;  /* Store to volatile to ensure side effect */
    
    return dest;
}

/* Function to force generation of SUBREG pattern */
int16_t subreg_operation(int32_t a) {
    /* Cast between different integer sizes generates SUBREG */
    int16_t b = (int16_t)a;
    return b;
}

/* Function to force generation of complex MEM address pattern */
int complex_mem_operation(int idx) {
    /* Complex addressing mode: array + variable index + offset */
    volatile int *arr = global_arr;
    
    /* Initialize array with some values */
    for (int i = 0; i < 256; i++) {
        arr[i] = i * 3 + 1;
    }
    
    /* Variable index with offset - creates non-simple memory address */
    int value = arr[idx + 5];
    
    /* Even more complex: pointer arithmetic with multiple operations */
    int value2 = *(global_ptr + idx * 2);
    
    return value + value2;
}

/* Main function combining all patterns */
int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    /* Use command line argument for variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize with seed-dependent values to prevent constant propagation */
    global_start = (seed % 16) + 1;
    global_width = (seed % 8) + 4;
    global_src = seed * 0xABCD;
    
    /* Initialize array with seed-dependent values */
    for (int i = 0; i < 256; i++) {
        global_arr[i] = (i + seed) * 7919;  /* Prime number for variety */
    }
    
    int result = 0;
    
    /* Perform operations in sequence - all in same basic block */
    result += zero_extract_operation();
    result += strict_low_part_operation();
    result += subreg_operation(seed);
    result += complex_mem_operation(seed % 200);
    
    /* Additional combined operations to increase coverage probability */
    
    /* Combined bitfield extract with memory operand */
    volatile struct {
        unsigned int field1 : 8;
        unsigned int field2 : 12;
        unsigned int field3 : 12;
    } bitfield_struct;
    
    bitfield_struct.field1 = (seed >> 8) & 0xFF;
    bitfield_struct.field2 = zero_extract_operation() & 0xFFF;
    
    /* Access bitfield - may generate ZERO_EXTRACT with MEM */
    result += bitfield_struct.field2;
    
    /* Pointer casting for SUBREG with MEM */
    volatile char *char_ptr = (char*)&global_src;
    result += char_ptr[1];  /* Accesses byte within int - may use SUBREG */
    
    /* Complex addressing with multiple components */
    volatile int *complex_ptr = &global_arr[seed % 100] + (seed % 10);
    result += *complex_ptr;
    
    /* Print result to ensure all computations are observable */
    printf("Result: %d (seed: %d)\n", result, seed);
    
    return result & 0xFF;  /* Return non-zero to prevent dead code elimination */
}
