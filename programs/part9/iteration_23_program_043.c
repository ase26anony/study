/* Test program to trigger uncovered RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_int = 0;
volatile short g_volatile_short = 0;
volatile char g_volatile_char = 0;

/* ==================== BIT-FIELD TESTS (ZERO_EXTRACT) ==================== */

struct BitFieldStruct {
    volatile unsigned int flag:1;
    volatile unsigned int value:10;
    volatile unsigned int mode:3;
    unsigned int padding:18;
};

struct NestedBitField {
    struct {
        volatile unsigned int low:4;
        volatile unsigned int high:4;
    } byte;
    volatile unsigned int word:16;
};

void test_bitfield_zero_extract(void) {
    struct BitFieldStruct s1 = {0};
    struct BitFieldStruct s2 = {0};
    
    /* Direct bit-field assignments - should generate ZERO_EXTRACT */
    s1.flag = 1;
    s1.value = 511;  /* Max 10-bit value */
    s1.mode = 3;
    
    /* Bit-field to bit-field assignment */
    s2.flag = s1.flag;
    s2.value = s1.value;
    s2.mode = s1.mode;
    
    /* Bit-field in arithmetic expression */
    unsigned int temp = s1.value + s2.value;
    s1.value = temp & 0x3FF;  /* Mask to fit in 10 bits */
    
    /* Complex bit-field expression */
    s1.mode = (s1.flag << 2) | (s2.mode & 1);
    
    /* Use volatile global to force side effects */
    g_volatile_int = s1.value;
}

void test_nested_bitfield(void) {
    struct NestedBitField nbf = {0};
    
    /* Access nested bit-fields */
    nbf.byte.low = 0xF;
    nbf.byte.high = 0xA;
    nbf.word = 0xABCD;
    
    /* Cross-byte bit-field operations */
    unsigned int combined = (nbf.byte.high << 4) | nbf.byte.low;
    nbf.word = combined | (nbf.word & 0xFF00);
    
    g_volatile_int = nbf.word;
}

/* ==================== PARTIAL REGISTER TESTS (STRICT_LOW_PART) ==================== */

void test_strict_low_part(void) {
    volatile short vs = 0;
    volatile char vc = 0;
    int i = 100;
    long l = 1000;
    
    /* Cast to smaller types - should generate STRICT_LOW_PART */
    vs = (short)(i + 50);
    vc = (char)(l % 256);
    
    /* Arithmetic on sub-word types */
    vs = vs + 10;
    vc = vc - 5;
    
    /* Mixed-size operations */
    i = vs * 2;      /* short to int promotion */
    vs = i / 3;      /* int to short with truncation */
    
    /* Store to global volatiles */
    g_volatile_short = vs;
    g_volatile_char = vc;
}

void test_partial_register_arithmetic(void) {
    volatile unsigned short us = 65535;
    volatile signed char sc = -128;
    
    /* Operations that might generate partial register updates */
    us = us + 1;      /* Should wrap for 16-bit */
    sc = sc - 1;      /* Signed 8-bit arithmetic */
    
    /* Compound assignment on partial types */
    us += 0x100;
    sc *= 2;
    
    g_volatile_int = us + sc;
}

/* ==================== SUBREG TESTS ==================== */

/* Vector type for SUBREG operations */
typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));

void test_subreg_vector(void) {
    v4si v = {1, 2, 3, 4};
    v8hi w = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Vector element access - should generate SUBREG */
    int element = v[2];
    short selement = w[5];
    
    /* Partial vector operations */
    v[0] = element * 2;
    w[3] = selement + 100;
    
    /* Vector to scalar conversion */
    g_volatile_int = v[1];
    g_volatile_short = w[4];
}

union TypePunning {
    unsigned int full;
    struct {
        volatile unsigned short low;
        volatile unsigned short high;
    } halves;
    volatile unsigned char bytes[4];
};

