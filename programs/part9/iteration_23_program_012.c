/* Test program to trigger specific RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_int = 0;
volatile short g_volatile_short = 0;
volatile char g_volatile_char = 0;

/* ========== Test 1: Bit-field operations for ZERO_EXTRACT ========== */

struct BitFieldStruct {
    volatile unsigned int flag:1;
    volatile unsigned int value:10;
    unsigned int pad:21;
    volatile unsigned int mode:4;
    unsigned int :0; /* Force alignment */
};

struct NestedBitField {
    struct {
        volatile unsigned int a:3;
        volatile unsigned int b:5;
    } inner;
    volatile unsigned int c:8;
};

void test_bitfield_extract(void) {
    struct BitFieldStruct s1 = {0};
    struct BitFieldStruct s2 = {0};
    
    /* Direct bit-field assignments - should generate ZERO_EXTRACT */
    s1.flag = 1;
    s1.value = 511; /* Max 10-bit value */
    s1.mode = 7;
    
    /* Bit-field to bit-field assignment */
    s2.value = s1.value;
    s2.flag = s1.flag ^ 1;
    
    /* Complex expression with bit-fields */
    unsigned int temp = (s1.value << 3) | s1.mode;
    s2.mode = temp & 0xF;
    
    /* Prevent dead code elimination */
    g_volatile_int = s1.flag + s2.value;
}

void test_nested_bitfield(void) {
    struct NestedBitField nbf = {0};
    
    /* Nested structure bit-field access */
    nbf.inner.a = 5;
    nbf.inner.b = 20;
    nbf.c = nbf.inner.a + nbf.inner.b;
    
    /* Bit-field in conditional */
    if (nbf.inner.a > 3) {
        nbf.inner.b = nbf.inner.b >> 1;
    }
    
    g_volatile_int = nbf.c;
}

/* ========== Test 2: Partial register operations for STRICT_LOW_PART ========== */

void test_partial_register(void) {
    volatile short vs = 0;
    volatile char vc = 0;
    int i = g_volatile_int;
    
    /* Cast to smaller types - may generate STRICT_LOW_PART */
    vs = (short)(i + 1000);
    vc = (char)(i & 0xFF);
    
    /* Arithmetic on partial registers */
    vs = vs + (short)50;
    vc = vc - 1;
    
    /* Mixed-size operations */
    short result = (short)((vs * 2) + vc);
    g_volatile_short = result;
    
    /* Pointer to partial type */
    volatile short *ps = &vs;
    *ps = *ps + 10;
}

/* Architecture-specific partial register tests */
#ifdef __i386__
void test_x86_partial_reg(void) {
    volatile short s;
    volatile char c;
    
    /* These operations often generate partial register updates on x86 */
    asm volatile ("movw %1, %0" : "=r" (s) : "r" ((short)0x1234));
    asm volatile ("movb %1, %0" : "=r" (c) : "r" ((char)0x56));
    
    g_volatile_short = s;
    g_volatile_char = c;
}
#endif

#ifdef __arm__
void test_arm_partial_reg(void) {
    volatile short s;
    /* ARM may use STRICT_LOW_PART for 16-bit stores */
    asm volatile ("mov %0, #0x1234" : "=r" (s));
    g_volatile_short = s;
}
#endif

/* ========== Test 3: Sub-register accesses for SUBREG ========== */

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
    } shorts;
};

/* GCC vector extension */
typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));

void test_subreg_operations(void) {
    /* Packed structure access */
    struct PackedStruct ps = {0};
    ps.a = 10;
    ps.b = 0x12345678;
    ps.c = (short)ps.b; /* This may involve SUBREG */
    
    /* Union type-punning */
    union TypePun up;
    up.i = 0x40490FDB; /* Approx pi */
    float f = up.f;    /* Bit reinterpretation */
    up.shorts.s1 = 0x1234;
    up.shorts.s2 = 0x5678;
    
    /* Vector operations */
    v4si vec = {1, 2, 3, 4};
    v8hi vec_short = {1, 2, 3, 4, 5, 6, 7, 8};
    
    /* Vector element access - often uses SUBREG */
    int elem = vec[2];
    short selem = vec_short[5];
    
    /* Vector operations that may create SUBREG patterns */
    vec = vec + 1;
    vec_short = vec_short << 2;
    
    /* Mixed vector/scalar */
    g_volatile_int = elem + selem + ps.c;
}

