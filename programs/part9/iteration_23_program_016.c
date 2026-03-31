/* Test program to trigger specific RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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
    volatile unsigned int full:16;
};

void test_bitfield_zero_extract(void) {
    struct BitFieldStruct s1 = {0};
    struct BitFieldStruct s2 = {1, 512, 7, 0};
    
    /* Direct assignments to bit-fields */
    s1.flag = 1;
    s1.value = 255;
    s1.mode = 3;
    
    /* Bit-field to bit-field assignment */
    s2.flag = s1.flag;
    s2.value = s1.value + 1;
    
    /* Bit-field in expression */
    int temp = s1.value * 2;
    s2.value = temp & 0x3FF;  /* Mask to fit in 10 bits */
    
    /* Complex bit-field expression */
    s1.mode = (s1.flag << 2) | (s2.flag << 1) | (s1.value > 100);
    
    /* Nested bit-field structure */
    struct NestedBitField nbf = {{5, 10}, 0xABCD};
    nbf.nibbles.a = nbf.nibbles.b;
    nbf.full = (nbf.nibbles.a << 8) | nbf.nibbles.b;
    
    global_result += s1.value + s2.value + nbf.full;
}

/* ==================== PARTIAL REGISTER TESTS (STRICT_LOW_PART) ==================== */

void test_strict_low_part(void) {
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    volatile int vi = 1000;
    
    /* Assignments to sub-word types */
    vs1 = (short)vi + 50;
    vs2 = vs1 * 2;
    
    /* Arithmetic on sub-word types */
    vc1 = (char)(vi & 0xFF);
    vc2 = vc1 + 1;
    vs1 = (short)vc1 * (short)vc2;
    
    /* Mixed-size operations */
    int result = (int)vs1 + (int)vc2;
    vs2 = (short)(result & 0xFFFF);
    
    /* Loop with partial register updates */
    for (volatile char i = 0; i < 10; i++) {
        vc1 = i * 2;
        vs1 += vc1;
    }
    
    /* Conditional partial update */
    if (vi > 500) {
        vs1 = 1234;
    } else {
        vs2 = 5678;
    }
    
    global_result += vs1 + vs2 + vc1 + vc2;
}

/* ==================== SUBREG TESTS ==================== */

typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));

union TypePun {
    uint32_t full;
    struct {
        volatile uint16_t low;
        volatile uint16_t high;
    } parts;
    volatile uint8_t bytes[4];
};

void test_subreg(void) {
    /* Vector extension tests */
    v4si vec_int = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Vector element access (generates SUBREG) */
    int elem3 = vec_int[3];
    short elem5 = vec_short[5];
    
    /* Vector operations */
    vec_int[2] = elem3 * 2;
    vec_short[1] = (short)(elem5 + 100);
    
    /* Type punning through union */
    union TypePun pun;
    pun.full = 0xDEADBEEF;
    
    /* Access different-sized members */
    pun.parts.low = pun.parts.high;
    pun.bytes[0] = pun.bytes[3];
    
    /* Float/integer conversions */
    volatile float fv = 3.14159f;
    volatile double dv = 2.71828;
    
    /* These conversions often involve SUBREG */
    uint32_t float_as_int = *(uint32_t*)&fv;
    uint64_t double_as_long = *(uint64_t*)&dv;
    
    /* Mixed-type arithmetic */
    fv = (float)((int)fv * 2);
    dv = dv + (double)float_as_int;
    
    global_result += elem3 + elem5 + pun.full + (int)fv;
}

/* ==================== COMBINED PATTERN TESTS ==================== */

struct Combined {
    volatile unsigned int bf1:5;
    volatile unsigned int bf2:11;
    volatile short partial;
    uint32_t full;
};

