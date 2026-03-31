/* Test program to trigger uncovered RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimizations from removing our test code */
static volatile int control = 0;

/* ========== Bit-field tests for ZERO_EXTRACT ========== */

struct BitFieldStruct {
    volatile unsigned int flag:1;
    unsigned int value:10;
    unsigned int padding:21;
    volatile unsigned int large_field:20;
};

struct NestedBitFields {
    struct {
        volatile unsigned int a:3;
        unsigned int b:5;
    } inner;
    volatile unsigned int c:7;
    unsigned int d:17;
};

void test_bit_fields(void) {
    struct BitFieldStruct s1 = {0};
    struct NestedBitFields s2 = {0};
    
    /* Simple bit-field assignments */
    s1.flag = 1;
    s1.value = 511;  /* Max for 10 bits */
    s1.large_field = 0xFFFFF & ((1 << 20) - 1);
    
    /* Cross assignments */
    unsigned int temp = s1.value;
    s1.flag = temp & 1;
    
    /* Nested structure bit-fields */
    s2.inner.a = 5;
    s2.inner.b = s2.inner.a + 2;
    s2.c = s2.inner.b;
    s2.d = s2.c * 100;
    
    /* Complex expression with bit-fields */
    s1.value = (s2.inner.a << 2) | (s2.c & 0x3F);
}

/* Union with bit-fields for type-punning */
union BitFieldUnion {
    struct {
        volatile unsigned int low:8;
        volatile unsigned int high:8;
        unsigned int rest:16;
    } bits;
    volatile unsigned int full;
};

void test_bitfield_union(void) {
    union BitFieldUnion u;
    u.full = 0x12345678;
    
    /* Access through bit-field members */
    unsigned char low_byte = u.bits.low;
    u.bits.high = low_byte + 1;
    
    /* Type-punning access */
    u.full = (u.bits.high << 24) | (u.bits.low << 16);
}

/* ========== Partial register tests for STRICT_LOW_PART ========== */

void test_partial_registers(void) {
    volatile short vs;
    volatile char vc;
    volatile int vi = 1000;
    
    /* Casts to smaller types */
    vs = (short)vi + 50;
    vc = (char)(vs >> 2);
    
    /* Arithmetic on sub-word types */
    volatile short vs2 = 200;
    vs = vs + vs2 - 100;
    
    /* Complex expression with partial updates */
    vi = (vc << 8) | (unsigned char)vs;
    
    /* Multiple partial updates in sequence */
    for (int i = 0; i < 4; i++) {
        vc = (char)(vi >> (i * 8));
        vs = (short)(vc * i);
    }
}

/* Packed structure for partial access */
struct __attribute__((packed)) PackedStruct {
    char a;
    short b;
    char c;
    int d;
};

void test_packed_partial(void) {
    struct PackedStruct ps = {1, 2, 3, 4};
    volatile short* ps_ptr = (volatile short*)&ps;
    
    /* Partial access to packed structure */
    short b_val = ps.b;
    ps.b = b_val * 2;
    
    /* Unaligned access (may generate SUBREG) */
    short unaligned = *(ps_ptr + 1);  /* May span a and b */
}

/* ========== Sub-register tests for SUBREG ========== */

/* Vector extension for SUBREG patterns */
typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));

void test_vector_subreg(void) {
    v4si v = {1, 2, 3, 4};
    v8hi vh = {1, 2, 3, 4, 5, 6, 7, 8};
    
    /* Element access - likely generates SUBREG */
    volatile int element = v[2];
    v[1] = element + 10;
    
    /* Vector-scalar operations */
    v = v + element;
    
    /* Type conversion between vector types */
    v8hi vh2 = (v8hi)v;  /* May involve SUBREG */
    
    /* Partial vector operations */
    for (int i = 0; i < 4; i++) {
        v[i] = vh[i*2] + vh[i*2 + 1];
    }
}

/* Float/integer conversions for SUBREG */
void test_float_conversions(void) {
    volatile float f = 3.14159f;
    volatile double d = 2.71828;
    volatile int i = 100;
    
    /* Type conversions of different sizes */
    i = (int)f;
    f = (float)i;
    
    /* Bit-level manipulation */
    unsigned int float_bits = *(unsigned int*)&f;
    f = *(float*)&float_bits;
    
    /* Double to float (size change) */
    f = (float)d;
    d = (double)(i * 2);
}

/* ========== Combined pattern tests ========== */

