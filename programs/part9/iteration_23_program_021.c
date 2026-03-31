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

void test_bitfield_zero_extract(void) {
    struct BitFieldStruct s1 = {0};
    struct BitFieldStruct s2 = {0};
    
    /* Basic bit-field assignments - should generate ZERO_EXTRACT */
    s1.flag = 1;
    s1.value = 511;  /* Max for 10 bits */
    s1.mode = 3;
    
    /* Bit-field to bit-field assignment */
    s2.flag = s1.flag;
    s2.value = s1.value + 1;
    
    /* Bit-field in expression */
    int x = s1.value * 2;
    g_volatile_int = x;
    
    /* Complex bit-field expression */
    s1.mode = (s1.flag << 2) | (s2.flag << 1);
    
    /* Nested bit-field structure */
    struct NestedBitField nbf;
    nbf.nibbles.a = 5;
    nbf.nibbles.b = 10;
    nbf.full = nbf.nibbles.a + nbf.nibbles.b;
    
    g_volatile_int = nbf.full;
}

/* ==================== PARTIAL REGISTER TESTS (STRICT_LOW_PART) ==================== */

void test_partial_register_strict_low_part(void) {
    volatile short vs;
    volatile char vc;
    
    /* Casts to smaller types - often generate STRICT_LOW_PART */
    int i = g_volatile_int;
    vs = (short)(i + 1000);
    vc = (char)(i + 50);
    
    /* Arithmetic on sub-word types */
    short s1 = 100, s2 = 200;
    vs = s1 + s2;
    vs = vs * 2;
    
    char c1 = 10, c2 = 20;
    vc = c1 * c2;
    
    /* Mixed-size operations */
    vs = (short)((int)vs + (int)vc);
    
    /* Pointer to sub-word type */
    unsigned char *ptr = (unsigned char*)&g_volatile_int;
    vc = ptr[0] + ptr[1];
    
    /* Loop with partial register updates */
    for (short j = 0; j < 10; j++) {
        vs = j * 10;
        g_volatile_short = vs;
    }
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
    uint32_t full;
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
    char bytes[4];
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
    
    /* Accessing misaligned members generates SUBREG */
    int b_val = ps.b;  /* May require unaligned access */
    short c_val = ps.c;
    g_volatile_int = b_val + c_val;
    
    /* Union type-punning */
    union TypePun pun;
    pun.full = 0xDEADBEEF;
    pun.parts.low = 0xCAFE;
    g_volatile_short = pun.parts.high;
    
    /* Access individual bytes - generates SUBREG */
    char first_byte = pun.bytes[0];
    g_volatile_char = first_byte;
    
    /* Vector operations */
    v4si vec_int = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Vector element access generates SUBREG */
    int elem = vec_int[2];
    vec_int[3] = elem * 2;
    
    short s_elem = vec_short[5];
    g_volatile_short = s_elem;
    
    /* Vector-scalar operations */
    vec_int = vec_int + 5;
    vec_short = vec_short * 2;
    
    /* Float/integer conversions */
    float f = 3.14159f;
    uint32_t f_bits;
    memcpy(&f_bits, &f, sizeof(f_bits));
    g_volatile_int = f_bits;
}

/* ==================== COMBINED PATTERNS ==================== */

struct Combined {
    volatile unsigned int bits:8;
    volatile short half;
    volatile char byte;
};

void test_combined_patterns(void) {
    struct Combined c;
    
    /* Bit-field to partial register */
    c.bits = 0x7F;
    g_volatile_short = (short)c.bits;  /* ZERO_EXTRACT -> SUBREG/STRICT_LOW_PART */
    
    /* Partial register to bit-field */
    c.half = 0x1234;
    c.bits = (c.half & 0xFF);  /* STRICT_LOW_PART -> ZERO_EXTRACT */
    
    /* Complex expression with multiple patterns */
    c.byte = (char)((c.bits << 1) | (c.half & 1));
    
    /* Union with bit-fields */
    union {
        struct {
            volatile unsigned int low:16;
            volatile unsigned int high:16;
        } bits;
        volatile uint32_t full;
    } u;
    
    u.full = 0x12345678;
    u.bits.low = (u.bits.high & 0xFF) << 8;
    g_volatile_int = u.full;
}

/* ==================== ARCHITECTURE-SPECIFIC TESTS ==================== */

#ifdef __i386__
void test_x86_specific(void) {
    /* x86 inline assembly with partial register output */
    unsigned int result;
    unsigned short s_result;
    
    __asm__ volatile (
        "movl $0x12345678, %%eax\n\t"
        "movw %%ax, %0\n\t"
        : "=r" (s_result)
        :
        : "%eax"
    );
    
    g_volatile_short = s_result;
    
    /* Bit test and set */
    unsigned int value = 0x0F0F0F0F;
    unsigned char bit;
    
    __asm__ volatile (
        "btl $8, %1\n\t"
        "setc %0\n\t"
        : "=r" (bit)
        : "r" (value)
        : "cc"
    );
    
    g_volatile_char = bit;
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    /* ARM inline assembly with byte/halfword operations */
    unsigned int val = 0x12345678;
    unsigned short halfword;
    
    __asm__ volatile (
        "uxth %0, %1\n\t"
        : "=r" (halfword)
        : "r" (val)
    );
    
    g_volatile_short = halfword;
}
#endif

/* ==================== BUILTIN FUNCTION TESTS ==================== */

void test_builtin_functions(void) {
    unsigned int x = g_volatile_int;
    
    /* Builtins that may generate bit-field-like operations */
    int leading_zeros = __builtin_clz(x);
    int parity = __builtin_parity(x);
    int popcount = __builtin_popcount(x);
    
    /* Bit extraction builtin */
    unsigned int extracted = __builtin_extract_bits(x, 0x0F0F);
    
    g_volatile_int = leading_zeros + parity + popcount + extracted;
    
    /* Bit field insert */
    unsigned int src = 0x00000ABC;
    unsigned int dst = 0x12345678;
    unsigned int result = __builtin_ibit_insert(dst, src, 8, 12);
    
    g_volatile_int = result;
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
    volatile int test_selector = 0;
    
    if (argc > 1) {
        test_selector = argv[1][0] - '0';
    }
    
    /* Run all tests or specific test based on selector */
    if (test_selector == 0) {
        /* Run all tests */
        for (size_t i = 0; i < sizeof(test_functions)/sizeof(test_functions[0]); i++) {
            test_functions[i]();
        }
    } else {
        /* Run specific test */
        size_t idx = test_selector - 1;
        if (idx < sizeof(test_functions)/sizeof(test_functions[0])) {
            test_functions[idx]();
        }
    }
    
    /* Ensure some computation happens to prevent dead code elimination */
    int result = g_volatile_int + g_volatile_short + g_volatile_char;
    
    printf("Test completed. Result: %d\n", result);
    
    /* Use result in a way that can't be optimized away */
    if (result > 1000) {
        printf("Large result detected\n");
    }
    
    return result & 0xFF;  /* Return non-zero to indicate execution */
}