void test_subreg_union(void) {
    union TypePunning u;
    u.full = 0x12345678;
    
    /* Access different views of same memory - should generate SUBREG */
    unsigned short low_half = u.halves.low;
    unsigned short high_half = u.halves.high;
    
    /* Byte access through array */
    u.bytes[1] = 0xAA;
    u.bytes[3] = 0xBB;
    
    /* Reconstruct from parts */
    u.halves.high = low_half;
    u.halves.low = high_half;
    
    g_volatile_int = u.full;
}

/* ==================== COMBINED PATTERN TESTS ==================== */

struct Combined {
    volatile unsigned int bf1:5;
    volatile unsigned int bf2:11;
    volatile short partial;
    volatile char small;
};

void test_combined_patterns(void) {
    struct Combined c = {0};
    
    /* Bit-field to partial register */
    c.bf1 = 0x1F;
    c.partial = (short)c.bf1 * 10;
    
    /* Partial register to bit-field */
    c.small = -50;
    c.bf2 = (unsigned int)c.small & 0x7FF;
    
    /* Complex expression mixing patterns */
    c.partial = (c.partial + c.bf2) | (c.small << 8);
    
    /* Loop to ensure RTL generation */
    for (int i = 0; i < 3; i++) {
        c.bf1 = (c.bf1 + i) & 0x1F;
        c.small = c.small + c.bf1;
    }
    
    g_volatile_int = c.bf1 + c.bf2 + c.partial + c.small;
}

/* ==================== ARCHITECTURE-SPECIFIC TESTS ==================== */

#ifdef __i386__
void test_x86_specific(void) {
    volatile int result;
    
    /* Inline assembly that might generate partial register ops */
    __asm__ volatile (
        "movw $0x1234, %%ax\n\t"
        "movb $0x56, %%ah\n\t"
        "movw %%ax, %0"
        : "=r" (result)
        :
        : "%ax"
    );
    
    g_volatile_int = result;
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    volatile short hs;
    
    /* ARM may generate different patterns for 16-bit accesses */
    __asm__ volatile (
        "movw %0, #0x1234\n\t"
        "movt %0, #0x5678"
        : "=r" (hs)
        :
    );
    
    g_volatile_short = hs;
}
#endif

/* ==================== BUILTIN TESTS ==================== */

void test_builtins(void) {
    unsigned int x = 0x12345678;
    
    /* Builtins that involve bit manipulation */
    int leading_zeros = __builtin_clz(x);
    int parity = __builtin_parity(x);
    int popcount = __builtin_popcount(x);
    
    /* Extract bits using builtins */
    unsigned int extracted = __builtin_ia32_bextr_u32(x, 0x0804);
    
    /* Use results to prevent optimization */
    g_volatile_int = leading_zeros + parity + popcount + extracted;
}

/* ==================== MAIN TEST DRIVER ==================== */

typedef void (*test_func_t)(void);

const test_func_t test_functions[] = {
    test_bitfield_zero_extract,
    test_nested_bitfield,
    test_strict_low_part,
    test_partial_register_arithmetic,
    test_subreg_vector,
    test_subreg_union,
    test_combined_patterns,
    test_builtins,
#ifdef __i386__
    test_x86_specific,
#endif
#ifdef __arm__
    test_arm_specific,
#endif
    NULL
};

int main(int argc, char *argv[]) {
    int test_to_run = 0;
    
    /* Use command line or volatile to control which tests run */
    if (argc > 1) {
        test_to_run = atoi(argv[1]) % 10;
    } else {
        test_to_run = g_volatile_int % 10;
    }
    
    /* Run all tests in sequence to maximize RTL pattern generation */
    for (int i = 0; test_functions[i] != NULL; i++) {
        if (test_to_run == 0 || test_to_run == i + 1) {
            test_functions[i]();
        }
    }
    
    /* Final computation to ensure program does something useful */
    int result = g_volatile_int + g_volatile_short + g_volatile_char;
    
    printf("Result: %d\n", result);
    return result == 0 ? 0 : 1;
}
