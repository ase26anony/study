/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile unsigned int global_val = 0xDEADBEEF;
volatile int global_idx = 0;
volatile short global_short;
volatile int global_int;
volatile int global_result = 0;

/* Volatile array for complex memory addressing */
volatile int arr[256];

/* Function to ensure operations aren't optimized away */
__attribute__((noinline)) 
int perform_operations(int seed) {
    volatile int local_result = 0;
    
    /* 1. Generate ZERO_EXTRACT pattern */
    /* Non-constant shift and width to prevent folding */
    volatile unsigned int val = global_val ^ seed;
    volatile int start = (seed % 16) + 1;
    volatile int width = (seed % 8) + 1;
    
    /* This should generate zero_extract RTL on architectures that support it */
    unsigned int extracted = (val >> start) & ((1U << width) - 1);
    local_result ^= extracted;
    
    /* 2. Generate STRICT_LOW_PART pattern */
    /* Assignment to smaller type may generate strict_low_part */
    volatile int src_int = seed * 7;
    volatile short dest_short;
    
    /* This assignment might generate STRICT_LOW_PART RTL */
    dest_short = (short)src_int;
    global_short = dest_short;  /* Store to volatile to ensure side effect */
    local_result += dest_short;
    
    /* 3. Generate SUBREG pattern */
    /* Cast between different-sized integers generates subreg */
    int32_t a = seed * 11;
    int16_t b = (int16_t)a;  /* This should generate SUBREG */
    local_result += b;
    
    /* 4. Generate complex MEM address pattern */
    /* Variable index with offset creates complex addressing mode */
    volatile int idx = (seed * 13) % 200;
    int value = arr[idx + 5];  /* Complex memory address: arr[idx+5] */
    local_result += value;
    
    /* Additional complex memory access with register + offset */
    volatile int *ptr = &arr[100];
    int value2 = ptr[seed % 50];  /* Another complex MEM pattern */
    local_result ^= value2;
    
    /* Mix in some arithmetic to prevent dead code elimination */
    local_result = (local_result * 0x9E3779B9) ^ seed;
    
    return local_result;
}

/* Another function with different patterns */
__attribute__((noinline))
int more_patterns(int seed) {
    volatile int result = 0;
    
    /* Try to force zero_extract with bitfield union */
    union {
        volatile unsigned int full;
        struct {
            volatile unsigned int low16 : 16;
            volatile unsigned int high16 : 16;
        } parts;
    } bitfield;
    
    bitfield.full = seed * 0x1234567;
    result += bitfield.parts.low16;  /* May generate zero_extract */
    
    /* Memory access with scaled index */
    volatile int *base = &arr[0];
    volatile int scale = seed % 4;
    result += base[scale * 16 + 3];  /* Complex addressing */
    
    /* Multiple subreg operations */
    volatile long long big_val = (long long)seed * 0x100000001LL;
    volatile int truncated = (int)big_val;  /* SUBREG */
    result ^= truncated;
    
    return result;
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    /* Use command line argument for variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize array with pseudo-random but deterministic values */
    for (int i = 0; i < 256; i++) {
        arr[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    int final_result = 0;
    
    /* Perform operations in a loop to increase coverage probability */
    for (int i = 0; i < 10; i++) {
        int iter_seed = seed + i * 17;
        
        /* Call functions that should generate target RTL patterns */
        final_result ^= perform_operations(iter_seed);
        final_result += more_patterns(iter_seed + 1);
        
        /* Additional direct operations in main */
        volatile int temp = iter_seed;
        
        /* Another strict_low_part candidate */
        volatile char small;
        small = (char)temp;  /* Assignment to char type */
        final_result += small;
        
        /* Complex memory store (not just load) */
        arr[(iter_seed * 19) % 256] = final_result & 0xFF;
    }
    
    /* Print result to ensure all computations are observable */
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
