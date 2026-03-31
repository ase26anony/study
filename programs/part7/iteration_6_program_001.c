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

_Static_assert(OPTIMIZATION_ENABLED, "Compile with optimization (-O1, -O2, or -O3)");

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* Function 1: Generate ZERO_EXTRACT patterns using bit-field operations */
NOINLINE static int test_zero_extract(void) {
    /* Use volatile to prevent complete optimization */
    volatile unsigned int source = 0xABCD1234;
    volatile unsigned int shift_amount = 8;
    volatile unsigned int mask_width = 12;
    
    /* These operations often generate ZERO_EXTRACT in RTL */
    unsigned int extracted = (source >> shift_amount) & ((1U << mask_width) - 1);
    
    /* Bit-field structure - may also generate ZERO_EXTRACT */
    struct bitfield {
        unsigned int low : 4;
        unsigned int middle : 12;
        unsigned int high : 16;
    } bf;
    
    bf.low = extracted & 0xF;
    bf.middle = (extracted >> 4) & 0xFFF;
    
    return bf.low + bf.middle;
}

/* Function 2: Generate STRICT_LOW_PART patterns using inline assembly */
NOINLINE static int test_strict_low_part(void) {
    int result = 0;
    
    /* STRICT_LOW_PART often appears with byte/halfword operations */
#if defined(__i386__) || defined(__x86_64__)
    /* x86-specific assembly that may generate STRICT_LOW_PART */
    unsigned short val16 = 0x1234;
    unsigned char val8 = 0xAB;
    
    asm volatile (
        "movw %1, %0\n\t"
        : "=r"(val16)
        : "r"(val16)
        : /* clobbers */
    );
    
    asm volatile (
        "movb %1, %0\n\t"
        : "=q"(val8)  /* "q" constraint for byte-addressable register */
        : "r"(val8)
        : /* clobbers */
    );
    
    result = val16 + val8;
#else
    /* Generic fallback: operations on sub-word types */
    volatile uint32_t full = 0xDEADBEEF;
    uint16_t half = (full >> 8) & 0xFFFF;
    uint8_t quarter = full & 0xFF;
    
    /* Pointer casting to access partial registers */
    uint8_t *byte_ptr = (uint8_t *)&full;
    result = byte_ptr[0] + byte_ptr[1] + half;
#endif
    
    return result;
}

/* Function 3: Generate SUBREG patterns through type conversions */
NOINLINE static int test_subreg(void) {
    volatile long long big_value = 0x123456789ABCDEF0LL;
    
    /* These conversions often generate SUBREG in RTL */
    int truncated = (int)big_value;           /* truncation */
    short shorter = (short)truncated;         /* further truncation */
    unsigned int extended = (unsigned int)shorter; /* extension */
    
    /* Access different parts of a larger type */
    union {
        long long full;
        struct {
            int low;
            int high;
        } parts;
    } u;
    
    u.full = big_value;
    int low_part = u.parts.low;    /* May generate SUBREG */
    int high_part = u.parts.high;  /* May generate SUBREG */
    
    /* Pointer arithmetic with different types */
    char *char_ptr = (char *)&big_value;
    int *int_ptr = (int *)&big_value;
    
    return truncated + shorter + extended + low_part + high_part + char_ptr[0] + int_ptr[0];
}

/* Function 4: Generate complex MEM_P patterns with addressing modes */
NOINLINE static int test_mem_operands(void) {
    volatile int array[64];
    volatile int *ptr = array;
    volatile int index1 = 10, index2 = 20, index3 = 30;
    
    /* Initialize to prevent optimization */
    for (int i = 0; i < 64; i++) {
        array[i] = i * 3;
    }
    
    /* Complex memory addressing patterns */
    int sum = 0;
    
    /* Array indexing with variable offsets */
    sum += array[index1];
    sum += array[index2 * 2];
    sum += *(ptr + index3);
    
    /* Pointer arithmetic with multiple variables */
    sum += ptr[index1 + index2];
    
    /* Structure access with variable field offset */
    struct nested {
        int a;
        int b[8];
        struct {
            int x;
            int y;
        } inner;
    } s;
    
    volatile int field_idx = 2;
    s.a = 100;
    s.b[field_idx] = 200;
    s.inner.x = 300;
    
    sum += s.a + s.b[field_idx] + s.inner.x;
    
    /* Multi-dimensional array with variable indices */
    volatile int matrix[8][8];
    volatile int row = 3, col = 4;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            matrix[i][j] = i * 8 + j;
        }
    }
    
    sum += matrix[row][col];
    sum += matrix[col][row];  /* Different access pattern */
    
    return sum;
}

/* Function 5: Mix all patterns in a loop to increase RTL generation */
NOINLINE static int test_mixed_patterns(int iterations) {
    int total = 0;
    
    for (int i = 0; i < iterations; i++) {
        total += test_zero_extract();
        total += test_strict_low_part();
        total += test_subreg();
        total += test_mem_operands();
        
        /* Prevent loop unrolling from eliminating patterns */
        volatile int barrier = i;
        if (barrier & 1) {
            total += 1;
        }
    }
    
    return total;
}

/* Main function that drives the tests */
int main(void) {
    int result = 0;
    
    /* Call each test function individually */
    result += test_zero_extract();
    result += test_strict_low_part();
    result += test_subreg();
    result += test_mem_operands();
    
    /* Mixed patterns in a loop */
    result += test_mixed_patterns(5);
    
    /* Return a non-zero value to indicate success */
    return (result != 0) ? 0 : 1;
}
