/* Comprehensive test for GCC RTL resource tracking coverage */
/* Compile with: gcc -O1 -fdump-rtl-all -fno-strict-aliasing -o coverage_test coverage_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int global_trigger = 0;
volatile int global_result = 0;

/* ==================== BIT-FIELD TESTS (ZERO_EXTRACT) ==================== */

struct BitFieldStruct {
    volatile unsigned int flag:1;
    volatile unsigned int value:10;
    volatile unsigned int mode:3;
    unsigned int padding:18;
};

struct NestedBitFields {
    struct {
        volatile unsigned int a:4;
        volatile unsigned int b:4;
    } nibbles;
    volatile unsigned int full:8;
};

void test_bitfield_operations(void) {
    struct BitFieldStruct s1 = {0, 0, 0, 0};
    struct BitFieldStruct s2 = {0, 0, 0, 0};
    
    /* Basic bit-field assignments */
    s1.flag = 1;
    s1.value = 511;  /* Max 10-bit value */
    s1.mode = 7;
    
    /* Cross-structure bit-field operations */
    s2.value = s1.value >> 2;
    s2.flag = s1.flag | (s1.mode & 1);
    
    /* Complex expression with bit-fields */
    global_result += s1.value * s2.flag;
    
    /* Nested bit-field structure */
    struct NestedBitFields nbf;
    nbf.nibbles.a = 5;
    nbf.nibbles.b = 10;
    nbf.full = (nbf.nibbles.a << 4) | nbf.nibbles.b;
    
    /* Bit-field in loop */
    for (int i = 0; i < 3; i++) {
        s1.value = (s1.value + i) & 0x3FF;  /* Keep within 10 bits */
        global_result += s1.value;
    }
}

/* ==================== PARTIAL REGISTER TESTS (STRICT_LOW_PART) ==================== */

void test_partial_register_ops(void) {
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    volatile int vi = 1000;
    
    /* Explicit casts to smaller types */
    vs1 = (short)(vi + 500);
    vc1 = (char)(vi % 256);
    
    /* Arithmetic on sub-word types */
    vs2 = vs1 + (short)50;
    vc2 = vc1 * 2;
    
    /* Store back to partial register */
    vs1 = vs2 - (short)10;
    vc1 = vc2 + (char)5;
    
    /* Mixed-size operations */
    global_result += vs1 + vc1;
    
    /* Loop with partial register updates */
    for (volatile char i = 0; i < 5; i++) {
        vc1 = vc1 + i;
        global_result += vc1;
    }
}

/* ==================== SUBREG TESTS ==================== */

/* Vector type for SUBREG operations */
typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));

/* Union for type-punning */
union TypePun {
    volatile int i;
    volatile float f;
    volatile short s[2];
    volatile char c[4];
};

void test_subreg_operations(void) {
    /* Vector operations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3;
    
    /* Vector operations that generate SUBREG */
    vec3 = vec1 + vec2;
    
    /* Access vector elements (triggers SUBREG) */
    for (int i = 0; i < 4; i++) {
        global_result += vec3[i];
    }
    
    /* Mixed vector types */
    v8hi short_vec = {1, 2, 3, 4, 5, 6, 7, 8};
    volatile short sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += short_vec[i];
    }
    global_result += sum;
    
    /* Type-punning union */
    union TypePun pun;
    pun.i = 0x12345678;
    
    /* Access different views of same memory */
    global_result += pun.s[0];  /* Accesses lower 16 bits */
    global_result += pun.c[2];  /* Accesses third byte */
    
    /* Float/int conversions */
    pun.f = 3.14159f;
    global_result += pun.i & 0xFF;  /* Access bits of float */
}

/* ==================== COMBINED PATTERN TESTS ==================== */

struct Combined {
    volatile unsigned int low:8;
    volatile unsigned int high:8;
    volatile short middle;
    volatile int full;
};

void test_combined_patterns(void) {
    struct Combined c = {0, 0, 0, 0};
    
    /* Combined bit-field and partial register */
    c.low = 0xAB;
    c.high = 0xCD;
    c.middle = (short)((c.high << 8) | c.low);
    
    /* Complex expression mixing patterns */
    c.full = (c.middle * 2) + ((c.low & 0xF) << 4);
    
    /* Nested operations */
    volatile short temp;
    temp = (short)c.full;
    c.low = temp & 0xFF;
    c.high = (temp >> 8) & 0xFF;
    
    global_result += c.full + c.middle;
}

/* ==================== ARCHITECTURE-SPECIFIC TESTS ==================== */

#ifdef __i386__
void test_x86_specific(void) {
    volatile int a = 0x12345678;
    volatile short b;
    
    /* Inline assembly that might generate partial register ops */
    asm volatile (
        "movw %1, %0\n\t"
        : "=r" (b)
        : "m" (a)
        : "memory"
    );
    
    global_result += b;
    
    /* Bit test and set */
    asm volatile (
        "btsl $5, %0\n\t"
        : "+m" (a)
        :
        : "cc"
    );
    
    global_result += a;
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    volatile int a = 100;
    volatile short b;
    
    /* ARM-specific operations */
    asm volatile (
        "sxth %0, %1\n\t"
        : "=r" (b)
        : "r" (a)
    );
    
    global_result += b;
}
#endif

/* ==================== BUILTIN FUNCTION TESTS ==================== */

void test_builtin_operations(void) {
    volatile unsigned int x = 0x12345678;
    volatile int count;
    
    /* Builtins that may involve bit extraction */
    count = __builtin_popcount(x);
    global_result += count;
    
    count = __builtin_ctz(x);
    global_result += count;
    
    /* Parity check */
    count = __builtin_parity(x);
    global_result += count;
    
    /* Bit reversal (if available) */
    #ifdef __builtin_bitreverse32
    x = __builtin_bitreverse32(x);
    global_result += x & 0xFF;
    #endif
}

/* ==================== MAIN TEST DRIVER ==================== */

typedef void (*test_func_t)(void);

const test_func_t test_functions[] = {
    test_bitfield_operations,
    test_partial_register_ops,
    test_subreg_operations,
    test_combined_patterns,
    test_builtin_operations,
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
        /* Use volatile to prevent compile-time elimination */
        test_to_run = global_trigger % (sizeof(test_functions)/sizeof(test_functions[0]) - 1);
    }
    
    /* Run selected test */
    if (test_functions[test_to_run]) {
        test_functions[test_to_run]();
    } else {
        /* Run all tests */
        for (int i = 0; test_functions[i] != NULL; i++) {
            test_functions[i]();
        }
    }
    
    /* Ensure the result is used to prevent dead code elimination */
    printf("Result: %d\n", global_result);
    
    /* Additional complex expression to ensure RTL generation */
    struct BitFieldStruct final_s = {0, 0, 0, 0};
    volatile short final_vs = 0;
    union TypePun final_pun;
    
    final_s.value = global_result & 0x3FF;
    final_vs = (short)global_result;
    final_pun.i = global_result;
    
    /* Combined access pattern */
    int final_result = final_s.value + final_vs + final_pun.s[0];
    
    return final_result & 0xFF;
}
