/* Test program to trigger uncovered RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Use volatile to prevent optimization */
volatile int control = 0;

/* ==================== ZERO_EXTRACT patterns ==================== */

/* Bit-field structures for ZERO_EXTRACT */
struct BitFieldStruct {
    volatile unsigned int flag:1;
    volatile unsigned int value:10;
    unsigned int padding:21;
};

struct NestedBitField {
    struct {
        volatile unsigned int a:3;
        volatile unsigned int b:5;
        volatile unsigned int c:8;
    } inner;
    volatile unsigned int d:16;
};

/* Test ZERO_EXTRACT patterns */
void test_zero_extract(void) {
    struct BitFieldStruct bfs1 = {0, 0, 0};
    struct BitFieldStruct bfs2 = {0, 0, 0};
    struct NestedBitField nbf = {{{0, 0, 0}}, 0};
    
    /* Basic bit-field assignments */
    bfs1.flag = 1;
    bfs1.value = 511; /* Max 10-bit value */
    
    /* Cross-structure bit-field assignment */
    bfs2.value = bfs1.value;
    bfs2.flag = bfs1.flag;
    
    /* Nested bit-field access */
    nbf.inner.a = 5;
    nbf.inner.b = 20;
    nbf.inner.c = nbf.inner.a + nbf.inner.b;
    nbf.d = (nbf.inner.c << 3) | nbf.inner.a;
    
    /* Complex expression with bit-fields */
    unsigned int temp = (bfs1.value << 1) | bfs1.flag;
    bfs2.value = temp & 0x3FF; /* Mask to 10 bits */
    
    /* Prevent dead code elimination */
    control = bfs1.flag + bfs2.value + nbf.d;
}

/* ==================== STRICT_LOW_PART patterns ==================== */

/* Test STRICT_LOW_PART patterns */
void test_strict_low_part(void) {
    volatile short vs1 = 0, vs2 = 0;
    volatile char vc1 = 0, vc2 = 0;
    volatile int vi = 1000;
    
    /* Partial register updates with smaller types */
    vs1 = (short)(vi + 500);
    vc1 = (char)(vs1 >> 2);
    
    /* Arithmetic on sub-word types */
    vs2 = vs1 + 100;
    vc2 = vc1 * 2;
    
    /* Mixed-size operations */
    vi = (int)vs1 + (int)vc1;
    vs1 = (short)(vi % 256); /* Force truncation */
    
    /* Loop with partial register updates */
    for (volatile char i = 0; i < 10; i++) {
        vc1 += i;
        vs1 += i * 10;
    }
    
    /* Prevent dead code elimination */
    control = vs1 + vs2 + vc1 + vc2 + vi;
}

/* ==================== SUBREG patterns ==================== */

/* Vector types for SUBREG patterns */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Union for type-punning */
union TypePun {
    unsigned int i;
    float f;
    struct {
        unsigned short low;
        unsigned short high;
    } parts;
};

/* Test SUBREG patterns */
void test_subreg(void) {
    /* Vector operations */
    v4si vec_int = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    v4sf vec_float = {1.0f, 2.0f, 3.0f, 4.0f};
    
    /* Vector element access (triggers SUBREG) */
    int elem1 = vec_int[2];
    short elem2 = vec_short[5];
    float elem3 = vec_float[1];
    
    /* Vector operations with mixing */
    vec_int[0] = (int)vec_short[0];
    vec_short[3] = (short)vec_int[3];
    
    /* Type-punning with union */
    union TypePun pun;
    pun.i = 0x3F800000; /* IEEE 754 representation of 1.0f */
    float from_int = pun.f;
    
    pun.f = 2.5f;
    unsigned int from_float = pun.i;
    
    /* Access partial register through union */
    unsigned short low_part = pun.parts.low;
    unsigned short high_part = pun.parts.high;
    pun.parts.high = low_part;
    
    /* Mixed float/int operations */
    int int_from_float = (int)pun.f;
    float float_from_int = (float)pun.i;
    
    /* Prevent dead code elimination */
    control = elem1 + elem2 + (int)elem3 + int_from_float + (int)float_from_int;
}

/* ==================== Combined patterns ==================== */

