/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile unsigned int global_val = 0x12345678;
volatile unsigned short global_short = 0;
volatile int global_array[256];
volatile int global_result = 0;

/* Function to ensure operations aren't optimized away */
__attribute__((noinline))
int perform_operations(int seed) {
    volatile int local_result = 0;
    
    /* 1. Generate ZERO_EXTRACT pattern */
    /* Bit-field extraction with variable width and position */
    volatile unsigned int extract_src = global_val ^ seed;
    volatile int start = (seed % 16) + 1;  /* Non-constant start position */
    volatile int width = (seed % 8) + 1;   /* Non-constant width */
    
    /* This should generate ZERO_EXTRACT RTL on architectures that support it */
    unsigned int extracted = (extract_src >> start) & ((1U << width) - 1);
    local_result ^= extracted;
    
    /* 2. Generate STRICT_LOW_PART pattern */
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    volatile int src_int = seed * 37;
    volatile short dest_short;
    
    /* Explicit cast and assignment to volatile short */
    dest_short = (short)src_int;
    global_short = dest_short;  /* Force memory store */
    local_result += dest_short;
    
    /* 3. Generate SUBREG pattern */
    /* Cast between different-sized integer types */
    int32_t a = seed * 73;
    int16_t b = (int16_t)a;      /* Should generate SUBREG */
    int32_t c = (int32_t)b;      /* Another SUBREG */
    local_result += c;
    
    /* 4. Generate complex MEM address pattern */
    /* Array access with variable, computed index */
    volatile int* volatile ptr = (volatile int*)global_array;
    int idx = (seed * 97) % 200;
    
    /* Complex addressing: base + scaled index + offset */
    int value1 = ptr[idx + 5];                    /* MEM with address calculation */
    int value2 = global_array[(idx * 3) % 200];   /* Another complex MEM */
    
    /* Even more complex: pointer arithmetic */
    volatile int* ptr2 = &global_array[100];
    int value3 = ptr2[(seed % 50) - 25];          /* MEM with base + variable offset */
    
    local_result += value1 + value2 + value3;
    
    /* Mix all results together */
    return local_result ^ (seed * 0x9E3779B9);
}

/* Additional function to create more RTL patterns in different contexts */
__attribute__((noinline))
void more_patterns(int seed) {
    /* Use union for potential bitfield operations */
    union {
        volatile uint32_t full;
        struct {
            volatile uint16_t low;
            volatile uint16_t high;
        } parts;
    } data;
    
    data.full = seed * 0xABCD1234;
    
    /* Access through union might generate interesting RTL */
    global_short = data.parts.low;
    
    /* Bitfield structure - may generate EXTRACT on some architectures */
    struct bitfields {
        unsigned int a : 5;
        unsigned int b : 10;
        unsigned int c : 15;
    };
    
    volatile struct bitfields bf;
    bf.a = (seed >> 3) & 0x1F;
    bf.b = (seed >> 8) & 0x3FF;
    bf.c = (seed >> 16) & 0x7FFF;
    
    /* Force use of bitfields */
    global_result += bf.b;
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    /* Use command line argument for variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize array with pseudo-random but deterministic values */
    for (int i = 0; i < 256; i++) {
        global_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Perform operations that should generate target RTL patterns */
    int result1 = perform_operations(seed);
    more_patterns(seed);
    
    /* Second round with different seed to potentially trigger different paths */
    int result2 = perform_operations(seed * 2);
    
    /* Combine results in a data-dependent way */
    int final_result = result1 + result2 + global_result;
    
    /* Print result to ensure all computations are observable */
    printf("Result: %d (0x%08x)\n", final_result, final_result);
    
    return final_result != 0 ? 0 : 1;
}