struct Combined {
    volatile unsigned int bits:4;
    volatile short partial;
    volatile int full;
};

void test_combined_patterns(void) {
    struct Combined c = {0};
    
    /* Bit-field to partial register */
    c.partial = (short)c.bits * 100;
    
    /* Partial register to bit-field */
    c.bits = c.partial & 0xF;
    
    /* Complex expression combining patterns */
    c.full = (c.bits << 16) | (c.partial & 0xFFFF);
    
    /* Loop with combined accesses */
    for (volatile int i = 0; i < 4; i++) {
        c.bits = i;
        c.partial = c.partial + (short)c.bits;
        c.full = c.full | (c.partial << (i * 8));
    }
}

/* Union with multiple views of same data */
union MultiView {
    struct {
        volatile unsigned int a:5;
        volatile unsigned int b:11;
        volatile unsigned int c:16;
    } bits;
    struct {
        volatile short low;
        volatile short high;
    } parts;
    volatile unsigned int full;
};

void test_multiview_union(void) {
    union MultiView uv;
    uv.full = 0xDEADBEEF;
    
    /* Mixed access patterns */
    uv.bits.a = uv.parts.low & 0x1F;
    uv.parts.high = (uv.bits.b << 5) | uv.bits.c;
    uv.full = uv.full ^ (uv.parts.low << 16);
}

/* ========== Architecture-specific patterns ========== */

#ifdef __i386__
void test_x86_specific(void) {
    volatile int result;
    
    /* Inline assembly that might generate STRICT_LOW_PART */
    __asm__ volatile (
        "movw $0x1234, %%ax\n\t"
        "addw $0x100, %%ax\n\t"
        "movw %%ax, %0"
        : "=m" (result)
        : 
        : "ax"
    );
    
    /* Byte operations */
    volatile char bytes[4] = {1, 2, 3, 4};
    __asm__ volatile (
        "movb %1, %%al\n\t"
        "addb $10, %%al\n\t"
        "movb %%al, %0"
        : "=m" (bytes[2])
        : "m" (bytes[1])
        : "al"
    );
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    volatile int reg;
    
    /* ARM may generate SUBREG for byte operations */
    __asm__ volatile (
        "ldrb %0, [%1]\n\t"
        "add %0, %0, #1\n\t"
        "strb %0, [%1]"
        : "=r" (reg)
        : "r" (&reg)
        : "memory"
    );
}
#endif

/* ========== Builtin functions ========== */

void test_builtins(void) {
    volatile unsigned int x = 0x12345678;
    
    /* Builtins that may involve bit extraction */
    int leading_zeros = __builtin_clz(x);
    int parity = __builtin_parity(x);
    int popcount = __builtin_popcount(x);
    
    /* Use results to prevent optimization */
    x = (x << leading_zeros) | (parity << 16) | popcount;
    
    /* Bit field builtins */
    unsigned int extracted = __builtin_ibit_extract(x, 8, 4);
    x = __builtin_ibit_insert(x, 0xF, 12, 4);
}

/* ========== Main test driver ========== */

typedef void (*test_func_t)(void);

const test_func_t test_functions[] = {
    test_bit_fields,
    test_bitfield_union,
    test_partial_registers,
    test_packed_partial,
    test_vector_subreg,
    test_float_conversions,
    test_combined_patterns,
    test_multiview_union,
    test_builtins,
#ifdef __i386__
    test_x86_specific,
#endif
#ifdef __arm__
    test_arm_specific,
#endif
};

int main(int argc, char *argv[]) {
    int num_tests = sizeof(test_functions) / sizeof(test_functions[0]);
    
    /* Use command line or volatile to control execution */
    int start_test = 0;
    int end_test = num_tests;
    
    if (argc > 1) {
        start_test = atoi(argv[1]) % num_tests;
        if (argc > 2) {
            end_test = atoi(argv[2]) % (num_tests + 1);
            if (end_test <= start_test) end_test = start_test + 1;
        }
    }
    
    /* Execute selected tests */
    for (int i = start_test; i < end_test && i < num_tests; i++) {
        if (control || (i % 2)) {  /* Volatile condition */
            test_functions[i]();
        }
    }
    
    /* Final computation to ensure program does something */
    volatile int result = 0;
    for (int i = 0; i < 100; i++) {
        result += i * (control & 1);
    }
    
    printf("Result: %d (control: %d)\n", result, control);
    
    return result != 0;
}
