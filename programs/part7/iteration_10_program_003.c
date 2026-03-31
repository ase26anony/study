/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile uint32_t global_val = 0x12345678;
volatile uint16_t global_short;
volatile uint8_t global_byte;
volatile int32_t global_array[256];

/* Function to ensure operations aren't optimized away */
static int use_result(int val) {
    return val;
}

/* Main function with operations targeting specific RTL expressions */
int main(int argc, char *argv[]) {
    /* Use command-line argument for variability but reproducibility */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    int result = 0;
    
    /* 1. Generate ZERO_EXTRACT RTL pattern */
    /* Bit-field extraction with variable width and position */
    volatile uint32_t extract_src = global_val ^ rand();
    volatile int start = (rand() % 16) + 1;  /* Non-constant start position */
    volatile int width = (rand() % 8) + 1;   /* Non-constant width */
    
    /* This should generate ZERO_EXTRACT when compiled to RTL */
    uint32_t extracted = (extract_src >> start) & ((1U << width) - 1);
    result += extracted;
    
    /* 2. Generate STRICT_LOW_PART RTL pattern */
    /* Assignment to smaller type - may generate strict_low_part */
    volatile uint32_t src32 = global_val + rand();
    volatile uint16_t dest16;
    
    /* Explicit cast and assignment to volatile smaller type */
    dest16 = (uint16_t)src32;
    global_short = dest16;  /* Force memory store */
    result += dest16;
    
    /* 3. SUBREG is very common - any size change operation */
    uint32_t reg32 = global_val;
    uint16_t reg16 = (uint16_t)reg32;  /* Likely generates SUBREG */
    uint8_t reg8 = (uint8_t)reg16;     /* Another SUBREG */
    result += reg8;
    
    /* 4. Complex MEM address with register + offset */
    /* Variable index with offset for complex addressing mode */
    volatile int idx = rand() % 200;
    volatile int offset = 5;
    
    /* Complex memory access: array base + variable index + constant offset */
    int mem_val = global_array[idx + offset];
    result += mem_val;
    
    /* Additional MEM pattern with pointer arithmetic */
    volatile int *ptr = &global_array[100];
    ptr += (rand() % 10);
    result += *ptr;
    
    /* Combine with bit-field in memory */
    volatile struct {
        uint32_t field1 : 8;
        uint32_t field2 : 12;
        uint32_t field3 : 4;
    } bitfield_struct;
    
    bitfield_struct.field1 = rand() & 0xFF;
    bitfield_struct.field2 = (rand() & 0xFFF);
    result += bitfield_struct.field2;
    
    /* Force all operations to be used */
    result = use_result(result);
    
    printf("Result: %d (seed: %d)\n", result, seed);
    
    /* Additional loop to increase chance of scheduling analysis */
    for (int i = 0; i < 10; i++) {
        /* Mix operations in loop body */
        uint32_t loop_val = global_array[i];
        uint16_t loop_short = (uint16_t)loop_val;
        global_array[i + 10] = loop_short + extracted;
    }
    
    return result & 0xFF;
}
