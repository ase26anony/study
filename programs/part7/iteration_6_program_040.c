/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource.cc during compilation with optimization enabled.
 * It aims to cover lines 282-290 in mark_referenced_resources().
 */

#include <stdint.h>
#include <stdio.h>

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* Compile-time check for optimization */
#ifndef __OPTIMIZE__
#error "This test requires optimization (-O1, -O2, or -O3) to generate target RTL patterns"
#endif

/* ========== Pattern 1: ZERO_EXTRACT (bit-field operations) ========== */
NOINLINE static int test_zero_extract(void) {
    /* Use volatile to prevent constant propagation */
    volatile unsigned int source = 0xABCD1234U;
    volatile int shift = 8;
    volatile int width = 12;
    
    /* Multiple bit-field extraction patterns */
    unsigned int result = 0;
    
    /* Pattern that may generate ZERO_EXTRACT: extract bit field */
    result = (source >> shift) & ((1U << width) - 1);
    
    /* Another pattern: extract and combine */
    result |= ((source >> 4) & 0xF) << 16;
    
    /* Use result to prevent dead code elimination */
    return (int)(result & 0xFFFF);
}

/* ========== Pattern 2: STRICT_LOW_PART (partial register access) ========== */
NOINLINE static int test_strict_low_part(void) {
    volatile uint32_t value = 0xDEADBEEF;
    uint32_t result = 0;
    
    /* Force partial register updates through various means */
    
    /* Method 1: Byte operations on larger registers */
    uint8_t *byte_ptr = (uint8_t *)&result;
    byte_ptr[0] = (uint8_t)(value >> 0);
    byte_ptr[1] = (uint8_t)(value >> 8);
    byte_ptr[2] = (uint8_t)(value >> 16);
    byte_ptr[3] = (uint8_t)(value >> 24);
    
    /* Method 2: 16-bit operations */
    uint16_t *short_ptr = (uint16_t *)&result;
    short_ptr[0] = (uint16_t)(value >> 0);
    short_ptr[1] = (uint16_t)(value >> 16);
    
    /* Method 3: Inline assembly for x86 (guarded) */
#if defined(__i386__) || defined(__x86_64__)
    uint32_t asm_result;
    __asm__ volatile (
        "movl %1, %0\n\t"
        "movb %b2, %b0\n\t"  /* Modify only low byte */
        : "=r"(asm_result)
        : "r"(value), "r"(0x42)
        : "cc"
    );
    result ^= asm_result;
#endif
    
    return (int)result;
}

/* ========== Pattern 3: SUBREG (type conversions and partial access) ========== */
NOINLINE static int test_subreg(void) {
    volatile long long big_value = 0x123456789ABCDEF0LL;
    int result = 0;
    
    /* Various type conversions that may generate SUBREG */
    
    /* 64-bit to 32-bit truncation */
    int truncated = (int)big_value;
    result += truncated;
    
    /* 32-bit to 16-bit */
    short half = (short)truncated;
    result += half;
    
    /* 16-bit to 8-bit */
    char quarter = (char)half;
    result += quarter;
    
    /* Pointer-based type punning */
    int32_t *as_int = (int32_t *)&big_value;
    result += as_int[0];  /* Low 32 bits */
    result += as_int[1];  /* High 32 bits (on little-endian) */
    
    /* Union for type punning */
    union {
        long long ll;
        int i[2];
    } pun;
    pun.ll = big_value;
    result += pun.i[0] + pun.i[1];
    
    return result;
}

/* ========== Pattern 4: MEM_P with complex addressing ========== */
NOINLINE static int test_mem_complex_address(void) {
    /* Multi-dimensional array with variable indices */
    volatile int matrix[10][20][30];
    volatile int indices[3] = {3, 7, 11};
    int result = 0;
    
    /* Complex addressing modes */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                /* Variable indices create complex address calculations */
                result += matrix[indices[i]][indices[j]][indices[k]];
            }
        }
    }
    
    /* Pointer arithmetic with variables */
    int *ptr = (int *)matrix;
    volatile int offset = 42;
    
    result += *(ptr + offset);
    result += *(ptr + offset * 2);
    result += *(ptr + offset * 3);
    
    /* Structure with nested arrays */
    struct {
        int data[5][5];
        int extra;
    } s;
    
    volatile int idx1 = 2, idx2 = 3;
    result += s.data[idx1][idx2];
    result += s.extra;
    
    return result;
}

/* ========== Combined test with loops to increase RTL generation ========== */
NOINLINE static int run_all_patterns(int iterations) {
    int total = 0;
    
    for (int i = 0; i < iterations; i++) {
        total += test_zero_extract();
        total += test_strict_low_part();
        total += test_subreg();
        total += test_mem_complex_address();
        
        /* Add some branching to prevent loop unrolling from eliminating patterns */
        if (total > 1000000) {
            total = total % 1000;
        }
    }
    
    return total;
}

/* ========== Main function ========== */
int main(void) {
    /* Run tests multiple times to increase chance of RTL generation */
    int result = run_all_patterns(10);
    
    /* Use result to prevent optimization */
    printf("Test result: %d\n", result);
    
    /* Return non-zero only on error (though any execution is success for coverage) */
    return (result == 0) ? 1 : 0;
}
