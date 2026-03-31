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
#include <assert.h>

/* Ensure optimization is enabled for this test */
#ifndef __OPTIMIZE__
#error "This test requires optimization to be enabled (-O1, -O2, or -O3)"
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* ========== Function 1: Generate ZERO_EXTRACT patterns ========== */
NOINLINE static int test_zero_extract(void) {
    /* Use volatile to prevent constant propagation */
    volatile uint32_t source = 0x12345678;
    volatile int shift = 8;
    volatile int width = 12;
    
    /* Bit-field extraction that may generate ZERO_EXTRACT in RTL */
    uint32_t result = (source >> shift) & ((1U << width) - 1);
    
    /* Multiple bit-field operations to increase chances */
    result |= (source >> 4) & 0xF;
    result ^= (source >> 16) & 0xFF;
    
    return (int)result;
}

/* ========== Function 2: Generate STRICT_LOW_PART patterns ========== */
NOINLINE static int test_strict_low_part(void) {
    int result = 0;
    
    /* Inline assembly that may generate STRICT_LOW_PART on x86 */
#if defined(__i386__) || defined(__x86_64__)
    int32_t val = 0x12345678;
    int32_t output;
    
    /* Byte operation that may use STRICT_LOW_PART */
    asm volatile (
        "movb %b1, %b0\n\t"
        : "=r"(output)
        : "r"(val)
        : "cc"
    );
    
    /* Half-word operation */
    asm volatile (
        "movw %w1, %w0\n\t"
        : "=r"(output)
        : "r"(val)
        : "cc"
    );
    
    result = output;
#else
    /* Fallback: Use union for partial register access */
    union {
        uint32_t full;
        struct {
            uint16_t low;
            uint16_t high;
        } parts;
    } u;
    
    u.full = 0x12345678;
    u.parts.low = 0xABCD;  /* May generate partial register operations */
    result = u.full;
#endif
    
    return result;
}

/* ========== Function 3: Generate SUBREG patterns ========== */
NOINLINE static int test_subreg(void) {
    /* Operations between different-sized types often generate SUBREG */
    int32_t a = 0x12345678;
    int16_t b = a;          /* Truncation may use SUBREG */
    int32_t c = b;          /* Extension may use SUBREG */
    
    /* Use volatile to prevent optimization */
    volatile int64_t large = 0x123456789ABCDEF0LL;
    int32_t low_part = (int32_t)large;      /* May generate SUBREG */
    int32_t high_part = (int32_t)(large >> 32);
    
    /* Pointer casting between types */
    float f = 3.14159f;
    int32_t int_view = *(int32_t*)&f;  /* Type punning */
    
    /* Structure with mixed-size members */
    struct mixed {
        char c;
        short s;
        int i;
    } m;
    
    m.c = 'A';
    m.s = b;
    m.i = a + c;
    
    return m.i + low_part + high_part + int_view;
}

/* ========== Function 4: Generate complex MEM_P patterns ========== */
NOINLINE static int test_mem_operands(void) {
    /* Multi-dimensional array with variable indices */
    volatile int idx1 = 3, idx2 = 7, idx3 = 2;
    int array[10][10][10];
    
    /* Initialize to prevent optimization */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 10; k++) {
                array[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Complex memory addressing with variable offsets */
    int result = array[idx1][idx2][idx3];
    result += *(int*)((char*)array + idx1 * 400 + idx2 * 40 + idx3 * 4);
    
    /* Pointer arithmetic with structure */
    struct point {
        int x, y, z;
    } points[100];
    
    volatile int index = 50;
    result += points[index].x;
    result += points[index + 1].y;
    result += (*(&points[0].z + index * 3));
    
    return result;
}

/* ========== Main function with loop to force RTL generation ========== */
int main(void) {
    int total = 0;
    
    /* Loop to increase chances of RTL generation and resource marking */
    for (int i = 0; i < 100; i++) {
        total += test_zero_extract();
        total += test_strict_low_part();
        total += test_subreg();
        total += test_mem_operands();
        
        /* Conditional to prevent loop unrolling from eliminating all RTL */
        if (total > 1000000) {
            total = 0;
        }
    }
    
    /* Use result to prevent dead code elimination */
    volatile int sink = total;
    
    /* Simple self-check */
    assert(sink >= 0);
    
    return sink != 0 ? 0 : 1;
}
