/* Test program to trigger specific RTL patterns in GCC's resource tracking */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimizations from removing our test code */
static volatile int control = 0;

/* ========== BIT-FIELD TESTS (ZERO_EXTRACT) ========== */

struct BitFieldStruct {
    volatile unsigned int flag:1;
    unsigned int value:10;
    unsigned int padding:21;
    volatile unsigned int mode:4;
    unsigned int data:8;
};

struct NestedBitField {
    struct {
        volatile unsigned int a:3;
        unsigned int b:5;
    } inner;
    volatile unsigned int c:7;
    unsigned int d:9;
};

void test_bitfields(void) {
    struct BitFieldStruct s1 = {0};
    struct NestedBitField s2 = {0};
    
    /* Simple bit-field assignments */
    s1.flag = 1;
    s1.value = 511;  /* Max for 10 bits */
    s1.mode = 7;
    s1.data = 42;
    
    /* Bit-field reads */
    unsigned int x = s1.flag;
    unsigned int y = s1.value;
    unsigned int z = s1.mode | s1.data;
    
    /* Nested bit-field operations */
    s2.inner.a = 3;
    s2.inner.b = 20;
    s2.c = 63;
    s2.d = 256;
    
    /* Complex bit-field expression */
    unsigned int result = (s2.inner.a << 8) | (s2.inner.b << 3) | s2.c;
    
    /* Prevent dead code elimination */
    control = x + y + z + result;
}

/* ========== PARTIAL REGISTER TESTS (STRICT_LOW_PART) ========== */

void test_partial_registers(void) {
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    volatile int vi = 1000;
    
    /* Partial register assignments */
    vs1 = (short)vi + 50;
    vs2 = (short)(vi * 2);
    
    /* Char operations that may use partial registers */
    vc1 = (char)(vi & 0xFF);
    vc2 = (char)((vi >> 8) & 0xFF);
    
    /* Arithmetic on partial registers */
    vs1 = vs1 + vs2;
    vc1 = vc1 - vc2;
    
    /* Mixed-size operations */
    int temp = vs1;
    vs2 = (short)(temp / 2);
    
    /* Prevent optimization */
    control = vs1 + vs2 + vc1 + vc2;
}

/* ========== SUBREG TESTS ========== */

/* Vector type using GCC extension */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

/* Packed structure for SUBREG access */
struct PackedData {
    unsigned char a;
    unsigned short b;
    unsigned int c;
} __attribute__((packed));

union TypePun {
    float f;
    int i;
    unsigned char bytes[4];
};

void test_subreg(void) {
    /* Vector operations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;
    
    /* Vector element access (likely generates SUBREG) */
    int elem = vec3[2];
    vec1[0] = elem * 2;
    
    /* Vector type conversion */
    v8hi short_vec = {1, 2, 3, 4, 5, 6, 7, 8};
    short_vec[3] = (short)elem;
    
    /* Packed structure access */
    struct PackedData pd;
    pd.a = 0xAA;
    pd.b = 0xBBBB;
    pd.c = 0xCCCCCCCC;
    
    /* Accessing misaligned data through different types */
    unsigned int from_packed = pd.c;
    pd.b = (unsigned short)(from_packed >> 8);
    
    /* Type punning through union */
    union TypePun pun;
    pun.f = 3.14159f;
    pun.i = pun.i ^ 0x80000000;  /* Flip sign bit */
    pun.bytes[1] = 0xFF;
    
    /* Prevent optimization */
    control = elem + vec1[0] + short_vec[3] + from_packed + pun.i;
}

/* ========== COMBINED PATTERN TESTS ========== */

struct Combined {
    volatile unsigned int bits:12;
    volatile short partial;
    unsigned char byte;
} __attribute__((packed));

void test_combined_patterns(void) {
    struct Combined c = {0};
    
    /* Combined bit-field and partial register */
    c.bits = 0xABC;
    c.partial = (short)c.bits;
    
    /* Complex expression with multiple patterns */
    unsigned int temp = c.bits;
    c.byte = (unsigned char)((temp >> 4) & 0xFF);
    c.partial = c.partial + (short)c.byte;
    
    /* Loop with combined operations */
    for (int i = 0; i < 4; i++) {
        c.bits = (c.bits << 1) | (c.bits >> 11);  /* Rotate */
        c.partial = (short)(c.partial ^ c.bits);
        c.byte = c.byte + 1;
    }
    
    /* Prevent optimization */
    control = c.bits + c.partial + c.byte;
}

/* ========== ARCHITECTURE-SPECIFIC TESTS ========== */

#ifdef __i386__
void test_x86_specific(void) {
    volatile int result;
    
    /* Inline assembly that might generate STRICT_LOW_PART */
    __asm__ volatile (
        "movw $0x1234, %%ax\n\t"
        "movw %%ax, %0\n\t"
        : "=m" (result)
        : 
        : "%ax"
    );
    
    /* Bit manipulation builtins */
    unsigned int x = 0x12345678;
    int count = __builtin_popcount(x);
    int parity = __builtin_parity(x);
    int clz = __builtin_clz(x);
    
    control = result + count + parity + clz;
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    volatile int result;
    
    /* ARM-specific operations */
    __asm__ volatile (
        "mov r0, #0xFF\n\t"
        "and r0, r0, #0xF0\n\t"
        "str r0, %0\n\t"
        : "=m" (result)
        :
        : "r0"
    );
    
    control = result;
}
#endif

/* ========== MAIN TEST DRIVER ========== */

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
    NULL
};

int main(int argc, char *argv[]) {
    int test_to_run = 0;
    
    /* Use command line or environment to control which tests run */
    if (argc > 1) {
        test_to_run = atoi(argv[1]) % 10;
    } else {
        /* Use a volatile variable to prevent compile-time optimization */
        test_to_run = control % (sizeof(test_functions)/sizeof(test_functions[0]) - 1);
    }
    
    /* Run selected test */
    if (test_functions[test_to_run]) {
        test_functions[test_to_run]();
    }
    
    /* Run all tests in a loop to ensure code generation */
    for (int i = 0; test_functions[i] != NULL; i++) {
        if (i != test_to_run) {
            test_functions[i]();
        }
    }
    
    /* Final computation to ensure program does something useful */
    printf("Control value: %d\n", control);
    
    return control != 0 ? 0 : 1;
}
