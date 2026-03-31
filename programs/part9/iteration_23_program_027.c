/* Test program to trigger uncovered RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* For architecture-specific code */
#if defined(__i386__) || defined(__x86_64__)
#define HAS_X86 1
#endif

#if defined(__arm__)
#define HAS_ARM 1
#endif

/* Volatile variables to prevent optimization */
volatile int g_control = 0;
volatile int g_result = 0;

/* ==================== ZERO_EXTRACT patterns ==================== */

/* Test 1: Basic bit-field operations */
void test_bitfields(void) {
    struct S1 {
        volatile unsigned int flag:1;
        volatile unsigned int value:10;
        unsigned int pad:21;
    } s1;
    
    struct S2 {
        unsigned int data:8;
        unsigned int mode:4;
        unsigned int :4;  /* unnamed bit-field */
        unsigned int status:16;
    } s2;
    
    /* Various bit-field assignments */
    s1.flag = 1;
    s1.value = 511;  /* Max for 10 bits */
    
    /* Cross assignments */
    s2.data = s1.value & 0xFF;
    s2.mode = 3;
    s2.status = (s1.flag << 15) | (s1.value & 0x7FFF);
    
    /* Complex expression with bit-fields */
    g_result = s1.flag + s1.value + s2.data + s2.mode + s2.status;
}

/* Test 2: Nested bit-field operations */
void test_nested_bitfields(void) {
    union U1 {
        struct {
            volatile unsigned int low:16;
            volatile unsigned int high:16;
        } parts;
        volatile unsigned int whole;
    } u1;
    
    struct Container {
        union U1 data;
        volatile unsigned int mask:12;
        volatile unsigned int shift:4;
    } container;
    
    /* Operations that should generate ZERO_EXTRACT */
    u1.whole = 0x12345678;
    container.mask = (u1.parts.low >> 4) & 0xFFF;
    container.shift = u1.parts.high & 0xF;
    
    /* Bit-field in conditional */
    if (container.mask > 100) {
        u1.parts.high = container.shift << 12;
    }
    
    g_result ^= u1.whole + container.mask;
}

/* ==================== STRICT_LOW_PART patterns ==================== */

/* Test 3: Partial register updates with small types */
void test_partial_registers(void) {
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    int i = 1000;
    
    /* Assignments to partial registers */
    vs1 = (short)i + 123;
    vs2 = (short)(i * 2) - 456;
    
    /* Arithmetic on partial registers */
    vc1 = (char)(vs1 & 0xFF);
    vc2 = (char)((vs2 >> 4) & 0xFF);
    
    /* Compound assignment to partial register */
    vc1 += vc2;
    vs1 -= vs2;
    
    g_result += vs1 + vs2 + vc1 + vc2;
}

/* Test 4: Architecture-specific partial register patterns */
void test_arch_partial_registers(void) {
#ifdef HAS_X86
    /* x86-specific patterns that often generate STRICT_LOW_PART */
    volatile unsigned short us;
    volatile signed char sc;
    
    /* Operations that work on partial registers */
    us = 0xABCD;
    sc = -64;
    
    /* Mixed-size operations */
    us = (us & 0xFF00) | (sc & 0xFF);
    sc = (us >> 8) & 0x7F;
    
    g_result += us * sc;
#endif
    
#ifdef HAS_ARM
    /* ARM may generate different patterns */
    volatile short arm_short;
    volatile char arm_char;
    
    arm_short = 32767;
    arm_char = arm_short >> 8;
    
    g_result += arm_char;
#endif
}

/* ==================== SUBREG patterns ==================== */

/* Test 5: Vector operations */
void test_vector_subreg(void) {
    /* GCC vector extension */
    typedef int v4si __attribute__ ((vector_size (16)));
    typedef short v8hi __attribute__ ((vector_size (16)));
    
    v4si v1 = {1, 2, 3, 4};
    v8hi v2 = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Access vector elements - should generate SUBREG */
    int elem1 = v1[2];
    short elem2 = v2[5];
    
    /* Vector operations with scalar results */
    v1[0] = elem2;
    v2[3] = (short)elem1;
    
    /* Type punning through union */
    union VecUnion {
        v4si vi;
        int arr[4];
    } vu;
    
    vu.vi = v1;
    g_result += vu.arr[0] + vu.arr[2] + elem2;
}

