/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile uint32_t global_val = 0x12345678;
volatile uint16_t global_short;
volatile uint8_t global_byte;
volatile int32_t global_array[256];
volatile int32_t *global_ptr = &global_array[0];

/* Function to ensure operations aren't optimized away */
static int use_result(int val) {
    volatile int sink = val;
    return sink;
}

/* Generate ZERO_EXTRACT pattern */
static uint32_t gen_zero_extract(volatile uint32_t src, int start, int width) {
    /* Non-constant shift and mask to prevent folding */
    uint32_t mask = (1U << width) - 1;
    return (src >> start) & mask;
}

/* Generate STRICT_LOW_PART pattern */
static void gen_strict_low_part(volatile uint32_t src) {
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    global_short = (uint16_t)src;
    global_byte = (uint8_t)src;
}

/* Generate SUBREG pattern */
static int32_t gen_subreg(volatile int32_t a) {
    /* Casts between different sizes generate SUBREG */
    int16_t b = (int16_t)a;
    int8_t c = (int8_t)a;
    return (int32_t)b + (int32_t)c;
}

/* Generate complex MEM address pattern */
static int32_t gen_complex_mem(volatile int32_t *arr, int idx) {
    /* Complex addressing: arr[idx + offset] */
    int offset = 7;
    return arr[idx + offset] + arr[idx * 2];
}

int main(int argc, char *argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize with pseudo-random but reproducible values */
    srand(seed);
    
    /* Initialize array with non-zero values */
    for (int i = 0; i < 256; i++) {
        global_array[i] = rand() % 1000;
    }
    
    /* Use command line or random for variability */
    int start_pos = (argc > 2) ? atoi(argv[2]) : 4;
    int width_val = (argc > 3) ? atoi(argv[3]) : 8;
    int array_idx = (argc > 4) ? atoi(argv[4]) : 32;
    
    /* Ensure values are in valid ranges */
    if (start_pos > 31) start_pos = 4;
    if (width_val > 16) width_val = 8;
    if (array_idx > 200) array_idx = 32;
    
    int result = 0;
    
    /* 1. Generate ZERO_EXTRACT pattern */
    uint32_t extract_result = gen_zero_extract(global_val, start_pos, width_val);
    result += (int)extract_result;
    
    /* 2. Generate STRICT_LOW_PART pattern */
    gen_strict_low_part(global_val + seed);
    
    /* 3. Generate SUBREG pattern */
    int32_t subreg_result = gen_subreg(global_val);
    result += subreg_result;
    
    /* 4. Generate complex MEM address pattern */
    int32_t mem_result = gen_complex_mem(global_array, array_idx);
    result += mem_result;
    
    /* Additional patterns in same basic block */
    {
        /* Combined operations to increase chance of hitting all patterns */
        volatile uint64_t big_val = 0x9876543210ABCDEFULL;
        
        /* Another ZERO_EXTRACT with 64-bit source */
        uint32_t extract2 = (big_val >> 16) & 0xFFF;
        result += extract2;
        
        /* Another STRICT_LOW_PART with pointer */
        volatile uint32_t *ptr = (uint32_t*)&big_val;
        global_short = *(uint16_t*)ptr;
        
        /* Memory with scaled index addressing */
        volatile int scale_arr[100];
        for (int i = 0; i < 100; i++) scale_arr[i] = i * 2;
        int idx2 = seed % 50;
        result += scale_arr[idx2 * 2 + 3];  /* Complex address: base + scaled index + offset */
    }
    
    /* Make result dependent on all operations */
    result = use_result(result);
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
