/* Test program to trigger uncovered RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimizations from removing our test code */
static volatile int control = 0;

/* ==================== Bit-field tests for ZERO_EXTRACT ==================== */

struct BitFieldStruct {
    volatile unsigned int flag:1;
    unsigned int value:10;
    unsigned int padding:21;
    volatile unsigned int large:20;
};

struct NestedBitField {
    struct {
        volatile unsigned int a:3;
        unsigned int b:5;
    } inner;
    volatile unsigned int c:12;
};

void test_bitfields(void) {
    struct BitFieldStruct s1 = {0};
    struct NestedBitField s2 = {0};
    
    /* Basic bit-field assignments - should generate ZERO_EXTRACT */
    s1.flag = 1;
    s1.value = 511;  /* Max for 10 bits */
    s1.large = 0xFFFFF;
    
    /* Bit-field reads */
    unsigned int read_flag = s1.flag;
    unsigned int read_value = s1.value;
    
    /* Cross assignments between bit-fields */
    s2.inner.a = s1.flag;
    s2.c = s1.value;
    
    /* Complex expression with bit-fields */
    s1.value = (s2.inner.a << 2) | (s2.c & 0x3F);
    
    /* Prevent dead code elimination */
    control = read_flag + read_value;
}

/* ==================== Partial register tests for STRICT_LOW_PART ==================== */

void test_partial_registers(void) {
    volatile short vs = 0;
    volatile char vc = 0;
    volatile int vi = 0;
    
    /* Partial register assignments */
    vs = (short)(control + 100);
    vc = (char)(vs + 50);
    
    /* Arithmetic on sub-word types */
    short s1 = 1000;
    short s2 = 2000;
    vs = s1 + s2;  /* May generate STRICT_LOW_PART for 16-bit store to 32-bit register */
    
    /* Mixed-size operations */
    vi = vs;  /* Sign/zero extension */
    vs = vi & 0xFFFF;  /* Truncation */
    
    /* Pointer casting to partial types */
    int array[4] = {1, 2, 3, 4};
    short *sp = (short *)array;
    vs = sp[1];  /* Load partial word */
    
    /* Prevent optimization */
    control = vs + vc + vi;
}

/* ==================== Sub-register tests for SUBREG ==================== */

/* Vector type for SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

/* Packed structure for SUBREG accesses */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

union TypePun {
    int i;
    float f;
    short s[2];
};

void test_subreg(void) {
    /* Vector operations - should generate SUBREG */
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;
    
    /* Vector element access */
    int element = v3[2];  /* SUBREG for element extraction */
    
    /* Vector type conversion */
    v8hi vh = {1, 2, 3, 4, 5, 6, 7, 8};
    short sh = vh[3];
    
    /* Packed structure access */
    struct PackedStruct ps;
    ps.a = 10;
    ps.b = 0x12345678;  /* Misaligned access may use SUBREG */
    ps.c = 1000;
    
    int b_val = ps.b;  /* Unaligned load */
    
    /* Type punning through union */
    union TypePun pun;
    pun.i = 0x40000000;
    float f = pun.f;  /* Bit reinterpretation */
    short s0 = pun.s[0];  /* Partial access */
    
    /* Float/integer conversions */
    float f1 = 3.14f;
    int fi = *(int*)&f1;  /* Type punning */
    float f2 = *(float*)&fi;
    
    /* Prevent optimization */
    control = element + b_val + fi + (int)f2;
}

/* ==================== Combined pattern tests ==================== */

struct Combined {
    volatile unsigned int bf1:4;
    volatile unsigned int bf2:8;
    volatile short partial;
    int full;
};

void test_combined_patterns(void) {
    struct Combined c = {0};
    
    /* Bit-field to partial register */
    c.bf1 = 7;
    c.partial = (short)c.bf1;  /* ZERO_EXTRACT -> SUBREG/STRICT_LOW_PART */
    
    /* Partial register to bit-field */
    c.partial = 255;
    c.bf2 = c.partial & 0xFF;  /* Multiple extractions */
    
    /* Complex expression combining patterns */
    c.full = (c.bf1 << 16) | (c.bf2 << 8) | c.partial;
    
    /* Nested operations */
    volatile short temp = c.partial;
    c.bf1 = temp & 0xF;
    c.bf2 = (temp >> 4) & 0xFF;
    
    /* Prevent optimization */
    control = c.full;
}

/* ==================== Architecture-specific tests ==================== */

#ifdef __i386__
void test_x86_specific(void) {
    volatile int result;
    
    /* Inline assembly that might generate partial register ops */
    __asm__ volatile (
        "movw %1, %%ax\n\t"
        "addw $100, %%ax\n\t"
        "movw %%ax, %0"
        : "=m" (result)
        : "i" (500)
        : "ax"
    );
    
    /* Bit test and set */
    unsigned int flags = 0;
    __asm__ volatile (
        "btsl $5, %0"
        : "+r" (flags)
        :
        : "cc"
    );
    
    control = result + flags;
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    volatile int result;
    
    /* ARM may generate interesting partial register patterns */
    __asm__ volatile (
        "uxth %0, %1"
        : "=r" (result)
        : "r" (0x1234ABCD)
    );
    
    control = result;
}
#endif

/* ==================== Builtin function tests ==================== */

void test_builtins(void) {
    unsigned int x = 0x12345678;
    
    /* Builtins that may involve bit extraction */
    int leading_zeros = __builtin_clz(x);
    int parity = __builtin_parity(x);
    int popcount = __builtin_popcount(x);
    
    /* Bit extraction builtins */
    unsigned int extracted = __builtin_extract_bits(x, 0x0F00);
    
    /* Prevent optimization */
    control = leading_zeros + parity + popcount + extracted;
}

/* ==================== Main test driver ==================== */

typedef void (*test_func_t)(void);

static test_func_t test_functions[] = {
    test_bitfields,
    test_partial_registers,
    test_subreg,
    test_combined_patterns,
    #ifdef __i386__
    test_x86_specific,
    #endif
    #ifdef __arm__
    test_arm_specific,
    #endif
    test_builtins,
    NULL
};

int main(int argc, char *argv[]) {
    int i;
    
    /* Use command line to control which tests run */
    int start_test = 0;
    int end_test = sizeof(test_functions)/sizeof(test_functions[0]) - 1;
    
    if (argc > 1) {
        start_test = atoi(argv[1]) % (end_test + 1);
    }
    if (argc > 2) {
        end_test = atoi(argv[2]) % (end_test + 1);
    }
    if (start_test > end_test) {
        int temp = start_test;
        start_test = end_test;
        end_test = temp;
    }
    
    /* Run selected tests */
    for (i = start_test; i <= end_test && test_functions[i] != NULL; i++) {
        test_functions[i]();
    }
    
    /* Ensure the program does something visible */
    printf("Control value: %d\n", control);
    
    /* Simple computation to ensure program runs */
    int sum = 0;
    for (i = 0; i < 100; i++) {
        sum += i;
    }
    
    return sum == 4950 ? 0 : 1;
}