/* Structure combining bit-fields and regular members */
struct Combined {
    volatile unsigned int bits:4;
    volatile unsigned int more_bits:12;
    volatile short partial;
    volatile char small;
    int regular;
};

/* Test combined patterns */
void test_combined(void) {
    struct Combined comb = {0, 0, 0, 0, 0};
    union TypePun pun = {0};
    
    /* Combine bit-field with partial register */
    comb.bits = 7;
    comb.partial = (short)(comb.bits * 100);
    
    /* Combine with type-punning */
    pun.i = (comb.more_bits << 16) | comb.bits;
    comb.small = (char)pun.parts.low;
    
    /* Nested operations */
    comb.more_bits = (comb.partial >> 4) & 0xFFF;
    comb.regular = (int)comb.small + (int)comb.bits;
    
    /* Complex expression mixing patterns */
    v4si vec = {comb.regular, comb.partial, comb.small, comb.bits};
    int vec_elem = vec[comb.bits & 3];
    comb.partial = (short)vec_elem;
    
    /* Loop with combined patterns */
    for (volatile unsigned int i = 0; i < 8; i++) {
        comb.bits = i & 0xF;
        comb.small = (char)(comb.partial + i);
        comb.partial += comb.small;
    }
    
    /* Prevent dead code elimination */
    control = comb.bits + comb.more_bits + comb.partial + comb.small + comb.regular;
}

/* ==================== Architecture-specific patterns ==================== */

#ifdef __i386__
/* x86-specific patterns that may generate STRICT_LOW_PART */
void test_x86_specific(void) {
    volatile unsigned char byte1 = 0, byte2 = 0;
    volatile unsigned short word1 = 0, word2 = 0;
    volatile unsigned int dword1 = 0, dword2 = 0;
    
    /* Operations known to generate partial register updates on x86 */
    byte1 = 0xAA;
    byte2 = 0x55;
    word1 = (byte1 << 8) | byte2;
    
    /* Explicit byte operations */
    dword1 = 0x12345678;
    byte1 = (dword1 >> 16) & 0xFF;
    word1 = dword1 & 0xFFFF;
    
    /* Inline assembly that might generate partial register RTL */
    __asm__ volatile (
        "movb %1, %0\n\t"
        : "=r" (byte1)
        : "r" (byte2)
        : "cc"
    );
    
    control = byte1 + word1 + dword1;
}
#endif

#ifdef __arm__
/* ARM-specific patterns */
void test_arm_specific(void) {
    volatile unsigned int reg = 0;
    
    /* ARM often uses SUBREG for byte/short operations */
    volatile unsigned char c = 128;
    volatile unsigned short s = 32768;
    
    reg = (c << 8) | s;
    c = reg & 0xFF;
    s = (reg >> 16) & 0xFFFF;
    
    control = reg + c + s;
}
#endif

/* ==================== Main test driver ==================== */

/* Array of test functions */
typedef void (*test_func_t)(void);

static test_func_t test_functions[] = {
    test_zero_extract,
    test_strict_low_part,
    test_subreg,
    test_combined,
#ifdef __i386__
    test_x86_specific,
#endif
#ifdef __arm__
    test_arm_specific,
#endif
    NULL
};

int main(int argc, char *argv[]) {
    int num_tests = 0;
    
    /* Count available tests */
    while (test_functions[num_tests] != NULL) {
        num_tests++;
    }
    
    /* Run tests based on command line or all tests */
    if (argc > 1) {
        /* Run specific test if provided */
        int test_num = atoi(argv[1]);
        if (test_num >= 0 && test_num < num_tests) {
            test_functions[test_num]();
            printf("Ran test %d\n", test_num);
        } else {
            /* Run all tests */
            for (int i = 0; i < num_tests; i++) {
                test_functions[i]();
            }
            printf("Ran all %d tests\n", num_tests);
        }
    } else {
        /* Default: run all tests */
        for (int i = 0; i < num_tests; i++) {
            test_functions[i]();
        }
        printf("Ran all %d tests\n", num_tests);
    }
    
    /* Use the control variable to prevent optimization */
    printf("Control value: %d\n", control);
    
    /* Simple computation to ensure program is valid */
    int result = 0;
    for (int i = 0; i < 100; i++) {
        result += i;
    }
    
    return result > 0 ? 0 : 1;
}
