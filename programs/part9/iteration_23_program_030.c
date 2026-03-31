/* Test program to trigger uncovered RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int global_volatile = 0;
volatile short global_short = 0;
volatile char global_char = 0;

/* ===== 1. Bit-field patterns for ZERO_EXTRACT ===== */

struct BitFieldStruct {
    volatile unsigned int flag:1;
    volatile unsigned int value:10;
    volatile unsigned int pad:5;
    volatile unsigned int mode:3;
    unsigned int full;
};

struct NestedBitField {
    struct {
        volatile unsigned int a:4;
        volatile unsigned int b:4;
    } nibbles;
    volatile unsigned int combined:8;
};

void test_bitfields(void) {
    struct BitFieldStruct s1 = {0};
    struct BitFieldStruct s2 = {0};
    
    /* Simple bit-field assignments */
    s1.flag = 1;
    s1.value = 511;  /* Max for 10 bits */
    s1.mode = 3;
    
    /* Bit-field to bit-field assignment */
    s2.value = s1.value;
    s2.flag = s1.flag;
    
    /* Bit-field in expression */
    int x = s1.value + s2.flag;
    global_volatile = x;
    
    /* Complex bit-field expression */
    s1.pad = (s1.value >> 3) & 0x1F;
    
    /* Nested bit-field structure */
    struct NestedBitField nbf;
    nbf.nibbles.a = 7;
    nbf.nibbles.b = 8;
    nbf.combined = nbf.nibbles.a | (nbf.nibbles.b << 4);
    
    /* Bit-field in loop */
    for (int i = 0; i < 4; i++) {
        s1.mode = i & 0x3;
        global_volatile += s1.mode;
    }
}

/* ===== 2. Partial register patterns for STRICT_LOW_PART ===== */

void test_partial_registers(void) {
    volatile short vs = 0;
    volatile char vc = 0;
    volatile int vi = 0;
    
    /* Casts to smaller types */
    int large = 0x12345678;
    vs = (short)large;
    vc = (char)large;
    
    /* Arithmetic on sub-word types */
    short s1 = 1000;
    short s2 = 2000;
    vs = s1 + s2;
    
    char c1 = 50;
    char c2 = 60;
    vc = c1 * c2;
    
    /* Mixed-size operations */
    vi = vs + vc;
    
    /* Partial register update in expression */
    vs = (short)(vs * 2 + 1);
    
    /* Loop with partial register updates */
    for (char i = 0; i < 10; i++) {
        vc = i * 5;
        global_char += vc;
    }
    
    /* Conditional partial updates */
    if (global_volatile > 0) {
        vs = 32767;
    } else {
        vs = -32768;
    }
}

/* ===== 3. Sub-register patterns for SUBREG ===== */

/* Packed structure for sub-register access */
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

void test_subregisters(void) {
    /* Packed structure access */
    struct PackedStruct ps;
    ps.a = 1;
    ps.b = 0x12345678;
    ps.c = 0x9ABC;
    
    /* Accessing misaligned members */
    int b_val = ps.b;  /* May involve SUBREG due to packing */
    global_volatile = b_val;
    
    /* Union type-punning */
    union TypePun tp;
    tp.i = 0x40490FDB;  /* Approx pi */
    float f_val = tp.f;
    short s1 = tp.halves.s1;
    short s2 = tp.halves.s2;
    
    /* Vector operations */
    v4si vec_int = {1, 2, 3, 4};
    v8hi vec_short = {1, 2, 3, 4, 5, 6, 7, 8};
    
    /* Vector element access */
    int elem = vec_int[2];
    vec_int[3] = elem * 2;
    
    /* Vector to scalar */
    for (int i = 0; i < 4; i++) {
        global_volatile += vec_int[i];
    }
    
    /* Mixed vector/scalar operations */
    vec_short[0] = (short)global_short;
    
    /* Float/int conversions */
    float f = 3.14159f;
    int *int_ptr = (int*)&f;
    int int_val = *int_ptr;
    global_volatile = int_val;
}

/* ===== 4. Combined patterns ===== */

struct Combined {
    volatile unsigned int low:8;
    volatile unsigned int high:8;
    volatile short middle;
};

void test_combined_patterns(void) {
    struct Combined c = {0};
    
    /* Bit-field to partial register */
    c.low = 0xAB;
    c.high = 0xCD;
    global_short = (short)((c.high << 8) | c.low);
    
    /* Partial register to bit-field */
    short temp = global_short;
    c.low = temp & 0xFF;
    c.high = (temp >> 8) & 0xFF;
    
    /* Complex expression with multiple patterns */
    c.middle = (short)(c.low + c.high * 2);
    
    /* In conditional */
    if (c.middle > 100) {
        c.low = c.middle & 0xFF;
        global_char = (char)c.low;
    }
    
    /* Loop with combined patterns */
    for (int i = 0; i < 8; i++) {
        c.low = i;
        c.high = 7 - i;
        c.middle = (short)(c.low - c.high);
        global_volatile += c.middle;
    }
}

/* ===== 5. Architecture-specific patterns ===== */

#ifdef __i386__
void test_x86_specific(void) {
    /* x86 inline assembly with partial register output */
    int result;
    short s_result;
    char c_result;
    
    /* Partial register output */
    asm volatile (
        "movw $0x1234, %%ax\n\t"
        "movb $0x56, %%ah\n\t"
        : "=a"(s_result)
        :
        : "ax"
    );
    
    /* Byte register operation */
    asm volatile (
        "movb $0x78, %%al\n\t"
        "addb $0x22, %%al\n\t"
        : "=a"(c_result)
        :
        : "al"
    );
    
    global_short = s_result;
    global_char = c_result;
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    /* ARM specific patterns */
    volatile int arm_var = 0;
    
    /* ARM may generate SUBREG for byte operations */
    char *ptr = (char*)&arm_var;
    ptr[0] = 1;
    ptr[1] = 2;
    ptr[2] = 3;
    ptr[3] = 4;
    
    global_volatile = arm_var;
}
#endif

/* ===== Builtin functions ===== */

void test_builtins(void) {
    unsigned int x = 0x12345678;
    
    /* Bit manipulation builtins */
    int leading_zeros = __builtin_clz(x);
    int trailing_zeros = __builtin_ctz(x);
    int parity = __builtin_parity(x);
    int popcount = __builtin_popcount(x);
    
    /* Extract bits */
    unsigned int extracted = (x >> 8) & 0xFF;  /* Like ZERO_EXTRACT */
    
    global_volatile = leading_zeros + trailing_zeros + parity + popcount + extracted;
}

/* ===== Main test driver ===== */

typedef void (*test_func_t)(void);

const test_func_t test_functions[] = {
    test_bitfields,
    test_partial_registers,
    test_subregisters,
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
    
    /* Use command line or volatile to control test selection */
    if (argc > 1) {
        test_to_run = atoi(argv[1]) % 10;
    } else {
        test_to_run = global_volatile % 10;
    }
    
    /* Run all tests in sequence */
    for (int i = 0; test_functions[i] != NULL; i++) {
        if (test_to_run == 0 || test_to_run == i + 1) {
            test_functions[i]();
        }
    }
    
    /* Final computation to ensure program does something */
    int result = global_volatile + global_short + global_char;
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
