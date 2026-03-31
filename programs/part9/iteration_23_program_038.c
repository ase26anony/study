/* Test program to trigger specific RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Use volatile to prevent optimization */
volatile int control = 0;

/* ====== Bit-field patterns for ZERO_EXTRACT ====== */

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
    volatile unsigned int full:8;
};

void test_bitfield_operations(void) {
    struct BitFieldStruct s1 = {0};
    struct BitFieldStruct s2 = {1, 512, 7, 0};
    
    /* Basic bit-field assignments */
    s1.flag = 1;
    s1.value = 42;
    s1.mode = 3;
    
    /* Cross assignments between bit-fields */
    s2.flag = s1.flag;
    s2.value = s1.value + 100;
    
    /* Complex expression with bit-fields */
    volatile int x = (s1.flag << 16) | (s1.value << 6) | s1.mode;
    
    /* Nested bit-field structure */
    struct NestedBitField nbf = {{1, 15}, 255};
    nbf.nibbles.low = nbf.nibbles.high & 0x7;
    nbf.full = (nbf.nibbles.low << 4) | nbf.nibbles.high;
    
    /* Prevent dead code elimination */
    control = s1.value + s2.value + x + nbf.full;
}

/* ====== Partial register patterns for STRICT_LOW_PART ====== */

void test_partial_register_ops(void) {
    volatile char vc;
    volatile short vs;
    volatile int vi = 1000;
    
    /* Partial register assignments with smaller types */
    vc = (char)vi + 50;           /* Should generate partial reg update */
    vs = (short)(vi * 2) - 100;   /* Another partial reg update */
    
    /* Mixed-type operations */
    volatile long vl = 1000000L;
    vc = (char)(vl >> 16);        /* Extract byte from long */
    
    /* Array with partial updates */
    volatile short arr[4] = {0};
    for (int i = 0; i < 4; i++) {
        arr[i] = (short)(vi + i * 100);
    }
    
    /* Complex expression forcing partial reg */
    volatile int temp = (vc << 8) | (vs & 0xFF);
    vs = (short)(temp ^ 0x55AA);
    
    /* Prevent optimization */
    control = vc + vs + arr[0];
}

/* ====== Sub-register patterns for SUBREG ====== */

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

/* GCC vector extension for SUBREG */
typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));

void test_subreg_operations(void) {
    /* Packed structure access */
    struct PackedStruct ps = {'A', 123456, 789};
    volatile int b_val = ps.b;  /* May generate SUBREG due to misalignment */
    
    /* Union type-punning */
    union TypePun up;
    up.i = 0x3F800000;  /* IEEE 754 representation of 1.0f */
    volatile float f_val = up.f;
    volatile short s_val = up.halves.s1;
    
    /* Vector operations */
    v4si vec_int = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Vector element access - likely generates SUBREG */
    volatile int elem = vec_int[2];
    volatile short selem = vec_short[5];
    
    /* Vector-scalar operations */
    vec_int[0] = elem * 2;
    vec_short[3] = (short)(selem + 100);
    
    /* Float/int conversions that may use SUBREG */
    volatile float f = 3.14159f;
    volatile int fi = *(int*)&f;  /* Type punning */
    volatile float f2 = *(float*)&fi;
    
    /* Prevent optimization */
    control = b_val + fi + elem + selem;
}

/* ====== Combined/complex patterns ====== */

struct ComplexBitField {
    volatile unsigned int a:3;
    volatile unsigned int b:5;
    volatile unsigned int c:8;
    volatile unsigned int d:16;
};

void test_combined_patterns(void) {
    struct ComplexBitField cbf = {1, 31, 128, 65535};
    
    /* Combine bit-field with partial register */
    volatile short partial = (short)cbf.c;  /* ZERO_EXTRACT + SUBREG/STRICT_LOW_PART */
    
    /* Nested operations */
    cbf.a = (cbf.b >> 2) & 0x7;
    cbf.d = (cbf.c << 8) | partial;
    
    /* Loop with combined operations */
    for (int i = 0; i < 4; i++) {
        volatile char temp = (char)((cbf.d >> (i * 4)) & 0xF);
        cbf.a = (cbf.a + temp) & 0x7;
    }
    
    /* Union with bit-field */
    union {
        struct ComplexBitField bf;
        unsigned int full;
    } u;
    u.bf = cbf;
    u.full = u.full ^ 0xAAAAAAAA;
    
    /* Architecture-specific builtins */
#ifdef __GNUC__
    /* Bit manipulation builtins that may generate extract patterns */
    volatile int popcnt = __builtin_popcount(u.full);
    volatile int parity = __builtin_parity(cbf.b);
    volatile int clz = __builtin_clz(cbf.d | 1);
#endif
    
    /* Prevent optimization */
    control = partial + u.full;
}

/* ====== Architecture-specific patterns ====== */

#ifdef __i386__
void test_x86_specific(void) {
    volatile int result;
    
    /* Inline assembly that might generate partial register updates */
    __asm__ volatile (
        "movw $0x1234, %%ax\n\t"
        "movb $0x56, %%ah\n\t"
        "movw %%ax, %0"
        : "=r" (result)
        :
        : "ax"
    );
    
    control += result;
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    volatile int result;
    
    /* ARM-specific operations that might generate SUBREG */
    __asm__ volatile (
        "uxth %0, %1\n\t"  /* Zero-extend halfword */
        : "=r" (result)
        : "r" (0x12345678)
    );
    
    control += result;
}
#endif

/* ====== Main test driver ====== */

typedef void (*test_func_t)(void);

const test_func_t test_functions[] = {
    test_bitfield_operations,
    test_partial_register_ops,
    test_subreg_operations,
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
    
    /* Use argc to control which tests run (prevents dead code elimination) */
    int start_test = (argc > 1) ? atoi(argv[1]) % 5 : 0;
    
    /* Run tests in a loop to ensure code generation */
    for (int i = 0; test_functions[i] != NULL; i++) {
        if (i >= start_test) {
            test_functions[i]();
            num_tests++;
        }
    }
    
    /* Final computation using all accumulated results */
    volatile int final_result = control + num_tests;
    
    /* Print something to ensure execution */
    printf("Tests executed: %d, Control value: %d\n", num_tests, final_result);
    
    return final_result != 0 ? 0 : 1;
}
