/* Test program to trigger specific RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_int = 0;
volatile short g_volatile_short = 0;
volatile char g_volatile_char = 0;

/* ==================== BIT-FIELD TESTS (ZERO_EXTRACT) ==================== */

struct BitFieldStruct {
    volatile unsigned int flag:1;
    volatile unsigned int value:10;
    volatile unsigned int pad:5;
    volatile unsigned int mode:3;
};

struct NestedBitField {
    struct {
        volatile unsigned int a:4;
        volatile unsigned int b:4;
    } nibbles;
    volatile unsigned int full:16;
};

void test_bitfield_zero_extract(void) {
    struct BitFieldStruct s1 = {0};
    struct BitFieldStruct s2 = {0};
    
    /* Basic bit-field assignments - should generate ZERO_EXTRACT */
    s1.flag = 1;
    s1.value = 511;  /* Max 10-bit value */
    s1.mode = 7;
    
    /* Bit-field to bit-field copy */
    s2.flag = s1.flag;
    s2.value = s1.value + 1;
    
    /* Bit-field in expression */
    unsigned int temp = s1.value * 2;
    s2.pad = temp & 0x1F;
    
    /* Complex bit-field expression */
    s1.mode = (s1.flag << 2) | (s1.value & 0x3);
    
    /* Nested bit-field structure */
    struct NestedBitField nbf;
    nbf.nibbles.a = 5;
    nbf.nibbles.b = 10;
    nbf.full = (nbf.nibbles.a << 8) | nbf.nibbles.b;
    
    /* Cross-structure bit-field operations */
    s1.value = nbf.nibbles.a + nbf.nibbles.b;
    
    /* Prevent dead code elimination */
    g_volatile_int = s1.flag + s1.value + s1.mode;
}

/* ==================== PARTIAL REGISTER TESTS (STRICT_LOW_PART) ==================== */

void test_partial_register_strict_low_part(void) {
    volatile short vs = 0;
    volatile char vc = 0;
    volatile int vi = 1000;
    
    /* Cast to smaller types - likely generates STRICT_LOW_PART */
    vs = (short)(vi + 500);
    vc = (char)(vs * 2);
    
    /* Arithmetic on sub-word types */
    short s1 = 100, s2 = 200;
    vs = s1 + s2 - 50;
    
    char c1 = 10, c2 = 20;
    vc = c1 * c2;
    
    /* Mixed-type operations */
    vi = (int)vs * (int)vc;
    vs = (short)(vi / 10);
    
    /* Pointer to sub-word type */
    unsigned char *p = (unsigned char *)&vi;
    vc = p[0] + p[1];
    
    /* Loop with partial register updates */
    for (short i = 0; i < 10; i++) {
        vs += i;
        vc = (char)(vs & 0xFF);
    }
    
    /* Prevent optimization */
    g_volatile_short = vs;
    g_volatile_char = vc;
}

/* ==================== SUBREG TESTS ==================== */

/* Vector type for SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

/* Packed structure for SUBREG access */
struct PackedData {
    int a;
    short b;
    char c;
} __attribute__((packed));

union TypePun {
    float f;
    int i;
    short s[2];
};

void test_subreg_patterns(void) {
    /* Vector operations - often generate SUBREG */
    v4si vec_int = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Vector element access - SUBREG pattern */
    int elem = vec_int[2];
    short selem = vec_short[5];
    
    /* Vector operations */
    vec_int[0] = elem * 2;
    vec_short[3] = (short)(selem + 100);
    
    /* Packed structure access */
    struct PackedData pd;
    pd.a = 0x12345678;
    pd.b = 0x9ABC;
    pd.c = 0xDE;
    
    /* Accessing misaligned/mixed-size members */
    short b_copy = pd.b;
    char c_copy = pd.c;
    
    /* Type punning through union */
    union TypePun tp;
    tp.f = 3.14159f;
    
    /* Access different representations - SUBREG patterns */
    int int_view = tp.i;
    short short_view = tp.s[0];
    
    /* Float/int conversions */
    float f = (float)int_view;
    int i = (int)f;
    
    /* Mixed-size operations */
    long long ll = (long long)vec_int[0] * (long long)vec_int[1];
    int truncated = (int)(ll >> 16);
    
    /* Prevent optimization */
    g_volatile_int = elem + int_view + truncated;
    g_volatile_short = b_copy + short_view;
}

/* ==================== COMBINED PATTERN TESTS ==================== */

struct Combined {
    volatile unsigned int bitfield:8;
    volatile short partial;
    volatile int full;
};

