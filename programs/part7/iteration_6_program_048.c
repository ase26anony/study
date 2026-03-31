/* Test program to cover specific RTL patterns in GCC's resource.cc */
#include <stdint.h>
#include <assert.h>

/* Ensure optimization is enabled for RTL generation */
#ifndef __OPTIMIZE__
#error "Compile with -O2 or -O3 for RTL pattern generation"
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* For STRICT_LOW_PART - use inline assembly on x86 */
#ifdef __x86_64__

NOINLINE static uint32_t strict_low_part_asm(uint32_t val) {
    uint8_t low_byte;
    /* Assembly that modifies only low byte of a register */
    asm volatile (
        "movb %b1, %b0"
        : "=q" (low_byte)      /* =q constraint for byte-addressable register */
        : "r" (val)
        : "cc"
    );
    return low_byte;
}

#else
/* Fallback for non-x86: use bit masking which may generate similar patterns */
NOINLINE static uint32_t strict_low_part_fallback(uint32_t val) {
    volatile uint8_t low_byte = val & 0xFF;
    /* Force memory access to prevent optimization */
    asm volatile ("" : "+m" (low_byte));
    return low_byte;
}
#endif

/* For ZERO_EXTRACT - use bit-field operations */
NOINLINE static uint32_t zero_extract_bitfield(volatile uint32_t *ptr) {
    /* Complex bit-field extraction that may generate ZERO_EXTRACT */
    uint32_t x = *ptr;  /* Volatile read */
    uint32_t result = 0;
    
    /* Multiple bit-field extractions with shifting and masking */
    result |= (x >> 3) & 0x1F;      /* Extract bits 3-7 */
    result |= ((x >> 10) & 0x3F) << 5;  /* Extract bits 10-15, shift left */
    result |= ((x >> 20) & 0xFF) << 11; /* Extract bits 20-27 */
    
    /* Another extraction pattern */
    uint32_t y = (x >> 5) & 0x7;    /* Extract 3 bits */
    result ^= y;
    
    return result;
}

/* For SUBREG - type conversions and partial register access */
NOINLINE static int32_t subreg_type_punning(uint64_t big_val) {
    /* Access different-sized parts of the value */
    int32_t low_part = (int32_t)big_val;          /* Truncation */
    int16_t high_part = (int16_t)(big_val >> 32); /* Extract high 16 bits */
    
    /* Mix operations to force SUBREG usage */
    volatile int32_t temp = low_part;
    temp += (int32_t)high_part;  /* Sign-extend high_part */
    
    /* Cast between different integer sizes */
    uint8_t byte_val = (uint8_t)temp;
    int32_t extended = (int32_t)byte_val;  /* May generate SUBREG for extension */
    
    return extended + temp;
}

/* For MEM_P with complex addressing - array and pointer arithmetic */
NOINLINE static int mem_complex_addressing(int index1, int index2) {
    volatile int array[64][64];  /* Force memory operations */
    int *ptr = (int*)array;
    int result = 0;
    
    /* Complex addressing calculations */
    result += array[index1][index2];
    result += *(ptr + index1 * 64 + index2);
    
    /* Pointer arithmetic with variable offsets */
    int *p = &array[0][0];
    for (int i = 0; i < 8; i++) {
        result += p[index1 + i * index2];
    }
    
    /* Structure-like access via pointer casting */
    struct { int a; int b; int c; } s;
    int *sp = (int*)&s;
    result += sp[index1 & 3];
    
    return result;
}

/* Combined function that uses all patterns in a loop */
NOINLINE static int combined_patterns(int iterations) {
    volatile uint32_t bitfield_var = 0xDEADBEEF;
    uint64_t big_val = 0x123456789ABCDEF0ULL;
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Modify values to prevent constant folding */
        bitfield_var ^= i;
        big_val += i;
        
        /* Call pattern functions */
        #ifdef __x86_64__
        sum += strict_low_part_asm(bitfield_var);
        #else
        sum += strict_low_part_fallback(bitfield_var);
        #endif
        
        sum += zero_extract_bitfield(&bitfield_var);
        sum += subreg_type_punning(big_val);
        sum += mem_complex_addressing(i & 31, (i * 7) & 31);
    }
    
    return sum;
}

/* Main function with compile-time optimization check */
int main(void) {
    /* Ensure optimization is enabled at compile time */
    _Static_assert(__OPTIMIZE__, "Optimization must be enabled for RTL coverage");
    
    /* Initialize with non-zero values */
    volatile int seed = 42;
    
    /* Run combined patterns with enough iterations to trigger optimizations */
    int result = combined_patterns(100);
    
    /* Use result to prevent dead code elimination */
    asm volatile ("" : : "r" (result));
    
    return result != 0 ? 0 : 1;
}
