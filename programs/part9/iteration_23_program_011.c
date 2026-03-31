/* Test program to trigger specific RTL patterns in GCC's resource.cc */
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
    } nibbles;
    volatile unsigned int combined:8;
};

void test_bitfield_zero_extract(void) {
    struct BitFieldStruct s1 = {0};
    struct BitFieldStruct s2 = {0};
    
    /* Direct bit-field assignments - should generate ZERO_EXTRACT */
    s1.flag = 1;
    s1.value = 511;  /* Max for 10 bits */
    s1.mode = 3;
    
    /* Bit-field to bit-field assignment */
    s2.value = s1.value;
    s2.flag = s1.flag ^ 1;  /* XOR operation */
    
    /* Complex expression with bit-fields */
    unsigned int temp = s1.value + s2.value;
    s1.mode = (temp >> 2) & 0x7;  /* Shift and mask */
    
    /* Nested bit-field structure */
    struct NestedBitField nbf;
    nbf.nibbles.low = 0xA;
    nbf.nibbles.high = 0x5;
    nbf.combined = (nbf.nibbles.high << 4) | nbf.nibbles.low;
    
    /* Bit-field in loop */
    for (int i = 0; i < 4; i++) {
        s1.value = (s1.value << 1) | (s1.value >> 9);  /* Rotate */
        g_volatile_int = s1.flag;  /* Force reference */
    }
}

/* ==================== PARTIAL REGISTER TESTS (STRICT_LOW_PART) ==================== */

void test_partial_register_strict_low_part(void) {
    volatile short vs;
    volatile char vc;
    int i = 100;
    long l = 1000L;
    
    /* Assignments to smaller types - may generate STRICT_LOW_PART */
    vs = (short)i + 5;
    vc = (char)(i * 2);
    
    /* Arithmetic on partial registers */
    vs = vs * 2 + 1;
    vc = vc - 32;
    
    /* Mixed-size operations */
    short result = (short)((i & 0xFFFF) + (l & 0xFFFF));
    vs = result;
    
    /* Pointer casting to partial types */
    int array[4] = {1, 2, 3, 4};
    short *sp = (short *)array;
    vs = sp[1];  /* Access half of int */
    
    /* Union for type-punning */
    union {
        int full;
        struct {
            short low;
            short high;
        } halves;
    } pun;
    pun.full = 0x12345678;
    vs = pun.halves.low;
    
    /* Loop with partial register updates */
    for (char c = 'A'; c <= 'Z'; c++) {
        vc = c;
        g_volatile_char = vc;
    }
}

/* ==================== SUB-REGISTER TESTS (SUBREG) ==================== */

/* Vector type using GCC extension */
typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));

void test_subreg_patterns(void) {
    /* Vector operations - often generate SUBREG */
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;
    
    /* Access vector elements - generates SUBREG */
    int element = v3[2];
    g_volatile_int = element;
    
    /* Vector shuffle pattern */
    v8hi vshort = {1, 2, 3, 4, 5, 6, 7, 8};
    short first = vshort[0];
    short last = vshort[7];
    g_volatile_short = first + last;
    
    /* Float/int conversions - may involve SUBREG */
    float f = 3.14159f;
    volatile int fi;
    fi = *(int *)&f;  /* Type punning */
    
    double d = 2.71828;
    volatile long dl;
    dl = *(long *)&d;
    
    /* Packed structure */
    struct __attribute__((packed)) Packed {
        char a;
        int b;
        short c;
    } packed = {1, 2, 3};
    
    /* Accessing misaligned members may generate SUBREG */
    int b_val = packed.b;
    g_volatile_int = b_val + packed.c;
}

/* ==================== COMBINED PATTERN TESTS ==================== */

struct Combined {
    volatile unsigned int bits:8;
    volatile short half;
    volatile char quarter;
};

void test_combined_patterns(void) {
    struct Combined c = {0};
    
    /* Combined bit-field and partial register */
    c.bits = 0xAB;
    c.half = (short)c.bits * 2;  /* ZERO_EXTRACT -> STRICT_LOW_PART */
    
    /* Nested operations */
    unsigned int temp = c.bits;
    c.quarter = (char)((temp >> 4) & 0xF);  /* Extract to partial register */
    
    /* Union with bit-fields and full types */
    union {
        struct {
            unsigned int low:16;
            unsigned int high:16;
        } bits;
        unsigned int full;
        short halves[2];
    } u;
    
    u.full = 0xDEADBEEF;
    c.half = u.halves[0];  /* Multiple indirections */
    c.bits = u.bits.low & 0xFF;  /* Bit-field extract */
    
    /* Complex expression chain */
    for (int i = 0; i < 8; i++) {
        c.bits = (c.bits << 1) | (c.bits >> 7);  /* Rotate */
        c.quarter = (char)(c.bits & 0xF);  /* Extract to char */
        c.half = c.half + c.quarter;  /* Partial register update */
    }
}

/* ==================== ARCHITECTURE-SPECIFIC TESTS ==================== */

#ifdef __i386__
void test_x86_specific(void) {
    /* x86 inline assembly that might generate partial register ops */
    unsigned int eax_val;
    unsigned short ax_val;
    
    __asm__ volatile (
        "movl $0x12345678, %%eax\n"
        "movw %%ax, %0\n"
        : "=r" (ax_val)
        : 
        : "%eax"
    );
    
    g_volatile_short = ax_val;
    
    /* Bit test operations */
    unsigned int value = 0x89ABCDEF;
    unsigned char bit3;
    
    __asm__ volatile (
        "btl $3, %1\n"
        "setc %0\n"
        : "=r" (bit3)
        : "r" (value)
        : "cc"
    );
    
    g_volatile_char = bit3;
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    /* ARM may generate interesting patterns for byte/halfword access */
    volatile unsigned int word = 0x12345678;
    volatile unsigned short halfword;
    volatile unsigned char byte;
    
    /* Access parts of word - may generate SUBREG */
    halfword = (word >> 16) & 0xFFFF;
    byte = word & 0xFF;
    
    g_volatile_short = halfword;
    g_volatile_char = byte;
}
#endif

/* ==================== BUILTIN FUNCTION TESTS ==================== */

void test_builtin_functions(void) {
    unsigned int x = 0xF0F0F0F0;
    
    /* Builtins that work with bits */
    int leading_zeros = __builtin_clz(x);  /* Count leading zeros */
    int parity = __builtin_parity(x);      /* Parity of bits */
    int popcount = __builtin_popcount(x);  /* Population count */
    
    /* Use results to prevent optimization */
    g_volatile_int = leading_zeros + parity + popcount;
    
    /* Bit extraction builtin */
    unsigned int extracted = __builtin_extract(x, 4, 8);  /* Extract 8 bits from position 4 */
    g_volatile_int = extracted;
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
    NULL
};

int main(int argc, char *argv[]) {
    int test_to_run = 0;
    
    /* Use command line or random to select test */
    if (argc > 1) {
        test_to_run = atoi(argv[1]) % 5;
    } else {
        test_to_run = g_volatile_int % 5;
    }
    
    /* Run all tests in sequence to ensure code generation */
    for (int i = 0; test_functions[i] != NULL; i++) {
        if (test_to_run == -1 || test_to_run == i) {
            test_functions[i]();
        }
    }
    
    /* Final computation to ensure program does something useful */
    int result = g_volatile_int + g_volatile_short + g_volatile_char;
    
    /* Print to prevent dead code elimination */
    printf("Test result: %d\n", result);
    
    return result == 0 ? 0 : 1;
}