void test_combined_patterns(void) {
    struct Combined c = {0};
    
    /* Bit-field to partial register */
    c.partial = (short)c.bf1;
    
    /* Partial register to bit-field */
    c.bf2 = (unsigned int)c.partial & 0x7FF;
    
    /* Complex expression combining both */
    volatile short temp_short = (short)(c.bf1 << 3);
    c.bf2 = (c.bf2 + (unsigned int)temp_short) & 0x7FF;
    
    /* Use in loop with conditions */
    for (volatile int i = 0; i < 5; i++) {
        c.bf1 = (c.bf1 + 1) & 0x1F;
        c.partial = (short)(c.partial + c.bf1);
        
        if (c.partial > 1000) {
            c.bf2 = c.bf2 >> 1;
        }
    }
    
    /* Pointer to sub-word type */
    volatile char *char_ptr = (volatile char*)&c.full;
    for (int i = 0; i < 4; i++) {
        char_ptr[i] = (char)(c.bf1 + i);
    }
    
    global_result += c.bf1 + c.bf2 + c.partial + c.full;
}

/* ==================== ARCHITECTURE-SPECIFIC TESTS ==================== */

#ifdef __i386__
void test_x86_specific(void) {
    volatile uint16_t w1, w2;
    volatile uint8_t b1, b2;
    
    /* x86 partial register access patterns */
    w1 = 0x1234;
    b1 = (uint8_t)(w1 & 0xFF);
    b2 = (uint8_t)((w1 >> 8) & 0xFF);
    
    /* Recombine */
    w2 = (uint16_t)b1 | ((uint16_t)b2 << 8);
    
    /* Inline assembly that might generate STRICT_LOW_PART */
    __asm__ volatile (
        "movw %1, %%ax\n\t"
        "addw $100, %%ax\n\t"
        "movw %%ax, %0\n\t"
        : "=r" (w1)
        : "r" (w2)
        : "ax"
    );
    
    global_result += w1 + w2;
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    volatile uint32_t reg;
    
    /* ARM often uses SUBREG for byte/short accesses */
    volatile uint16_t halfword;
    volatile uint8_t byte;
    
    reg = 0xA5A5A5A5;
    halfword = (uint16_t)(reg >> 16);
    byte = (uint8_t)(reg >> 8);
    
    /* Reconstruct with shifts */
    reg = (uint32_t)byte | ((uint32_t)halfword << 16);
    
    global_result += reg;
}
#endif

/* ==================== BUILTIN FUNCTION TESTS ==================== */

void test_builtins(void) {
    unsigned int x = 0x12345678;
    
    /* Builtins that may involve bit extraction */
    int leading_zeros = __builtin_clz(x);
    int trailing_zeros = __builtin_ctz(x);
    int parity = __builtin_parity(x);
    int popcount = __builtin_popcount(x);
    
    /* Extract specific bits */
    unsigned int extracted = (x >> 8) & 0xFF;  /* Like ZERO_EXTRACT */
    
    /* Rotate operations */
    unsigned int rotated = __builtin_rotateright32(x, 4);
    
    global_result += leading_zeros + trailing_zeros + parity + popcount + extracted + rotated;
}

/* ==================== MAIN TEST DRIVER ==================== */

typedef void (*test_func_t)(void);

const test_func_t test_functions[] = {
    test_bitfield_zero_extract,
    test_strict_low_part,
    test_subreg,
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
    
    /* Use command line or environment to control which tests run */
    if (argc > 1) {
        test_to_run = atoi(argv[1]) % 10;
    } else {
        /* Use volatile to prevent compile-time optimization */
        test_to_run = global_control % (sizeof(test_functions)/sizeof(test_functions[0]) - 1);
    }
    
    /* Run all tests in sequence to ensure all code paths are compiled */
    for (int i = 0; test_functions[i] != NULL; i++) {
        if (test_to_run == -1 || test_to_run == i) {
            test_functions[i]();
        }
    }
    
    /* Final computation to ensure program does something useful */
    printf("Result: %d\n", global_result);
    
    /* Return value based on result to prevent dead code elimination */
    return (global_result > 0) ? 0 : 1;
}
