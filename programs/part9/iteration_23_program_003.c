/* Test program to trigger specific RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int global_trigger = 0;
volatile int global_result = 0;

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
    } nibbles;
    volatile unsigned int full:16;
};

void test_bitfield_operations(void) {
    struct BitFieldStruct s1 = {0};
    struct BitFieldStruct s2 = {1, 511, 7, 0};
    
    /* Basic bit-field assignments - should generate ZERO_EXTRACT */
    s1.flag = s2.flag;
    s1.value = s2.value & 0x3FF;
    s1.mode = (s2.mode + 1) & 0x7;
    
    /* Cross-structure bit-field operations */
    unsigned int temp = s1.value;
    s2.value = temp | 0x100;
    
    /* Complex expression with bit-fields */
    global_result += (s1.flag << 16) | (s1.value << 6) | s1.mode;
}

void test_nested_bitfields(void) {
    struct NestedBitField nbf = {{0x5, 0xA}, 0xABCD};
    
    /* Access nested bit-fields */
    nbf.nibbles.low = (nbf.nibbles.high >> 1) & 0x7;
    nbf.nibbles.high = (nbf.full >> 12) & 0xF;
    
    /* Combine bit-fields in expressions */
    nbf.full = (nbf.nibbles.high << 12) | 
               (nbf.nibbles.low << 8) | 
               (nbf.full & 0xFF);
    
    global_result ^= nbf.full;
}

/* ==================== PARTIAL REGISTER PATTERNS (STRICT_LOW_PART) ==================== */

void test_partial_register_ops(void) {
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    volatile int vi = 0x12345678;
    
    /* Casts to smaller types - may generate STRICT_LOW_PART */
    vs1 = (short)vi + 0x100;
    vs2 = (short)(vi >> 16) - 0x50;
    
    /* Arithmetic on sub-word types */
    vc1 = (char)vs1 + 10;
    vc2 = (char)(vs2 * 2) - 5;
    
    /* Store back with modification */
    vs1 = vs1 + (short)vc1;
    vs2 = vs2 - (short)vc2;
    
    /* Mixed-size operations */
    global_result += (int)vs1 + (int)vs2 + (int)vc1 + (int)vc2;
}

/* Architecture-specific partial register patterns */
#ifdef __i386__
void test_x86_partial_registers(void) {
    volatile short sval;
    volatile char cval;
    int ival;
    
    /* These patterns often generate STRICT_LOW_PART on x86 */
    ival = 0xDEADBEEF;
    sval = (short)ival;
    cval = (char)(ival >> 24);
    
    /* Modify partial values */
    sval = sval ^ 0x55AA;
    cval = cval & 0x7F;
    
    global_result += (int)sval + (int)cval;
}
#endif

#ifdef __arm__
void test_arm_partial_registers(void) {
    volatile short hs1, hs2;
    volatile char hc;
    
    /* ARM often uses STRICT_LOW_PART for byte/short operations */
    hs1 = 0x1234;
    hs2 = 0x5678;
    hc = (char)(hs1 + hs2);
    
    hs1 = hs1 >> 4;
    hs2 = hs2 << 2;
    
    global_result += (int)hs1 * (int)hs2 + (int)hc;
}
#endif

/* ==================== SUBREG PATTERNS ==================== */

/* Vector extension for SUBREG patterns */
typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));
typedef char v16qi __attribute__ ((vector_size (16)));

void test_vector_subregs(void) {
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v8hi vh = {10, 20, 30, 40, 50, 60, 70, 80};
    v16qi vc;
    
    /* Vector operations that generate SUBREG */
    v4si v3 = v1 + v2;
    v4si v4 = v1 * v2;
    
    /* Access individual elements - often uses SUBREG */
    int elem1 = v3[0];
    int elem2 = v3[2];
    
    /* Cross-type vector access */
    for (int i = 0; i < 8; i++) {
        vh[i] = (short)(vh[i] + i);
    }
    
    /* Initialize char vector */
    for (int i = 0; i < 16; i++) {
        vc[i] = (char)(i * 3);
    }
    
    global_result += elem1 + elem2 + (int)vh[3] + (int)vc[10];
}

/* Union for type-punning SUBREG patterns */
union TypePun {
    volatile float f;
    volatile int i;
    volatile short s[2];
    volatile char c[4];
};

