/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent optimization and ensure RTL generation */
volatile unsigned int global_val = 0xDEADBEEF;
volatile int global_idx = 5;
volatile unsigned short global_short = 0x1234;
volatile int global_width = 8;
volatile int global_start = 4;

/* Volatile array for complex memory addressing */
volatile int mem_array[256];

/* Function to ensure operations aren't optimized away */
static int use_result(int val) {
    volatile int sink = val;
    return sink;
}

/* Main function with operations targeting specific RTL expressions */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Initialize with pseudo-random but reproducible values */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Initialize array with values */
    for (int i = 0; i < 256; i++) {
        mem_array[i] = rand() % 1000;
    }
    
    /* 1. Generate ZERO_EXTRACT RTL pattern */
    /* Bit-field extraction with variable width and start */
    volatile unsigned int extract_src = global_val;
    volatile int width = global_width + (seed % 4);  /* Non-constant */
    volatile int start = global_start + (seed % 3);  /* Non-constant */
    
    /* This should generate ZERO_EXTRACT when compiled to RTL */
    unsigned int extracted = (extract_src >> start) & ((1U << width) - 1);
    result += use_result(extracted);
    
    /* 2. Generate STRICT_LOW_PART RTL pattern */
    /* Assignment to smaller type - may generate strict_low_part */
    volatile int src_int = 0xABCD1234;
    volatile short dest_short;
    
    /* Explicit cast and assignment to volatile short */
    dest_short = (short)src_int;
    result += use_result(dest_short);
    
    /* Another potential STRICT_LOW_PART case */
    volatile char dest_char;
    dest_char = (char)(src_int + seed);
    result += use_result(dest_char);
    
    /* 3. Generate SUBREG RTL pattern (very common) */
    /* Casts between different sized integers generate SUBREG */
    int32_t a = 0x12345678;
    int16_t b = (int16_t)a;          /* SUBREG for low 16 bits */
    int8_t c = (int8_t)(a >> 16);    /* Another SUBREG */
    
    result += use_result(b);
    result += use_result(c);
    
    /* 4. Generate MEM_P with complex addressing */
    /* Array access with variable index and offset */
    volatile int idx = global_idx + (seed % 50);
    
    /* Complex addressing: array + variable index + constant offset */
    int mem_value = mem_array[idx + 10];  /* Should generate MEM with PLUS address */
    
    /* More complex addressing with computation */
    int mem_value2 = mem_array[(idx * 3) % 256];
    
    /* Even more complex: nested array with pointer arithmetic */
    volatile int *ptr = (volatile int *)mem_array;
    ptr += idx;
    int mem_value3 = *ptr;
    
    result += use_result(mem_value);
    result += use_result(mem_value2);
    result += use_result(mem_value3);
    
    /* Combine all results to ensure all operations are used */
    int final_result = result ^ seed;
    
    printf("Result: %d (seed: %d)\n", final_result, seed);
    
    /* Additional loop to increase chance of scheduling analysis */
    for (int i = 0; i < 10; i++) {
        /* Mix different operations in loop */
        volatile int temp = mem_array[i] & ((1U << (width % 8)) - 1);
        final_result += (short)temp;  /* Potential STRICT_LOW_PART */
        final_result += mem_array[temp % 256];  /* Complex MEM access */
    }
    
    printf("Final result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
