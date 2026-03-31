/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent optimization and ensure RTL generation */
volatile unsigned int global_val = 0xDEADBEEF;
volatile int global_idx = 0;
volatile int global_start = 4;
volatile int global_width = 8;

/* Arrays for memory access patterns */
volatile int mem_array[256];
volatile short short_array[256];

/* Function to ensure operations aren't optimized away */
static int use_result(int val) {
    volatile int sink = val;
    return sink;
}

/* Generate ZERO_EXTRACT pattern */
static unsigned int gen_zero_extract(void) {
    /* Non-constant shift and mask to prevent folding */
    volatile unsigned int val = global_val;
    volatile int start = global_start;
    volatile int width = global_width;
    
    /* This should generate ZERO_EXTRACT RTL on architectures that support it */
    unsigned int result = (val >> start) & ((1U << width) - 1);
    
    /* Add dependency to prevent dead code elimination */
    global_val ^= result;
    return result;
}

/* Generate STRICT_LOW_PART pattern */
static short gen_strict_low_part(void) {
    volatile int src = global_val;
    volatile short dest;
    
    /* Assignment to smaller type may generate STRICT_LOW_PART */
    dest = (short)src;
    
    /* Modify source to ensure the operation matters */
    global_val += 1;
    return dest;
}

/* Generate SUBREG pattern */
static int16_t gen_subreg(void) {
    volatile int32_t a = global_val;
    volatile int16_t b;
    
    /* Cast between different sizes should generate SUBREG */
    b = (int16_t)a;
    
    return b;
}

/* Generate complex MEM address pattern */
static int gen_complex_mem(void) {
    volatile int idx = global_idx;
    volatile int result;
    
    /* Complex addressing: array base + variable index + constant offset */
    /* This should generate MEM with non-simple address expression */
    result = mem_array[idx + 5];
    
    /* Also try with pointer arithmetic */
    volatile int *ptr = &mem_array[100];
    result += ptr[idx - 3];
    
    return result;
}

/* Additional patterns for different architectures */
#ifdef __arm__
static unsigned int arm_specific_patterns(void) {
    /* ARM-specific patterns that might generate ZERO_EXTRACT */
    volatile unsigned int val = global_val;
    unsigned int result;
    
    /* UBFX-like pattern */
    result = (val >> 8) & 0xFF;
    
    /* Bitfield insert/extract patterns */
    __asm__ volatile("" : "+r" (result) : "r" (val));
    
    return result;
}
#endif

#ifdef __x86_64__
static unsigned int x86_specific_patterns(void) {
    volatile unsigned int val = global_val;
    unsigned int result;
    
    /* BMI2 BZHI-like pattern if available */
    result = val & ((1U << 12) - 1);
    
    /* Use inline asm to prevent optimization */
    __asm__ volatile("" : "+r" (result) : "r" (val));
    
    return result;
}
#endif

int main(int argc, char *argv[]) {
    int seed = 0;
    unsigned int final_result = 0;
    
    /* Use command line argument for reproducible but variable input */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = 12345;
    }
    
    /* Initialize with pseudo-random but reproducible values */
    srand(seed);
    global_val = rand();
    global_idx = rand() % 200;
    global_start = rand() % 16;
    global_width = (rand() % 8) + 1;  /* width 1-8 */
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        mem_array[i] = rand();
        short_array[i] = (short)rand();
    }
    
    /* Generate all target RTL patterns in sequence */
    final_result ^= gen_zero_extract();
    final_result += gen_strict_low_part();
    final_result ^= gen_subreg();
    final_result += gen_complex_mem();
    
    /* Architecture-specific patterns */
#ifdef __arm__
    final_result ^= arm_specific_patterns();
#endif
    
#ifdef __x86_64__
    final_result ^= x86_specific_patterns();
#endif
    
    /* Additional mixed patterns in a loop to increase coverage chance */
    for (int i = 0; i < 10; i++) {
        volatile int temp = mem_array[global_idx + i];
        volatile short stemp = (short)temp;
        final_result += stemp;
        
        /* Bitfield operation with variable parameters */
        volatile int shift = (i * 3) % 16;
        final_result ^= (global_val >> shift) & 0xF;
    }
    
    /* Ensure result is used */
    final_result = use_result(final_result);
    
    printf("Result: %u (0x%08X)\n", final_result, final_result);
    
    return (final_result != 0) ? 0 : 1;
}
