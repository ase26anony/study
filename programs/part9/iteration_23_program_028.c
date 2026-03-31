/* Test program to trigger specific RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimizations from removing our test code */
static volatile int control = 0;

/* ========== Test 1: Bit-field operations for ZERO_EXTRACT ========== */
struct BitFieldStruct {
    volatile unsigned int flag:1;
    unsigned int value:10;
    unsigned int padding:21;
    volatile unsigned int another:4;
};

struct NestedBitField {
    struct {
        volatile unsigned int low:8;
        volatile unsigned int high:8;
    } parts;
    unsigned int full;
};

void test_bitfields(void) {
    struct BitFieldStruct s1 = {0};
    struct BitFieldStruct s2 = {0};
    struct NestedBitField n = {0};
    
    /* Basic bit-field assignments */
    s1.flag = 1;
    s1.value = 511;  /* Max 10-bit value */
    s1.another = 7;
    
    /* Cross assignments between bit-fields */
    s2.value = s1.value;
    s2.flag = s1.flag ^ 1;
    
    /* Complex expression with bit-fields */
    unsigned int temp = (s1.value << 3) | s1.another;
    s2.another = temp & 0xF;
    
    /* Nested bit-field access */
    n.parts.low = 0xAB;
    n.parts.high = 0xCD;
    n.full = (n.parts.high << 8) | n.parts.low;
    
    /* Bit-field in loop */
    for (int i = 0; i < 4; i++) {
        s1.another = (s1.another + 1) & 0xF;
    }
    
    /* Prevent dead code elimination */
    control = s1.flag | s2.value | n.full;
}

/* ========== Test 2: Partial register operations for STRICT_LOW_PART ========== */
void test_partial_registers(void) {
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    int i = 100;
    long l = 1000L;
    
    /* Casts to smaller types */
    vs1 = (short)i + 5;
    vc1 = (char)(i * 2);
    
    /* Arithmetic on sub-word types */
    vs2 = vs1 * 2;
    vc2 = vc1 + 10;
    
    /* Mixed-size operations */
    short result = (short)((int)vs1 + (int)vs2);
    vs1 = result;
    
    /* Partial updates in expressions */
    i = (int)vs1 + (vc2 << 8);
    
    /* Loop with partial register updates */
    for (char c = 0; c < 10; c++) {
        vc1 = c * 2;
        vs1 = (short)(vs1 + vc1);
    }
    
    /* Prevent dead code elimination */
    control = vs1 | vs2 | vc1 | vc2;
}

/* ========== Test 3: Sub-register accesses for SUBREG ========== */
typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));
typedef float v4sf __attribute__ ((vector_size (16)));

union MixedAccess {
    unsigned int full;
    struct {
        unsigned char b0;
        unsigned char b1;
        unsigned char b2;
        unsigned char b3;
    } bytes;
    struct {
        unsigned short low;
        unsigned short high;
    } words;
};

void test_subreg(void) {
    /* Vector operations */
    v4si vec_int = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    v4sf vec_float = {1.0f, 2.0f, 3.0f, 4.0f};
    
    /* Vector element access (triggers SUBREG) */
    int elem1 = vec_int[2];
    short elem2 = vec_short[5];
    float elem3 = vec_float[1];
    
    /* Vector operations that may create SUBREG */
    vec_int[0] = elem1 * 2;
    vec_short[3] = (short)(elem2 + elem1);
    
    /* Union type-punning */
    union MixedAccess u;
    u.full = 0x12345678;
    
    /* Access different views of same data */
    unsigned char byte_val = u.bytes.b2;
    unsigned short word_val = u.words.high;
    
    /* Mixed-type conversions */
    float f = (float)u.full;
    int i_from_f = (int)f;
    
    /* Packed structure */
    struct __attribute__((packed)) PackedStruct {
        char a;
        int b;
        short c;
    } ps = {1, 2, 3};
    
    /* Accessing packed members may require SUBREG */
    int b_val = ps.b;
    short c_val = ps.c;
    
    /* Prevent dead code elimination */
    control = elem1 | elem2 | byte_val | word_val | i_from_f | b_val | c_val;
}

