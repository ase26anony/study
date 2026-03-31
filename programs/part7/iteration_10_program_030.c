/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile unsigned int global_val = 0x12345678;
volatile short global_short;
volatile int global_int;
volatile int global_array[256];
volatile int *global_ptr = &global_array[0];

/* Function to force generation of specific RTL patterns */
int generate_rtl_patterns(int seed) {
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
    volatile int src_int = seed * 3;
    volatile short dest_short;
    
    /* Force assignment with truncation */
    dest_short = (short)src_int;
    global_short = dest_short;
    result += dest_short;
    
    /* 3. Generate SUBREG pattern */
    /* Cast between different-sized integer types */
    int32_t a = seed * 5;
    int16_t b;
    
    /* Explicit cast should generate SUBREG */
    b = (int16_t)a;
    result += b;
    
    /* Another SUBREG example with unsigned types */
    uint64_t large = (uint64_t)seed * 7;
    uint32_t smaller = (uint32_t)large;
    result ^= smaller;
    
    /* 4. Generate MEM_P with complex addressing */
    /* Complex memory addressing with variable index */
    volatile int idx = (seed * 11) % 200;
    
    /* Multiple complex addressing modes */
    int val1 = global_array[idx + 5];          /* Register + offset */
    int val2 = global_ptr[idx * 2];            /* Pointer with index */
    int val3 = *(global_ptr + idx + 3);        /* Pointer arithmetic */
    
    /* Even more complex: scaled index */
    int val4 = global_array[(idx * 3) % 256];
    
    result += val1 + val2 + val3 + val4;
    
    /* 5. Combine with bitfield operations that might generate ZERO_EXTRACT */
    /* Using union for bitfield access */
    union {
        volatile uint32_t full;
        struct {
            volatile uint32_t low : 8;
            volatile uint32_t mid : 8;
            volatile uint32_t high : 8;
            volatile uint32_t top : 8;
        } bits;
    } bitfield_union;
    
    bitfield_union.full = global_val + seed;
    result += bitfield_union.bits.mid;
    
    /* 6. Additional patterns in a loop to increase coverage probability */
    for (int i = 0; i < 3; i++) {
        /* More MEM with complex addressing */
        volatile int temp = global_array[(idx + i * 17) % 256];
        
        /* More type conversions for SUBREG */
        volatile char c = (char)(temp & 0xFF);
        result += c;
        
        /* Another potential STRICT_LOW_PART */
        volatile short s = (short)temp;
        global_short = s;
    }
    
    return result;
}

/* Main function with command-line argument for variability */
int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize array with pseudo-random but reproducible values */
    srand(seed);
    for (int i = 0; i < 256; i++) {
        global_array[i] = rand();
    }
    
    /* Generate the RTL patterns multiple times */
    int final_result = 0;
    for (int iteration = 0; iteration < 10; iteration++) {
        int iter_seed = seed + iteration * 19;
        final_result ^= generate_rtl_patterns(iter_seed);
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", final_result);
    
    return 0;
}
