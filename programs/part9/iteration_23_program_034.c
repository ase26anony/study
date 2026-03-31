/* Test program to trigger specific RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int global_control = 0;
volatile int global_result = 0;

/* ==================== BIT-FIELD TESTS (ZERO_EXTRACT) ==================== */

struct BitFieldStruct {
    volatile unsigned int flag:1;
    volatile unsigned int value:10;
    volatile unsigned int mode:3;
    unsigned int padding:18;
};

struct NestedBitField {
    struct {
        volatile unsigned int a:4;
        volatile unsigned int b:4;
    } nibbles;
    volatile unsigned int full:8;
};

void test_bit_fields(void) {
    struct BitFieldStruct s1 = {0, 0, 0, 0};
    struct BitFieldStruct s2 = {0, 0, 0, 0};
    
    /* Multiple bit-field assignments */
    s1.flag = 1;
    s1.value = 511;  /* Max for 10 bits */
    s1.mode = 3;
    
    /* Bit-field to bit-field assignment */
    s2.flag = s1.flag;
    s2.value = s1.value >> 1;
    s2.mode = s1.mode | 1;
    
    /* Bit-field to integer */
    int x = s1.value;
    int y = s2.mode;
    
    /* Integer to bit-field with masking */
    s1.value = (x + y) & 0x3FF;
    
    /* Complex expression with bit-fields */
    global_result += s1.flag + s2.value * s1.mode;
    
    /* Nested bit-field structure */
    struct NestedBitField nbf;
    nbf.nibbles.a = 5;
    nbf.nibbles.b = 10;
    nbf.full = (nbf.nibbles.a << 4) | nbf.nibbles.b;
    
    global_result += nbf.full;
}

/* ==================== PARTIAL REGISTER TESTS (STRICT_LOW_PART) ==================== */

void test_partial_registers(void) {
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    volatile int vi = 1000;
    
    /* Casts to smaller types */
    vs1 = (short)vi;
    vs2 = (short)(vi * 2);
    
    /* Arithmetic on partial registers */
    vs1 = vs1 + 50;
    vs2 = vs2 - 25;
    
    /* Char operations */
    vc1 = (char)vi;
    vc2 = (char)(vi >> 8);
    vc1 = vc1 + vc2;
    
    /* Mixed-size operations */
    int temp = vs1 * vs2;
    vc1 = (char)(temp & 0xFF);
    
    /* Loop with partial register updates */
    for (volatile char i = 0; i < 10; i++) {
        vs1 = vs1 + i;
    }
    
    global_result += vs1 + vs2 + vc1 + vc2;
}

/* ==================== SUBREG TESTS ==================== */

/* Packed structure for SUBREG generation */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

/* Union for type-punning */
union TypePun {
    int i;
    float f;
    struct {
        short s1;
        short s2;
    } halves;
};

/* GCC vector extension */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

void test_subreg_patterns(void) {
    /* Packed structure access */
    struct PackedStruct ps;
    ps.a = 1;
    ps.b = 0x12345678;
    ps.c = 0x9ABC;
    
    /* This should generate SUBREG for misaligned access */
    int b_val = ps.b;
    short c_val = ps.c;
    
    /* Union type-punning */
    union TypePun up;
    up.i = 0x40000000;  /* 2.0 in float */
    float f_val = up.f;
    short s1_val = up.halves.s1;
    
    /* Vector operations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;
    
    /* Vector element access - should generate SUBREG */
    int elem = vec3[2];
    
    /* Vector with smaller elements */
    v8hi vec_short = {1, 2, 3, 4, 5, 6, 7, 8};
    vec_short[3] = 99;
    
    /* Float/double conversions */
    float f1 = 3.14159f;
    int f_as_int = *(int*)&f1;  /* Type punning */
    float f2 = *(float*)&f_as_int;
    
    global_result += b_val + c_val + (int)f_val + s1_val + elem + vec_short[3] + f_as_int;
}

/* ==================== COMBINED PATTERNS ==================== */

struct Combined {
    volatile unsigned int bits:8;
    volatile short half;
    volatile char quarter;
};

void test_combined_patterns(void) {
    struct Combined c;
    c.bits = 0xAB;
    c.half = 0x1234;
    c.quarter = 0x56;
    
    /* Bit-field to partial register */
    volatile short temp = c.bits;
    
    /* Partial register to bit-field */
    c.bits = temp & 0xFF;
    
    /* Complex expression mixing types */
    c.half = (c.bits << 8) | c.quarter;
    c.quarter = (c.half >> 4) & 0xF;
    
    /* Loop with combined operations */
    for (int i = 0; i < 4; i++) {
        c.bits = (c.bits >> 1) | ((c.bits & 1) << 7);
        c.half = c.half + c.quarter;
    }
    
    global_result += c.bits + c.half + c.quarter;
}

/* ==================== ARCHITECTURE-SPECIFIC TESTS ==================== */

#ifdef __i386__
void test_x86_specific(void) {
    volatile int a = 0x12345678;
    volatile short b;
    
    /* Inline assembly that might generate partial register ops */
    __asm__ volatile (
        "movw %1, %0\n\t"
        : "=r" (b)
        : "m" (a)
        : "memory"
    );
    
    /* Bit manipulation builtins */
    int count = __builtin_popcount(a);
    int parity = __builtin_parity(a);
    
    global_result += b + count + parity;
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    volatile int a = 0x12345678;
    volatile short b;
    
    /* ARM-specific operations */
    int rev = __builtin_bswap32(a);
    int clz = __builtin_clz(a);
    
    global_result += rev + clz;
}
#endif

/* ==================== MAIN TEST DRIVER ==================== */

typedef void (*test_func_t)(void);

const test_func_t test_functions[] = {
    test_bit_fields,
    test_partial_registers,
    test_subreg_patterns,
    test_combined_patterns,
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
    
    /* Use argc to control which tests run */
    int start_test = 0;
    int end_test = sizeof(test_functions)/sizeof(test_functions[0]) - 2; /* Exclude NULL */
    
    if (argc > 1) {
        start_test = atoi(argv[1]) % (end_test + 1);
    }
    if (argc > 2) {
        end_test = atoi(argv[2]) % (end_test + 1);
        if (end_test < start_test) {
            int temp = start_test;
            start_test = end_test;
            end_test = temp;
        }
    }
    
    /* Run selected tests */
    for (int i = start_test; i <= end_test && test_functions[i] != NULL; i++) {
        test_functions[i]();
        num_tests++;
    }
    
    /* Ensure the result is used */
    printf("Ran %d tests, result: %d\n", num_tests, global_result);
    
    /* Simple computation to ensure program is valid */
    int final_result = global_result % 256;
    return final_result;
}