/* Test 6: Type conversions and packed structures */
void test_type_conversions(void) {
    /* Packed structure with mixed types */
    struct __attribute__((packed)) Packed {
        char c;
        int i;
        short s;
    } packed;
    
    /* Type punning through union */
    union FloatInt {
        float f;
        int i;
        short s[2];
    } fi;
    
    /* Operations that may generate SUBREG */
    packed.c = 'A';
    packed.i = 0x12345678;
    packed.s = 0x9ABC;
    
    fi.f = 3.14159f;
    
    /* Access parts of larger types */
    short low_half = fi.s[0];
    short high_half = fi.s[1];
    
    /* Convert between float and int representations */
    int int_bits = fi.i;
    fi.s[0] = packed.s;
    
    g_result += packed.c + packed.i + low_half + int_bits;
}

/* ==================== Combined patterns ==================== */

/* Test 7: Complex expressions combining multiple patterns */
void test_combined_patterns(void) {
    /* Structure with bit-fields */
    struct {
        volatile unsigned int header:4;
        volatile unsigned int data:20;
        volatile unsigned int footer:8;
    } bf;
    
    /* Union for type-punning */
    union {
        volatile unsigned int full;
        volatile unsigned short halves[2];
        volatile unsigned char bytes[4];
    } converter;
    
    /* Initialize */
    bf.header = 0xF;
    bf.data = 0x12345;
    bf.footer = 0xAA;
    
    /* Complex expression: bit-field -> partial register -> vector element */
    converter.full = (bf.data << 4) | bf.footer;
    
    /* Access partial register (should be STRICT_LOW_PART or SUBREG) */
    volatile unsigned short partial = converter.halves[1];
    
    /* Bit-field extraction from partial register */
    unsigned int extracted = (partial >> 4) & 0xF;
    
    /* Update bit-field based on extracted value */
    bf.header = extracted;
    
    /* Vector operation on the result */
    typedef int v2si __attribute__ ((vector_size (8)));
    v2si vec = {bf.data, bf.header};
    int vec_result = vec[0] - vec[1];
    
    g_result = partial + extracted + vec_result;
}

/* Test 8: Builtin functions that may generate extract patterns */
void test_builtins(void) {
    unsigned int x = 0x12345678;
    
    /* Builtins that work with bits */
    int count_leading_zeros = __builtin_clz(x);
    int parity = __builtin_parity(x);
    int bit_reverse = __builtin_bitreverse32(x);
    
    /* Extract bit ranges */
    unsigned int extracted_bits = (x >> 8) & 0xFFF;  /* 12 bits */
    
    /* Combine with bit-fields */
    struct {
        unsigned int a:12;
        unsigned int b:10;
        unsigned int c:10;
    } bf;
    
    bf.a = extracted_bits;
    bf.b = count_leading_zeros & 0x3FF;
    bf.c = parity ? 1 : 0;
    
    g_result += bit_reverse + bf.a + bf.b;
}

/* ==================== Main test driver ==================== */

/* Array of test functions */
typedef void (*test_func_t)(void);

static test_func_t test_functions[] = {
    test_bitfields,
    test_nested_bitfields,
    test_partial_registers,
    test_arch_partial_registers,
    test_vector_subreg,
    test_type_conversions,
    test_combined_patterns,
    test_builtins,
    NULL
};

int main(int argc, char *argv[]) {
    int num_tests = sizeof(test_functions) / sizeof(test_functions[0]) - 1;
    int start_test = 0;
    int end_test = num_tests - 1;
    
    /* Use command line args to control which tests run */
    if (argc > 1) {
        start_test = atoi(argv[1]) % num_tests;
        if (argc > 2) {
            end_test = atoi(argv[2]) % num_tests;
        } else {
            end_test = start_test;
        }
    }
    
    if (start_test > end_test) {
        int temp = start_test;
        start_test = end_test;
        end_test = temp;
    }
    
    printf("Running tests %d to %d\n", start_test, end_test);
    
    /* Run selected tests */
    for (int i = start_test; i <= end_test && i < num_tests; i++) {
        if (test_functions[i]) {
            test_functions[i]();
        }
    }
    
    /* Ensure result is used */
    printf("Final result: %d\n", g_result);
    
    /* Simple computation to ensure program runs */
    int final_check = 0;
    for (int i = 0; i < 100; i++) {
        final_check += i * g_result;
    }
    
    return final_check != 0 ? 0 : 1;
}
