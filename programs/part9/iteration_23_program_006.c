/* Test program to trigger specific RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimizations from removing our test code */
static volatile int control = 0;

/* ==================== Bit-field tests for ZERO_EXTRACT ==================== */

struct BitFieldStruct {
    volatile unsigned int flag:1;
    volatile unsigned int value:10;
    unsigned int padding:21;
    volatile unsigned int mode:4;
    unsigned int :0; /* force alignment */
};

struct NestedBitField {
    struct {
        volatile unsigned int a:3;
        volatile unsigned int b:5;
    } inner;
    volatile unsigned int c:8;
};

void test_bitfields(void) {
    struct BitFieldStruct s1 = {0};
    struct NestedBitField s2 = {0};
    
    /* Simple bit-field assignments */
    s1.flag = 1;
    s1.value = 511; /* Max for 10 bits */
    s1.mode = 7;
    
    /* Bit-field reads */
    int read_flag = s1.flag;
    int read_value = s1.value;
    int read_mode = s1.mode;
    
    /* Cross assignments */
    s2.inner.a = s1.flag ? 3 : 0;
    s2.inner.b = read_value & 0x1F;
    s2.c = s1.mode << 1;
    
    /* Complex expression with bit-fields */
    s1.value = (s2.inner.a << 5) | s2.inner.b;
    
    /* Prevent dead code elimination */
    control = read_flag + read_value + read_mode;
}

/* ==================== Partial register tests for STRICT_LOW_PART ==================== */

void test_partial_registers(void) {
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    volatile int vi = 1000;
    
    /* Casts to smaller types */
    vs1 = (short)vi + 50;
    vc1 = (char)(vi >> 2);
    
    /* Arithmetic on sub-word types */
    vs2 = vs1 * 2;
    vc2 = vc1 + 1;
    
    /* Mixed-size operations */
    short result = (short)(vs1 + vc1);
    vs1 = result;
    
    /* Store partial results */
    *(volatile char*)&vc2 = (char)(vs1 & 0xFF);
    
    /* Prevent optimization */
    control = vs1 + vs2 + vc1 + vc2;
}

/* ==================== Sub-register tests for SUBREG ==================== */

typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));
typedef float v4sf __attribute__ ((vector_size (16)));

union MixedAccess {
    v4si vec;
    int array[4];
    struct {
        int a, b, c, d;
    } parts;
};

void test_subreg(void) {
    /* Vector operations */
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3;
    
    /* Vector element access - should generate SUBREG */
    int elem0 = v1[0];
    int elem2 = v1[2];
    
    /* Vector operations */
    v3 = v1 + v2;
    
    /* Store individual elements */
    v1[1] = elem0 + elem2;
    
    /* Mixed union access */
    union MixedAccess u;
    u.vec = v3;
    u.parts.a = 100;
    u.array[2] = 200;
    
    /* Float/int conversions */
    volatile float f = 3.14159f;
    volatile int fi = *(volatile int*)&f;  /* Type punning */
    volatile float f2 = (float)fi;
    
    /* Vector float operations */
    v4sf vf1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vf2 = vf1 * 2.0f;
    float f_elem = vf2[3];
    
    /* Prevent optimization */
    control = elem0 + elem2 + fi + (int)f_elem;
}

/* ==================== Combined pattern tests ==================== */

struct Combined {
    volatile unsigned int field1:4;
    volatile unsigned int field2:12;
    volatile short partial;
    int full;
};

void test_combined_patterns(void) {
    struct Combined c = {0};
    volatile short temp;
    
    /* Bit-field to partial register */
    c.field1 = 7;
    temp = (short)c.field1;  /* ZERO_EXTRACT -> SUBREG/STRICT_LOW_PART */
    
    /* Partial register to bit-field */
    c.field2 = temp * 2;
    
    /* Complex nested expression */
    c.partial = (short)((c.field1 << 8) | c.field2);
    
    /* Type punning through union */
    union {
        struct Combined s;
        unsigned int raw[2];
    } u;
    
    u.s = c;
    u.raw[1] = 0xDEADBEEF;
    
    /* Mixed operations */
    for (int i = 0; i < 4; i++) {
        c.field1 = (c.field1 + 1) & 0xF;
        c.partial += (short)c.field1;
    }
    
    /* Prevent optimization */
    control = temp + c.partial + u.raw[0];
}

/* ==================== Architecture-specific patterns ==================== */

#ifdef __i386__
void test_x86_specific(void) {
    volatile int result;
    
    /* Inline assembly that might generate partial register ops */
    __asm__ volatile (
        "movw $0x1234, %%ax\n\t"
        "movb $0x56, %%ah\n\t"
        "movw %%ax, %0\n\t"
        : "=m"(result)
        :
        : "ax"
    );
    
    /* Use builtins that may generate bit operations */
    unsigned int x = 0x12345678;
    int count = __builtin_popcount(x);
    int parity = __builtin_parity(x);
    
    control = result + count + parity;
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    volatile int result;
    
    /* ARM may generate interesting patterns for byte operations */
    volatile char bytes[4] = {1, 2, 3, 4};
    volatile int word;
    
    /* Byte accesses that might use SUBREG */
    word = (bytes[0] << 24) | (bytes[1] << 16) | 
           (bytes[2] << 8) | bytes[3];
    
    control = word;
}
#endif

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
    NULL
};

int main(int argc, char *argv[]) {
    int num_tests = 0;
    
    /* Use argc to control which tests run, preventing optimization */
    int test_mask = argc > 1 ? atoi(argv[1]) : 0xFF;
    
    printf("Running RTL pattern tests (mask: 0x%02x)\n", test_mask);
    
    /* Run selected tests */
    for (int i = 0; test_functions[i] != NULL; i++) {
        if (test_mask & (1 << i)) {
            test_functions[i]();
            num_tests++;
        }
    }
    
    /* Final computation to ensure program does something useful */
    int final_result = control + num_tests;
    
    printf("Completed %d tests, final result: %d\n", num_tests, final_result);
    
    /* Return something based on the tests to prevent dead code elimination */
    return final_result > 0 ? 0 : 1;
}
