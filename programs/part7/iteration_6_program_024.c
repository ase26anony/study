/* test_resource_coverage.c
 * 
 * This program is designed to generate RTL patterns that will exercise
 * the uncovered lines in GCC's resource.cc (lines 282-290) during
 * compilation with optimization enabled.
 * 
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fschedule-insns -c test_resource_coverage.c
 * For coverage: gcc -O2 -fprofile-arcs -ftest-coverage test_resource_coverage.c -o test
 */

#include <stdint.h>
#include <stdlib.h>

/* Ensure optimization is enabled at compile time */
#ifdef __OPTIMIZE__
#define OPTIMIZATION_ENABLED 1
#else
#define OPTIMIZATION_ENABLED 0
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* Static assertion to ensure optimization */
_Static_assert(OPTIMIZATION_ENABLED, "Compile with -O1 or higher for coverage");

/* Global volatile variables to prevent optimization */
volatile unsigned int global_volatile = 0x12345678;
volatile int global_index = 0;

/* ============================================
 * Function 1: Generate ZERO_EXTRACT patterns
 * ============================================ */
NOINLINE static unsigned int test_zero_extract(void) {
    /* Bit-field extraction operations that may generate ZERO_EXTRACT */
    volatile unsigned int source = global_volatile;
    
    /* Multiple bit-field extractions */
    unsigned int result = 0;
    
    /* Extract bits 4-7 */
    result |= (source >> 4) & 0xF;
    
    /* Extract bits 8-15 */
    result |= (source >> 8) & 0xFF;
    
    /* Extract bits 16-23 with masking */
    result |= (source & 0x00FF0000) >> 16;
    
    /* Nested extractions */
    unsigned int temp = (source >> 12) & 0x7;
    result |= (temp << 3) & 0x38;
    
    return result;
}

/* ============================================
 * Function 2: Generate STRICT_LOW_PART patterns
 * ============================================ */
NOINLINE static unsigned int test_strict_low_part(void) {
    unsigned int result = 0;
    
#ifdef __x86_64__ || __i386__
    /* x86-specific inline assembly that may generate STRICT_LOW_PART */
    unsigned char byte_val = 0x42;
    unsigned short word_val = 0x1234;
    
    /* Byte operation that modifies only part of a register */
    asm volatile (
        "movb %1, %b0\n\t"
        : "=q"(result)
        : "r"(byte_val)
        : "cc"
    );
    
    /* Word operation */
    asm volatile (
        "movw %1, %w0\n\t"
        : "+r"(result)
        : "r"(word_val)
        : "cc"
    );
#else
    /* Generic fallback: operations on different-sized types */
    uint32_t full = 0xDEADBEEF;
    uint16_t half;
    
    /* This may generate SUBREG or similar patterns */
    half = (uint16_t)full;
    result = half;
    
    /* Byte access through pointer casting */
    uint8_t *byte_ptr = (uint8_t*)&full;
    result = byte_ptr[1] | (byte_ptr[2] << 8);
#endif
    
    return result;
}

/* ============================================
 * Function 3: Generate SUBREG patterns
 * ============================================ */
NOINLINE static unsigned int test_subreg(void) {
    unsigned int result = 0;
    
    /* Operations on different-sized types */
    long long big_val = 0x1122334455667788LL;
    int int_val;
    short short_val;
    char char_val;
    
    /* Type conversions that may generate SUBREG */
    int_val = (int)big_val;           /* truncation */
    short_val = (short)int_val;       /* narrowing */
    char_val = (char)short_val;       /* further narrowing */
    
    /* Access different parts of larger types */
    result = (int)(big_val >> 32);    /* high part */
    result |= (int)big_val;           /* low part */
    
    /* Mix sizes in calculations */
    result = int_val + short_val + char_val;
    
    /* Union for type punning */
    union {
        uint64_t dword;
        uint32_t words[2];
        uint16_t halves[4];
    } pun;
    
    pun.dword = big_val;
    result = pun.words[0] + pun.halves[2];
    
    return result;
}

/* ============================================
 * Function 4: Generate complex MEM_P patterns
 * ============================================ */
NOINLINE static unsigned int test_mem_operands(void) {
    /* Complex memory addressing modes */
    static int array[256][16];
    int *ptr = &array[0][0];
    volatile int idx = global_index;
    unsigned int result = 0;
    
    /* Multi-dimensional array access with variable indices */
    for (int i = 0; i < 8; i++) {
        /* Complex addressing: array[idx + i][idx % 16] */
        result += array[idx + i][idx % 16];
        
        /* Pointer arithmetic with non-constant offsets */
        result += *(ptr + (idx * 4) + i);
        
        /* Nested memory accesses */
        int temp = array[i][idx & 0xF];
        result += array[temp & 0xFF][i];
    }
    
    /* Structure with nested arrays */
    struct {
        int data[4][8];
        int *next;
    } s;
    
    /* Initialize to avoid undefined behavior */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            s.data[i][j] = i * 8 + j;
        }
    }
    
    /* Complex structure member access */
    for (int i = 0; i < 4; i++) {
        result += s.data[i][idx % 8];
    }
    
    return result;
}

/* ============================================
 * Function 5: Combined patterns in loop
 * ============================================ */
NOINLINE static unsigned int test_combined(void) {
    unsigned int result = 0;
    
    /* Loop to increase RTL generation opportunities */
    for (volatile int i = 0; i < 100; i++) {
        /* Mix different patterns */
        result ^= test_zero_extract();
        
        if (i & 1) {
            result += test_strict_low_part();
        }
        
        /* Memory access with bit-field extraction */
        volatile unsigned int mem_val = global_volatile + i;
        result |= (mem_val >> (i % 16)) & 0xFF;
        
        /* Type conversion in loop */
        short temp = (short)result;
        result = temp + i;
    }
    
    return result;
}

/* ============================================
 * Main function
 * ============================================ */
int main(void) {
    unsigned int result = 0;
    
    /* Call all test functions multiple times */
    for (int i = 0; i < 10; i++) {
        result += test_zero_extract();
        result += test_strict_low_part();
        result += test_subreg();
        result += test_mem_operands();
        result += test_combined();
        
        /* Modify global to change behavior */
        global_index = (global_index + 1) & 0xFF;
    }
    
    /* Use result to prevent optimization */
    return (int)(result & 0x7FFFFFFF);
}
