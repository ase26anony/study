/* Test program to trigger uncovered RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int g_control = 0;
volatile int g_result = 0;

/* ==================== BIT-FIELD PATTERNS (ZERO_EXTRACT) ==================== */

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

void test_bit_fields(void) {
    struct BitFieldStruct s1 = {0};
    struct BitFieldStruct s2 = {1, 512, 7, 0};
    
    /* Multiple bit-field assignments */
    s1.flag = s2.flag;
    s1.value = s2.value & 0x1FF;  /* Complex expression with bit-field */
    s1.mode = (s2.mode + 1) & 0x7;
    
    /* Cross-structure bit-field operations */
    unsigned int temp = s1.value;
    s2.value = temp | 0x100;
    
    /* Nested bit-field structure */
    struct NestedBitField nbf = {{3, 12}, 0xABCD};
    nbf.byte.low = nbf.byte.high ^ 0xF;
    nbf.word = (nbf.word << 4) | nbf.byte.low;
    
    g_result += s1.value + s2.flag + nbf.word;
}

/* ==================== PARTIAL REGISTER PATTERNS (STRICT_LOW_PART) ==================== */

void test_partial_registers(void) {
    volatile char vc1 = 0, vc2 = 0;
    volatile short vs1 = 0, vs2 = 0;
    volatile int vi = 1000;
    
    /* Partial register assignments with arithmetic */
    vc1 = (char)(vi + 255);  /* May wrap, creating partial update */
    vc2 = vc1 * 2 + 1;
    
    vs1 = (short)(vi * 2);
    vs2 = (short)(vs1 - 1000);
    
    /* Mixed-size operations */
    vi = (int)vc1 + (int)vs1;
    
    /* Complex expression with partial types */
    volatile short vs3 = (short)((vc1 << 8) | vc2);
    
    g_result += vc1 + vc2 + vs1 + vs2 + vs3;
}

/* ==================== SUBREG PATTERNS ==================== */

/* Packed structure forcing sub-register access */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

/* Union for type-punning */
union TypePun {
    uint32_t word;
    struct {
        uint16_t low;
        uint16_t high;
    } halves;
    float f;
};

/* GCC vector extension for SUBREG patterns */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

void test_subreg_patterns(void) {
    /* Packed structure access */
    struct PackedStruct ps = {1, 0x12345678, 0x9ABC};
    ps.b = ps.b + ps.a;  /* Mixed-size access in packed struct */
    
    /* Union type-punning */
    union TypePun pun;
    pun.word = 0x3F800000;  /* IEEE 754 1.0f */
    pun.halves.low = pun.halves.high ^ 0xFFFF;
    float f = pun.f * 2.0f;
    
    /* Vector operations */
    v4si vec_int = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Vector element access - likely generates SUBREG */
    int elem = vec_int[2];
    vec_int[1] = elem * 2;
    
    /* Vector-scalar operations */
    vec_short[3] = (short)vec_int[0];
    
    /* Mixed vector operations */
    for (int i = 0; i < 4; i++) {
        vec_int[i] += (int)vec_short[i*2];
    }
    
    g_result += ps.b + (int)f + elem + vec_int[0];
}

/* ==================== COMBINED COMPLEX PATTERNS ==================== */

struct ComplexBitfield {
    volatile unsigned int data:24;
    volatile unsigned int tag:8;
};

void test_combined_patterns(void) {
    /* Bit-field to partial register */
    struct ComplexBitfield cb = {0x123456, 0x78};
    volatile short partial;
    
    /* Extract portion of bit-field to short */
    partial = (short)(cb.data >> 8);
    cb.tag = (unsigned int)partial & 0xFF;
    
    /* Union with bit-fields */
    union {
        struct {
            volatile unsigned int low:16;
            volatile unsigned int high:16;
        } bits;
        volatile uint32_t word;
        volatile float float_val;
    } u;
    
    u.word = 0x40000000;  /* IEEE 754 2.0f */
    u.bits.low = u.bits.high ^ 0xAAAA;
    
    /* Complex expression mixing patterns */
    u.word = (u.word << 4) | (cb.data & 0xF);
    
    /* Vector with bit-field source */
    v4si vec = {0};
    vec[0] = (int)cb.data;
    vec[1] = (int)u.bits.low;
    vec[2] = (int)partial;
    
    g_result += partial + u.word + vec[0];
}

/* ==================== ARCHITECTURE-SPECIFIC PATTERNS ==================== */

#ifdef __i386__
void test_x86_specific(void) {
    volatile uint32_t result;
    
    /* Inline assembly that might generate STRICT_LOW_PART */
    __asm__ volatile (
        "movw $0x1234, %%ax\n\t"
        "movw %%ax, %0\n\t"
        : "=m" (result)
        : 
        : "ax"
    );
    
    /* Bit test and set */
    volatile uint32_t value = 0x12345678;
    uint8_t bit;
    __asm__ volatile (
        "btl $5, %1\n\t"
        "setc %0\n\t"
        : "=r" (bit)
        : "r" (value)
        : "cc"
    );
    
    g_result += result + bit;
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    volatile uint32_t val = 0;
    
    /* ARM inline assembly with byte operations */
    __asm__ volatile (
        "mov r0, #0x55\n\t"
        "strb r0, [%0]\n\t"
        : 
        : "r" (&val)
        : "r0", "memory"
    );
    
    g_result += val;
}
#endif

/* ==================== BUILTIN FUNCTIONS ==================== */

void test_builtins(void) {
    volatile unsigned int x = 0x12345678;
    
    /* Builtins that may involve bit extraction */
    int leading_zeros = __builtin_clz(x);
    int parity = __builtin_parity(x);
    int popcount = __builtin_popcount(x);
    
    /* Bit field insert/extract builtins */
    unsigned int extracted = __builtin_ibit_extract(x, 8, 8);
    unsigned int inserted = __builtin_ibit_insert(x, 0xAA, 16, 8);
    
    g_result += leading_zeros + parity + popcount + extracted + inserted;
}

/* ==================== MAIN TEST DRIVER ==================== */

typedef void (*test_func_t)(void);

const test_func_t test_functions[] = {
    test_bit_fields,
    test_partial_registers,
    test_subreg_patterns,
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
    
    /* Use command line or global to control which tests run */
    if (argc > 1) {
        test_to_run = atoi(argv[1]) % 10;
    } else {
        test_to_run = g_control;
    }
    
    /* Run all tests or specific ones based on input */
    if (test_to_run == 0) {
        /* Run all tests */
        for (int i = 0; test_functions[i] != NULL; i++) {
            test_functions[i]();
        }
    } else {
        /* Run specific test based on input */
        int idx = (test_to_run - 1) % 5;
        if (test_functions[idx] != NULL) {
            test_functions[idx]();
        }
    }
    
    /* Ensure the program does real work */
    printf("Result: %d\n", g_result);
    
    /* Additional loop to ensure RTL generation for control flow */
    for (int i = 0; i < 10; i++) {
        g_result += i * (g_result & 1);
    }
    
    return g_result > 0 ? 0 : 1;
}