void test_type_punning(void) {
    union TypePun up;
    
    up.f = 3.14159f;
    
    /* Access different views of the same memory - may generate SUBREG */
    int int_view = up.i;
    short short_view = up.s[1];
    char char_view = up.c[3];
    
    /* Modify through different types */
    up.s[0] = (short)(up.i >> 16);
    up.c[2] = (char)(up.s[1] & 0xFF);
    
    up.f = up.f * 2.0f;
    
    global_result += int_view + short_view + char_view;
}

/* ==================== COMBINED PATTERNS ==================== */

struct Combined {
    volatile unsigned int bits:12;
    volatile short half;
    volatile char quarter;
    int full;
};

void test_combined_patterns(void) {
    struct Combined c = {0xFFF, 0x1234, 0x56, 0x789ABCDE};
    
    /* Combine bit-field with partial register */
    c.half = (short)c.bits;
    c.quarter = (char)(c.bits >> 4);
    
    /* Bit-field from partial register */
    c.bits = (c.full & 0xFFF) | ((unsigned int)c.half & 0xF00);
    
    /* Complex expression mixing all types */
    int temp = (c.full >> 16) & 0xFFFF;
    c.half = (short)temp + (short)c.bits;
    c.quarter = (char)(c.half & 0x7F);
    
    /* Loop to ensure RTL generation */
    for (int i = 0; i < 3; i++) {
        c.bits = (c.bits << 1) | (c.bits >> 11);
        c.half = c.half ^ (short)c.bits;
    }
    
    global_result += c.full + (int)c.half + (int)c.quarter + (int)c.bits;
}

/* ==================== COMPLEX EXPRESSIONS WITH BUILTINS ==================== */

void test_builtin_operations(void) {
    volatile unsigned int x = 0x12345678;
    volatile unsigned int y = 0x9ABCDEF0;
    
    /* Builtins that may involve bit manipulation */
    int popcount = __builtin_popcount(x);
    int parity = __builtin_parity(y);
    int clz = __builtin_clz(x);
    int ctz = __builtin_ctz(y);
    
    /* Combine with bit-fields */
    struct {
        volatile unsigned int a:5;
        volatile unsigned int b:5;
        volatile unsigned int c:5;
    } bf = {popcount & 0x1F, parity & 0x1F, clz & 0x1F};
    
    bf.a = (bf.a + bf.b) & 0x1F;
    bf.c = (bf.c ^ bf.a) & 0x1F;
    
    /* Use in expression with partial register */
    volatile short vs = (short)((bf.a << 10) | (bf.b << 5) | bf.c);
    
    global_result += popcount + parity + clz + ctz + (int)vs;
}

/* ==================== MAIN TEST DRIVER ==================== */

typedef void (*test_func_t)(void);

const test_func_t test_functions[] = {
    test_bitfield_operations,
    test_nested_bitfields,
    test_partial_register_ops,
#ifdef __i386__
    test_x86_partial_registers,
#endif
#ifdef __arm__
    test_arm_partial_registers,
#endif
    test_vector_subregs,
    test_type_punning,
    test_combined_patterns,
    test_builtin_operations,
    NULL
};

int main(int argc, char *argv[]) {
    int test_to_run = 0;
    
    /* Use command line or volatile to control test selection */
    if (argc > 1) {
        test_to_run = atoi(argv[1]) % 10;
    } else {
        test_to_run = global_trigger % 10;
    }
    
    printf("Running test patterns for GCC RTL coverage...\n");
    
    /* Run all tests or specific one based on input */
    if (test_to_run == 0) {
        /* Run all tests */
        for (int i = 0; test_functions[i] != NULL; i++) {
            test_functions[i]();
        }
    } else {
        /* Run specific test */
        int idx = (test_to_run - 1) % 8;
        if (test_functions[idx] != NULL) {
            test_functions[idx]();
        }
    }
    
    /* Ensure the result is used to prevent dead code elimination */
    printf("Result: 0x%08X\n", global_result);
    
    /* Simple computation to ensure program is valid */
    int final_check = global_result;
    for (int i = 0; i < 100; i++) {
        final_check = (final_check * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    return final_check % 256;
}
