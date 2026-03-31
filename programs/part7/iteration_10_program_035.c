/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile unsigned int global_val = 0xDEADBEEF;
volatile unsigned short global_short = 0;
volatile int global_array[256];
volatile int global_result = 0;

/* Function to ensure operations aren't optimized away */
__attribute__((noinline)) 
static int compute_index(int seed) {
    return (seed * 1103515245 + 12345) & 255;
}

/* Function containing the target RTL patterns */
__attribute__((noinline))
static int generate_rtl_patterns(int seed) {
    volatile int local_result = 0;
    
    /* 1. Generate ZERO_EXTRACT pattern */
    /* Bit-field extract with variable width and position */
    volatile unsigned int extract_src = global_val ^ seed;
    volatile int start_pos = (seed % 16) + 1;  /* Non-constant start */
    volatile int width = (seed % 8) + 1;       /* Non-constant width */
    
    /* This should generate ZERO_EXTRACT RTL on architectures that support it */
    unsigned int extracted = (extract_src >> start_pos) & ((1U << width) - 1);
    local_result ^= extracted;
    
    /* 2. Generate STRICT_LOW_PART pattern */
    /* Assignment to smaller type - may generate STRICT_LOW_PART */
    volatile int src_int = seed * 3;
    volatile short dest_short;
    dest_short = (short)src_int;  /* Potential STRICT_LOW_PART */
    global_short = dest_short;     /* Force side effect */
    local_result += dest_short;
    
    /* 3. SUBREG is generated implicitly by the cast above and below */
    /* Explicit cast between different-sized types */
    int32_t wide_val = seed * 5;
    int16_t narrow_val = (int16_t)wide_val;  /* Generates SUBREG */
    local_result += narrow_val;
    
    /* 4. Complex MEM address pattern */
    /* Array access with variable, computed index */
    int idx = compute_index(seed);
    
    /* Complex addressing: base + scaled index + offset */
    /* Should generate MEM with non-simple address expression */
    int mem_val = global_array[idx * 2 + 3];  /* Complex address */
    local_result += mem_val;
    
    /* Another complex MEM access with pointer arithmetic */
    volatile int *ptr = &global_array[100];
    int mem_val2 = ptr[idx - 50];  /* Another complex address */
    local_result ^= mem_val2;
    
    return local_result;
}

/* Additional function to create more patterns in different contexts */
__attribute__((noinline))
static void more_patterns(int seed) {
    /* More bit-field operations for ZERO_EXTRACT */
    volatile struct {
        unsigned int field1 : 4;
        unsigned int field2 : 8;
        unsigned int field3 : 12;
    } bitfield;
    
    bitfield.field1 = (seed >> 0) & 0xF;
    bitfield.field2 = (seed >> 4) & 0xFF;
    bitfield.field3 = (seed >> 12) & 0xFFF;
    
    /* Extract from bitfield - may generate ZERO_EXTRACT */
    unsigned int from_bitfield = bitfield.field2;
    global_result += from_bitfield;
    
    /* Memory access with multiple indexing levels */
    volatile int matrix[10][10];
    int i = seed % 10;
    int j = (seed * 3) % 10;
    int matrix_val = matrix[i][j];  /* Complex MEM address: base + i*stride + j */
    global_result ^= matrix_val;
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    /* Use command line argument for variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize array with pseudo-random values */
    for (int i = 0; i < 256; i++) {
        global_array[i] = (i * 1103515245 + 12345) & 0xFFFF;
    }
    
    /* Generate the target RTL patterns */
    int result1 = generate_rtl_patterns(seed);
    more_patterns(seed);
    
    /* Combine results to ensure all computations are used */
    int final_result = result1 + global_result + global_short;
    
    /* Print result to prevent optimization */
    printf("Result: %d (seed: %d)\n", final_result, seed);
    
    return final_result != 0 ? 0 : 1;
}