void test_combined_patterns(void) {
    struct Combined c1, c2;
    
    /* Bit-field to partial register */
    c1.bitfield = 0xAB;
    c1.partial = (short)c1.bitfield * 2;
    
    /* Partial register to bit-field */
    c2.bitfield = (unsigned int)c1.partial & 0xFF;
    
    /* Complex expression combining patterns */
    c1.full = (c1.bitfield << 16) | (c1.partial & 0xFFFF);
    c2.partial = (short)((c1.full >> 8) & 0xFF);
    
    /* Loop with combined operations */
    for (int i = 0; i < 5; i++) {
        c1.bitfield = (c1.bitfield + i) & 0xFF;
        c1.partial = (short)(c1.partial - c1.bitfield);
        c1.full = c1.full ^ (c1.bitfield << 8);
    }
    
    /* Union with bit-field */
    union {
        struct {
            unsigned int low:16;
            unsigned int high:16;
        } bits;
        volatile int full;
    } u;
    
    u.full = 0x12345678;
    u.bits.low = u.bits.high ^ 0x5555;
    
    /* Prevent optimization */
    g_volatile_int = c1.full + c2.full + u.full;
}

/* ==================== ARCHITECTURE-SPECIFIC TESTS ==================== */

#ifdef __i386__
void test_x86_specific(void) {
    /* x86 inline assembly that might generate partial register ops */
    unsigned int eax_val;
    unsigned short ax_val;
    
    __asm__ volatile (
        "movl $0x12345678, %%eax\n\t"
        "movw %%ax, %0\n\t"
        "movl %%eax, %1\n\t"
        : "=m"(ax_val), "=m"(eax_val)
        :
        : "%eax"
    );
    
    /* Use the results */
    g_volatile_short = ax_val;
    g_volatile_int = eax_val;
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    /* ARM specific - accessing halfwords */
    volatile uint32_t word = 0xA5A5A5A5;
    volatile uint16_t halfword;
    
    /* Cast to halfword - may generate SUBREG */
    halfword = (uint16_t)(word >> 8);
    
    /* ARM inline assembly for partial register access */
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n\t"
        "uxth %0, r0\n\t"
        : "=r"(result)
        : "r"(word)
        : "r0"
    );
    
    g_volatile_int = result + halfword;
}
#endif

/* ==================== BUILTIN FUNCTION TESTS ==================== */

void test_builtin_functions(void) {
    unsigned int x = 0x12345678;
    
    /* Builtins that involve bit manipulation */
    int count_leading_zeros = __builtin_clz(x);
    int parity = __builtin_parity(x);
    int popcount = __builtin_popcount(x);
    
    /* Bit extraction builtins */
    unsigned int extracted = __builtin_extract_bits(x, 0x0F00);
    
    /* Rotate operations */
    unsigned int rotated = __builtin_rotateright32(x, 4);
    
    /* Combine results */
    g_volatile_int = count_leading_zeros + parity + popcount + extracted + rotated;
}

/* ==================== MAIN TEST DRIVER ==================== */

typedef void (*test_func_t)(void);

const test_func_t test_functions[] = {
    test_bitfield_zero_extract,
    test_partial_register_strict_low_part,
    test_subreg_patterns,
    test_combined_patterns,
    test_builtin_functions,
#ifdef __i386__
    test_x86_specific,
#endif
#ifdef __arm__
    test_arm_specific,
#endif
};

int main(int argc, char *argv[]) {
    int num_tests = sizeof(test_functions) / sizeof(test_functions[0]);
    
    /* Use volatile to prevent compile-time optimization of test selection */
    volatile int start_test = 0;
    volatile int end_test = num_tests;
    
    /* If arguments provided, use them to select tests */
    if (argc > 1) {
        start_test = argv[1][0] - '0';
        if (start_test < 0) start_test = 0;
        if (start_test >= num_tests) start_test = num_tests - 1;
    }
    if (argc > 2) {
        end_test = argv[2][0] - '0';
        if (end_test < start_test) end_test = start_test;
        if (end_test > num_tests) end_test = num_tests;
    }
    
    /* Run selected tests */
    for (int i = start_test; i < end_test; i++) {
        test_functions[i]();
    }
    
    /* Final computation to ensure program does something useful */
    int result = g_volatile_int + g_volatile_short + g_volatile_char;
    
    printf("Test completed. Result: %d\n", result);
    
    /* Return non-zero if no tests were run */
    return (end_test - start_test) > 0 ? 0 : 1;
}
