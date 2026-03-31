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
    /* Use volatile to prevent constant folding */
    volatile uint32_t x = 0x12345678;
    volatile uint32_t y = 0x9ABCDEF0;
    
    /* Bit-field operations that may generate ZERO_EXTRACT */
    uint32_t result = 0;
    
    /* Multiple bit-field extractions */
    result |= (x >> 4) & 0xF;      /* Extract 4 bits */
    result |= (y >> 8) & 0xFF;     /* Extract 8 bits */
    result |= (x >> 16) & 0x3FF;   /* Extract 10 bits */
    
    /* Nested extractions */
    uint32_t temp = (x >> 2) & 0x3F;
    result |= (temp >> 1) & 0x1F;
    
    return (int)result;
}

/* ========== Function 2: Generate STRICT_LOW_PART patterns ========== */
NOINLINE static int test_strict_low_part(void) {
    int result = 0;
    
    /* Use inline assembly for byte operations on x86/x86_64 */
    #if defined(__i386__) || defined(__x86_64__)
    volatile uint32_t val32 = 0x12345678;
    volatile uint16_t val16 = 0x9ABC;
    volatile uint8_t val8 = 0xDE;
    
    /* Byte operations that may generate STRICT_LOW_PART */
    asm volatile (
        "movb %[src8], %%al\n\t"
        "movb %%al, %[dst8]"
        : [dst8] "=m" (val8)
        : [src8] "r" ((uint8_t)0x42)
        : "al"
    );
    
    /* Half-word operation */
    asm volatile (
        "movw %[src16], %%ax\n\t"
        "movw %%ax, %[dst16]"
        : [dst16] "=m" (val16)
        : [src16] "r" ((uint16_t)0x1234)
        : "ax"
    );
    
    result = val8 + val16;
    #else
    /* Fallback for non-x86: use bit-field structure */
    struct {
        unsigned int low : 8;
        unsigned int high : 24;
    } bits = {0};
    
    bits.low = 0x42;  /* This may generate STRICT_LOW_PART */
    result = bits.low;
    #endif
    
    return result;
}

/* ========== Function 3: Generate SUBREG patterns ========== */
NOINLINE static int test_subreg(void) {
    volatile int32_t i32 = 0x12345678;
    volatile int16_t i16 = 0;
    volatile int8_t i8 = 0;
    
    /* Type conversions that may generate SUBREG */
    i16 = (int16_t)i32;          /* 32-bit to 16-bit */
    i8 = (int8_t)i32;            /* 32-bit to 8-bit */
    
    /* Access different parts of a 64-bit value */
    volatile int64_t i64 = 0x123456789ABCDEF0LL;
    i32 = (int32_t)i64;          /* Extract low 32 bits */
    i16 = (int16_t)(i64 >> 32);  /* Extract bits 32-47 */
    
    /* Use union for type punning */
    union {
        uint32_t u32;
        uint16_t u16[2];
        uint8_t u8[4];
    } pun;
    
    pun.u32 = 0xDEADBEEF;
    i16 = pun.u16[0];  /* May generate SUBREG */
    i8 = pun.u8[1];    /* May generate SUBREG */
    
    return i32 + i16 + i8;
}

/* ========== Function 4: Generate complex MEM_P patterns ========== */
NOINLINE static int test_mem_complex(void) {
    volatile int array[100];
    volatile int *ptr = array;
    volatile int index1 = 10, index2 = 20, index3 = 30;
    
    /* Initialize array to prevent optimization */
    for (int i = 0; i < 100; i++) {
        array[i] = i * 3;
    }
    
    int result = 0;
    
    /* Complex memory addressing modes */
    result += array[index1];                 /* Base + index */
    result += array[index2 * 2];             /* Base + scaled index */
    result += *(ptr + index3);               /* Pointer arithmetic */
    result += array[index1 + index2];        /* Base + computed index */
    
    /* Multi-dimensional array with variable indices */
    volatile int matrix[10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    volatile int row = 3, col = 4;
    result += matrix[row][col];              /* 2D array access */
    result += matrix[row][col + 1];          /* Offset within row */
    
    /* Structure with multiple fields */
    struct {
        int a;
        int b;
        int c;
        int d[5];
    } s = {0};
    
    s.a = 1;
    s.b = 2;
    s.c = 3;
    for (int i = 0; i < 5; i++) {
        s.d[i] = i * 10;
    }
    
    result += s.a + s.b + s.c + s.d[index1 % 5];
    
    return result;
}

/* ========== Function 5: Combined patterns in loop ========== */
NOINLINE static int test_combined(void) {
    int sum = 0;
    
    /* Loop to increase chance of RTL generation and resource marking */
    for (volatile int i = 0; i < 10; i++) {
        /* Mix different patterns in loop body */
        sum += test_zero_extract();
        sum += test_strict_low_part();
        sum += test_subreg();
        sum += test_mem_complex();
        
        /* Additional bit-field operations in loop */
        volatile uint32_t x = i * 0x11111111;
        sum += (x >> (i % 16)) & ((1 << (i % 8 + 1)) - 1);
    }
    
    return sum;
}

/* ========== Main function ========== */
int main(void) {
    int result = 0;
    
    /* Static assertion to ensure optimization */
    _Static_assert(__OPTIMIZE__, "Optimization must be enabled");
    
    /* Call all test functions */
    result += test_zero_extract();
    result += test_strict_low_part();
    result += test_subreg();
    result += test_mem_complex();
    result += test_combined();
    
    /* Use result to prevent dead code elimination */
    volatile int sink = result;
    
    return sink != 0 ? 0 : 1;
}
