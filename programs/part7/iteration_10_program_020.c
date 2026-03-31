/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile unsigned int global_val = 0x12345678;
volatile unsigned short global_short = 0;
volatile int global_result = 0;
volatile int global_array[256];

/* Function to ensure operations aren't optimized away */
__attribute__((noinline))
int compute_with_patterns(int seed) {
    volatile int local_result = 0;
    
    /* 1. Generate ZERO_EXTRACT pattern */
    /* Bit-field extraction with variable width and position */
    volatile unsigned int extract_src = global_val ^ seed;
    volatile int start = (seed % 16) + 1;      /* Non-constant start position */
    volatile int width = (seed % 8) + 1;       /* Non-constant width */
    
    /* This should generate ZERO_EXTRACT RTL on architectures that support it */
    unsigned int extracted = (extract_src >> start) & ((1U << width) - 1);
    local_result ^= extracted;
    
    /* 2. Generate STRICT_LOW_PART pattern */
    /* Assignment to smaller type - may generate STRICT_LOW_PART */
    volatile int src_int = seed * 37;
    volatile short dest_short;
    
    /* Force assignment with type conversion */
    dest_short = (short)src_int;
    global_short = dest_short;  /* Store to volatile global */
    local_result += dest_short;
    
    /* 3. Generate SUBREG pattern */
    /* Cast between different-sized integer types */
    int32_t a = seed * 73;
    int16_t b = (int16_t)a;     /* Should generate SUBREG */
    local_result += b;
    
    /* Also test unsigned variants */
    uint32_t c = (uint32_t)seed * 97;
    uint8_t d = (uint8_t)c;     /* Another SUBREG */
    local_result += d;
    
    /* 4. Generate MEM_P with complex addressing */
    /* Array access with variable, computed index */
    volatile int* arr = (volatile int*)global_array;
    
    /* Initialize array with some values */
    for (int i = 0; i < 256; i++) {
        arr[i] = i * 3;
    }
    
    /* Complex addressing: base + (index * scale) + offset */
    int idx = (seed * 5) % 200;
    int offset = (seed % 16) * 4;
    
    /* This should generate MEM with complex address expression */
    int mem_value = arr[idx + offset];
    local_result ^= mem_value;
    
    /* Even more complex addressing: pointer arithmetic */
    volatile int* ptr = arr + idx;
    int mem_value2 = ptr[offset / 4];
    local_result += mem_value2;
    
    /* Additional MEM pattern: dereference with computation */
    int complex_idx = (idx * 3 + offset) & 0xFF;
    local_result += arr[complex_idx];
    
    return local_result;
}

/* Another function with different patterns */
__attribute__((noinline))
int more_complex_patterns(int seed) {
    volatile int result = 0;
    
    /* Use union for potential bitfield operations */
    union {
        volatile uint32_t full;
        struct {
            volatile uint16_t low;
            volatile uint16_t high;
        } parts;
    } data;
    
    data.full = seed * 0x9E3779B9;  /* Arbitrary constant */
    
    /* Access structure members - may generate SUBREG/MEM combinations */
    result += data.parts.low;
    result -= data.parts.high;
    
    /* Pointer chasing with complex addresses */
    volatile int* ptr_array[8];
    for (int i = 0; i < 8; i++) {
        ptr_array[i] = (volatile int*)&global_array[i * 32];
    }
    
    int ptr_idx = seed % 8;
    int elem_idx = (seed * 7) % 32;
    
    /* Nested array access with computation */
    result += ptr_array[ptr_idx][elem_idx];
    
    return result;
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    /* Use command line argument for variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * i;
    }
    
    int result1 = compute_with_patterns(seed);
    int result2 = more_complex_patterns(seed + 1);
    
    /* Combine results to ensure all computations are used */
    int final_result = result1 ^ result2;
    
    /* Print result to prevent optimization */
    printf("Result: %d (seed: %d)\n", final_result, seed);
    
    /* Additional loop to increase chance of scheduling analysis */
    for (int i = 0; i < 100; i++) {
        volatile int temp = compute_with_patterns(seed + i);
        final_result += temp;
    }
    
    printf("Final result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