void test_float_conversions(void) {
    /* Float/int conversions often involve SUBREG */
    volatile float fv = 3.14159f;
    volatile double dv = 2.71828;
    
    /* Type punning through unions */
    union {
        float f;
        unsigned int i;
    } converter;
    
    converter.f = fv;
    unsigned int bits = converter.i; /* This may use SUBREG */
    
    /* Size-changing operations */
    int int_from_float = (int)fv;
    float float_from_int = (float)g_volatile_int;
    
    g_volatile_int = bits + int_from_float;
}

/* ========== Test 4: Combined patterns ========== */

struct Combined {
    volatile unsigned int low:8;
    volatile unsigned int high:8;
    volatile short full;
};

void test_combined_patterns(void) {
    struct Combined c = {0};
    
    /* Bit-field to partial register */
    c.low = 0xAB;
    c.high = 0xCD;
    c.full = (c.high << 8) | c.low; /* Combines ZERO_EXTRACT and SUBREG/STRICT_LOW_PART */
    
    /* Partial register to bit-field */
    volatile short temp = c.full;
    c.low = temp & 0xFF;
    c.high = (temp >> 8) & 0xFF;
    
    /* Complex expression with all patterns */
    union TypePun u;
    u.i = (c.high << 24) | (c.low << 16) | (temp & 0xFFFF);
    float result = u.f * 2.0f;
    
    /* Use result to prevent optimization */
    g_volatile_int = (int)result;
}

void test_loop_combined(void) {
    struct BitFieldStruct bfs[4];
    volatile short outputs[4];
    
    /* Loop with mixed operations */
    for (int i = 0; i < 4; i++) {
        bfs[i].value = i * 100;
        bfs[i].flag = i & 1;
        
        /* Bit-field to short (partial register) */
        outputs[i] = (short)bfs[i].value;
        
        /* Modify through pointer with partial type */
        volatile short *ptr = &outputs[i];
        *ptr = *ptr + bfs[i].flag;
    }
    
    g_volatile_int = outputs[0] + outputs[3];
}

/* ========== Test 5: Builtin functions ========== */

void test_builtins(void) {
    unsigned int x = g_volatile_int;
    
    /* Builtins that may involve bit manipulation */
    int count = __builtin_popcount(x);
    int parity = __builtin_parity(x);
    int clz = __builtin_clz(x);
    int ctz = __builtin_ctz(x | 1); /* Avoid undefined behavior */
    
    /* Bit extraction builtins */
    unsigned int extracted = __builtin_extract_bits(x, 0x0F0F);
    
    g_volatile_int = count + parity + clz + ctz + extracted;
}

/* ========== Main test driver ========== */

typedef void (*test_func_t)(void);

struct TestCase {
    const char *name;
    test_func_t func;
};

#define NUM_TESTS 10

int main(int argc, char *argv[]) {
    struct TestCase tests[NUM_TESTS] = {
        {"bitfield_extract", test_bitfield_extract},
        {"nested_bitfield", test_nested_bitfield},
        {"partial_register", test_partial_register},
        {"subreg_operations", test_subreg_operations},
        {"float_conversions", test_float_conversions},
        {"combined_patterns", test_combined_patterns},
        {"loop_combined", test_loop_combined},
        {"builtins", test_builtins},
    };
    
    int num_tests = 8;
    
#ifdef __i386__
    tests[num_tests++] = (struct TestCase){"x86_partial_reg", test_x86_partial_reg};
#endif
    
#ifdef __arm__
    tests[num_tests++] = (struct TestCase){"arm_partial_reg", test_arm_partial_reg};
#endif
    
    /* Use command line argument or iterate through all tests */
    if (argc > 1) {
        int test_num = atoi(argv[1]);
        if (test_num >= 0 && test_num < num_tests) {
            printf("Running test: %s\n", tests[test_num].name);
            tests[test_num].func();
        } else {
            /* Run all tests */
            for (int i = 0; i < num_tests; i++) {
                printf("Running test %d: %s\n", i, tests[i].name);
                tests[i].func();
            }
        }
    } else {
        /* Default: run all tests */
        for (int i = 0; i < num_tests; i++) {
            tests[i].func();
        }
    }
    
    /* Final computation to ensure program does something */
    int result = g_volatile_int + g_volatile_short + g_volatile_char;
    printf("Final result: %d\n", result);
    
    return result != 0;
}