/* ========== Test 4: Combined patterns ========== */
struct Combined {
    volatile unsigned int bits:5;
    volatile short partial;
    int full;
};

void test_combined(void) {
    struct Combined c = {0};
    
    /* Bit-field to partial register */
    c.bits = 15;
    c.partial = (short)c.bits * 10;
    
    /* Partial register to bit-field */
    c.bits = (c.partial >> 1) & 0x1F;
    
    /* Complex nested expression */
    volatile short temp;
    temp = (short)((c.bits << 3) | (c.partial & 0x7));
    c.full = (int)temp * 100;
    
    /* Union with bit-field and regular types */
    union {
        struct {
            volatile unsigned int low_bits:4;
            volatile unsigned int high_bits:4;
        } bf;
        volatile unsigned char byte;
    } u;
    
    u.bf.low_bits = 3;
    u.bf.high_bits = 5;
    c.partial = (short)u.byte;
    
    /* Loop combining patterns */
    for (unsigned int i = 0; i < 8; i++) {
        c.bits = i & 0x1F;
        c.partial = (short)(c.partial + c.bits);
        u.byte = (unsigned char)c.partial;
    }
    
    /* Prevent dead code elimination */
    control = c.bits | c.partial | c.full | u.byte;
}

/* ========== Architecture-specific tests ========== */
#ifdef __i386__
void test_x86_specific(void) {
    volatile int result;
    
    /* Inline assembly that might generate partial register ops */
    asm volatile (
        "movw $0x1234, %%ax\n\t"
        "movb $0x56, %%ah\n\t"
        "movw %%ax, %0\n\t"
        : "=r" (result)
        :
        : "%ax"
    );
    
    /* Bit test and set */
    unsigned int value = 0x12345678;
    unsigned char bit;
    asm volatile (
        "btl $5, %1\n\t"
        "setc %0\n\t"
        : "=r" (bit)
        : "r" (value)
        : "cc"
    );
    
    control = result | bit;
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    volatile int result;
    
    /* ARM inline assembly with register constraints */
    asm volatile (
        "uxth %0, %1\n\t"  /* Zero-extend halfword */
        : "=r" (result)
        : "r" (0x12345678)
    );
    
    /* Bit-field insert/extract */
    unsigned int val = 0x12345678;
    unsigned int extracted;
    asm volatile (
        "ubfx %0, %1, #8, #8\n\t"  /* Extract bits 8-15 */
        : "=r" (extracted)
        : "r" (val)
    );
    
    control = result | extracted;
}
#endif

/* ========== Test using builtins ========== */
void test_builtins(void) {
    unsigned int x = 0x12345678;
    
    /* Builtins that may involve bit manipulation */
    int count = __builtin_popcount(x);
    int parity = __builtin_parity(x);
    int clz = __builtin_clz(x);
    int ctz = __builtin_ctz(x);
    
    /* Bit reversal */
    unsigned int rev = __builtin_bswap32(x);
    
    /* Rotate operations */
    unsigned int rotated = __builtin_rotateright32(x, 8);
    
    /* Prevent dead code elimination */
    control = count | parity | clz | ctz | rev | rotated;
}

/* ========== Main test driver ========== */
typedef void (*test_func_t)(void);

const test_func_t test_functions[] = {
    test_bitfields,
    test_partial_registers,
    test_subreg,
    test_combined,
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
    
    /* Use command line or environment to control which tests run */
    int start_test = 0;
    int end_test = num_tests;
    
    if (argc > 1) {
        start_test = atoi(argv[1]) % num_tests;
        if (argc > 2) {
            end_test = atoi(argv[2]) % (num_tests + 1);
            if (end_test <= start_test) end_test = start_test + 1;
        }
    }
    
    if (end_test > num_tests) end_test = num_tests;
    
    printf("Running tests %d to %d\n", start_test, end_test - 1);
    
    /* Run selected tests */
    for (int i = start_test; i < end_test; i++) {
        test_functions[i]();
    }
    
    /* Final computation to ensure program does something */
    int result = 0;
    for (int i = 0; i < 100; i++) {
        result += control * i;
    }
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
