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

/* Function to ensure operations aren't optimized away */
__attribute__((noinline)) 
static int compute_result(int seed) {
    volatile int result = 0;
    
    /* 1. Generate ZERO_EXTRACT pattern */
    /* Bit-field extraction with variable width and position */
    volatile unsigned int extract_src = global_val ^ seed;
    volatile int start = (seed % 16) + 1;      /* Non-constant start position */
    volatile int width = (seed % 8) + 1;       /* Non-constant width */
    
    /* This should generate ZERO_EXTRACT RTL on architectures that support it */
    unsigned int extracted = (extract_src >> start) & ((1U << width) - 1);
    result ^= extracted;
    
    /* 2. Generate STRICT_LOW_PART pattern */
    /* Assignment to smaller type - may generate STRICT_LOW_PART */
    volatile int src_int = seed * 37;
    volatile short dest_short;
    
    /* Force assignment with truncation */
    dest_short = (short)src_int;
    global_short = dest_short;
    result += dest_short;
    
    /* 3. Generate SUBREG pattern */
    /* Cast between different-sized integer types */
    int32_t a = seed * 73;
    int16_t b = (int16_t)a;          /* Should generate SUBREG */
    int32_t c = (int32_t)b;          /* Another SUBREG */
    result += c;
    
    /* 4. Generate complex MEM address pattern */
    /* Array access with variable, computed index */
    volatile int* arr = (volatile int*)global_array;
    
    /* Initialize array with some values */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * seed;
    }
    
    /* Complex addressing: base + (index * scale) + displacement */
    int idx = global_idx + (seed % 50);
    
    /* This should generate MEM with complex address expression */
    int mem_value = arr[idx + 3];    /* MEM with register+offset addressing */
    result += mem_value;
    
    /* More complex addressing: arr[(idx*2) + 5] */
    int mem_value2 = arr[(idx * 2) + 5];
    result ^= mem_value2;
    
    /* Pointer arithmetic for another MEM pattern */
    volatile int* ptr = arr + idx;
    int mem_value3 = ptr[global_idx];
    result += mem_value3;
    
    return result;
}

/* Another function with different patterns */
__attribute__((noinline))
static int bitfield_operations(int seed) {
    volatile int result = 0;
    
    /* Use union for bit-field operations that might generate ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low16 : 16;
            unsigned int high16 : 16;
        } parts;
    } u;
    
    u.full = seed * 0xABCD;
    
    /* Access bit-fields - may generate ZERO_EXTRACT */
    volatile unsigned int low_part = u.parts.low16;
    volatile unsigned int high_part = u.parts.high16;
    
    result = low_part + (high_part << 16);
    
    /* Mixed-size operations for SUBREG */
    volatile char char_val = (seed >> 3) & 0xFF;
    volatile short short_val = char_val * 2;
    volatile int int_val = short_val + seed;
    
    result += int_val;
    
    return result;
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    /* Use command line argument for variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * (seed + 1);
    }
    
    /* Perform operations that should generate target RTL patterns */
    int result1 = compute_result(seed);
    int result2 = bitfield_operations(seed);
    
    /* Combine results to ensure all computations are used */
    int final_result = result1 + result2 * 3;
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", final_result);
    
    /* Additional loop to increase chance of scheduling analysis */
    for (int i = 0; i < 10; i++) {
        /* Mix in more operations */
        volatile int temp = compute_result(seed + i);
        final_result ^= temp;
        
        /* More MEM accesses with complex addressing */
        volatile int* ptr = (volatile int*)global_array;
        int idx = (seed * i) % 90;
        final_result += ptr[idx + (i % 5)];  /* Complex address */
        
        /* More size conversions for SUBREG */
        volatile long long big_val = seed * 1000LL;
        volatile int small_val = (int)big_val;  /* SUBREG */
        final_result += small_val;
    }
    
    printf("Final result: %d\n", final_result);
    
    return 0;
}
