/* Test program to trigger uncovered RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_int = 0;
volatile short g_volatile_short = 0;
volatile char g_volatile_char = 0;

/* ==================== BIT-FIELD TESTS (ZERO_EXTRACT) ==================== */

/* Test 1: Basic bit-field operations */
struct BitFieldStruct {
    volatile unsigned int flag:1;
    volatile unsigned int value:10;
    volatile unsigned int pad:5;
    volatile unsigned int mode:3;
};

void test_bitfield_basic(void) {
    struct BitFieldStruct s = {0};
    
    /* Multiple bit-field assignments */
    s.flag = 1;
    s.value = 512;
    s.mode = 3;
    
    /* Bit-field reads */
    int x = s.flag;
    int y = s.value;
    int z = s.mode;
    
    /* Cross assignments */
    s.value = x + y;
    s.mode = z | 1;
    
    /* Prevent optimization */
    g_volatile_int = x + y + z;
}

/* Test 2: Complex bit-field expressions */
struct NestedBitFields {
    struct {
        volatile unsigned int a:4;
        volatile unsigned int b:4;
    } inner;
    volatile unsigned int outer:8;
};

void test_bitfield_complex(void) {
    struct NestedBitFields nb = {0};
    
    /* Nested bit-field access */
    nb.inner.a = 7;
    nb.inner.b = 8;
    nb.outer = nb.inner.a + nb.inner.b;
    
    /* Bit-field in conditional */
    if (nb.inner.a > 3) {
        nb.outer = 15;
    }
    
    /* Loop with bit-field update */
    for (int i = 0; i < 4; i++) {
        nb.inner.a = i;
        nb.inner.b = 7 - i;
    }
    
    g_volatile_int = nb.outer;
}

/* ==================== PARTIAL REGISTER TESTS (STRICT_LOW_PART) ==================== */

/* Test 3: Sub-word type operations */
void test_partial_register(void) {
    volatile short vs = 0;
    volatile char vc = 0;
    
    /* Casts to smaller types */
    int i = g_volatile_int;
    vs = (short)(i + 100);
    vc = (char)(i & 0xFF);
    
    /* Arithmetic on sub-word types */
    short s1 = 100;
    short s2 = 200;
    vs = s1 + s2;
    vs = vs * 2;
    
    char c1 = 50;
    char c2 = 60;
    vc = c1 - c2;
    
    /* Mixed-size operations */
    vs = (short)(vc * 2);
    vc = (char)(vs >> 2);
    
    g_volatile_short = vs;
    g_volatile_char = vc;
}

/* Test 4: Architecture-specific partial register patterns */
#ifdef __i386__
void test_x86_partial_reg(void) {
    volatile short vs;
    volatile char vc;
    
    /* Operations likely to generate partial register updates on x86 */
    int x = g_volatile_int;
    
    /* These often compile to MOVZX/MOVSX with partial register updates */
    vs = (short)(x * 2);
    vc = (char)(x / 3);
    
    /* Byte operations */
    unsigned char uc1 = 0xAA;
    unsigned char uc2 = 0x55;
    vc = uc1 & uc2;
    vc = uc1 | uc2;
    vc = uc1 ^ uc2;
    
    g_volatile_short = vs;
}
#endif

/* ==================== SUBREG TESTS ==================== */

/* Test 5: Vector extensions */
typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));

void test_vector_subreg(void) {
    v4si v = {1, 2, 3, 4};
    v8hi w = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Vector element access - often generates SUBREG */
    int elem1 = v[0];
    int elem2 = v[2];
    short selem1 = w[1];
    short selem3 = w[3];
    
    /* Vector operations */
    v4si v2 = v + 1;
    v8hi w2 = w * 2;
    
    /* Cross-type access */
    v[1] = elem1 + elem2;
    w[4] = selem1 + selem3;
    
    g_volatile_int = v[0] + w[0];
}

/* Test 6: Type punning through unions */
union TypePun {
    volatile int full;
    struct {
        volatile short low;
        volatile short high;
    } parts;
    volatile char bytes[4];
};

void test_union_subreg(void) {
    union TypePun u;
    u.full = 0x12345678;
    
    /* Access different views of the same data */
    short low_part = u.parts.low;
    short high_part = u.parts.high;
    char first_byte = u.bytes[0];
    char last_byte = u.bytes[3];
    
    /* Modify through different views */
    u.parts.low = 0xABCD;
    u.bytes[2] = 0xEF;
    
    /* Mixed operations */
    u.full = u.parts.low + (u.parts.high << 16);
    
    g_volatile_int = u.full;
}

/* ==================== COMBINED PATTERN TESTS ==================== */

/* Test 7: Combined bit-field and partial register */
struct Combined {
    volatile unsigned int data:16;
    volatile unsigned int control:8;
};

void test_combined_patterns(void) {
    struct Combined c = {0};
    
    /* Bit-field to partial register */
    c.data = 0xABCD;
    c.control = 0xEF;
    
    /* Extract to smaller type */
    volatile short extracted;
    extracted = (short)c.data;
    
    /* Modify and store back */
    c.data = extracted + 1;
    c.control = (unsigned char)(extracted >> 8);
    
    /* Complex expression */
    int temp = g_volatile_int;
    c.data = (temp & 0xFFFF) | ((temp >> 8) & 0xFF);
    
    g_volatile_short = extracted;
}

/* Test 8: Builtin functions with bit manipulation */
void test_builtin_bitops(void) {
    unsigned int x = g_volatile_int;
    
    /* Builtins that may involve bit extraction */
    int count = __builtin_popcount(x);
    int parity = __builtin_parity(x);
    int leading_zeros = __builtin_clz(x);
    int trailing_zeros = __builtin_ctz(x);
    
    /* Combine results */
    volatile short result;
    result = (short)(count + parity);
    
    /* Bit reversal pattern */
    unsigned int reversed = 0;
    for (int i = 0; i < 32; i++) {
        if (x & (1 << i)) {
            reversed |= 1 << (31 - i);
        }
    }
    
    g_volatile_int = reversed + result;
}

/* ==================== MAIN TEST DRIVER ==================== */

typedef void (*test_func_t)(void);

const test_func_t test_functions[] = {
    test_bitfield_basic,
    test_bitfield_complex,
    test_partial_register,
#ifdef __i386__
    test_x86_partial_reg,
#endif
    test_vector_subreg,
    test_union_subreg,
    test_combined_patterns,
    test_builtin_bitops,
    NULL
};

int main(int argc, char *argv[]) {
    int test_to_run = 0;
    
    /* Use command line or volatile to control test selection */
    if (argc > 1) {
        test_to_run = atoi(argv[1]) % 8;
    } else {
        test_to_run = g_volatile_int % 8;
    }
    
    /* Run all tests in sequence to maximize coverage */
    for (int i = 0; test_functions[i] != NULL; i++) {
        if (test_to_run == -1 || test_to_run == i) {
            test_functions[i]();
        }
    }
    
    /* Final computation to ensure program does something useful */
    int result = g_volatile_int + g_volatile_short + g_volatile_char;
    
    printf("Test completed. Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
