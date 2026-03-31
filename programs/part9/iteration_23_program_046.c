/* Test program to trigger uncovered RTL patterns in GCC's resource.cc */
#include <stdint.h>
#include <stdlib.h>

/* Prevent optimizations from removing our test code */
static volatile int global_volatile = 0;

/* ==================== BIT-FIELD TESTS (ZERO_EXTRACT) ==================== */

struct bitfield_struct {
    volatile unsigned int flag:1;
    unsigned int value:10;
    unsigned int padding:21;
    volatile unsigned int another:4;
};

struct nested_bitfield {
    struct {
        volatile unsigned int low:8;
        unsigned int high:8;
    } parts;
    unsigned int full;
};

void test_bitfields(void) {
    struct bitfield_struct bs = {0};
    struct nested_bitfield nb = {0};
    
    /* Simple bitfield assignments - should generate ZERO_EXTRACT */
    bs.flag = 1;
    bs.value = 512;
    bs.another = 7;
    
    /* Bitfield reads */
    unsigned int read_flag = bs.flag;
    unsigned int read_value = bs.value;
    
    /* Cross assignments between bitfields */
    bs.value = bs.another;
    bs.another = bs.flag;
    
    /* Complex bitfield expression */
    bs.value = (bs.flag << 9) | (bs.another << 5);
    
    /* Nested bitfield access */
    nb.parts.low = 0xFF;
    nb.parts.high = 0xAA;
    nb.full = (nb.parts.high << 8) | nb.parts.low;
    
    /* Prevent dead code elimination */
    global_volatile = read_flag + read_value;
}

/* ==================== PARTIAL REGISTER TESTS (STRICT_LOW_PART) ==================== */

void test_partial_registers(void) {
    volatile short vs;
    volatile char vc;
    volatile int vi = global_volatile;
    
    /* Partial register assignments */
    vs = (short)(vi + 100);
    vc = (char)(vs * 2);
    
    /* Arithmetic on sub-word types */
    short s1 = 100, s2 = 200;
    volatile short s3 = s1 + s2;
    
    char c1 = 50, c2 = 75;
    volatile char c3 = c1 - c2;
    
    /* Mixed-size operations */
    int result = (short)vi + (char)vc;
    vs = (short)result;
    
    /* Compound assignment on partial types */
    s3 += 10;
    c3 -= 5;
    
    /* Prevent optimization */
    global_volatile = s3 + c3;
}

/* ==================== SUBREG TESTS ==================== */

typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));

union type_pun {
    float f;
    int i;
    char bytes[4];
};

void test_subregs(void) {
    /* Vector operations - should generate SUBREG */
    v4si v = {1, 2, 3, 4};
    v8hi vh = {1, 2, 3, 4, 5, 6, 7, 8};
    
    /* Vector element access */
    int element = v[2];
    short helement = vh[5];
    
    /* Vector operations */
    v4si v2 = v + (v4si){10, 20, 30, 40};
    v8hi vh2 = vh * (v8hi){2, 2, 2, 2, 2, 2, 2, 2};
    
    /* Type punning through union */
    union type_pun pun;
    pun.f = 3.14159f;
    int int_from_float = pun.i;
    char byte_from_float = pun.bytes[0];
    
    /* Float/integer conversions */
    float f = (float)element;
    int i = (int)f;
    
    /* Prevent optimization */
    global_volatile = element + helement + int_from_float + byte_from_float;
}

/* ==================== COMBINED PATTERN TESTS ==================== */

struct combined {
    volatile unsigned int bf1:3;
    volatile unsigned int bf2:5;
    volatile short partial;
    int full;
};

void test_combined_patterns(void) {
    struct combined c = {0};
    
    /* Bit-field to partial register */
    c.bf1 = 5;
    c.partial = (short)c.bf1;
    
    /* Partial register to bit-field */
    c.bf2 = (unsigned int)c.partial;
    
    /* Complex expression combining patterns */
    c.full = (c.bf1 << 16) | (c.bf2 << 8) | (unsigned short)c.partial;
    
    /* Nested operations */
    volatile short temp = (short)((c.bf1 << 3) | c.bf2);
    c.bf1 = (temp >> 3) & 0x7;
    c.bf2 = temp & 0x1F;
    
    /* Loop with combined patterns */
    for (int i = 0; i < 4; i++) {
        c.bf1 = (c.bf1 + 1) & 0x7;
        c.partial = (short)(c.partial + c.bf1);
        c.bf2 = (c.bf2 + c.partial) & 0x1F;
    }
    
    global_volatile = c.full;
}

/* ==================== ARCHITECTURE-SPECIFIC TESTS ==================== */

#ifdef __i386__
void test_x86_specific(void) {
    volatile int result;
    
    /* Inline assembly that might generate STRICT_LOW_PART */
    asm volatile (
        "movw $0x1234, %%ax\n\t"
        "movb $0x56, %%ah\n\t"
        "movw %%ax, %0\n\t"
        : "=m" (result)
        : 
        : "ax"
    );
    
    /* Bit manipulation builtins */
    unsigned int x = 0x12345678;
    int count = __builtin_popcount(x);
    int parity = __builtin_parity(x);
    
    global_volatile = count + parity;
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    volatile int result;
    
    /* ARM-specific operations that might generate SUBREG */
    asm volatile (
        "uxth %0, %1\n\t"
        : "=r" (result)
        : "r" (0x12345678)
    );
    
    global_volatile = result;
}
#endif

/* ==================== MAIN TEST DRIVER ==================== */

typedef void (*test_func_t)(void);

static test_func_t test_functions[] = {
    test_bitfields,
    test_partial_registers,
    test_subregs,
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
    
    /* Use command line or volatile to control which tests run */
    if (argc > 1) {
        test_to_run = atoi(argv[1]) % 10;
    } else {
        test_to_run = global_volatile % 10;
    }
    
    /* Run all tests in sequence to ensure all RTL patterns are generated */
    for (int i = 0; test_functions[i] != NULL; i++) {
        if (test_to_run == 0 || test_to_run == i + 1) {
            test_functions[i]();
        }
    }
    
    /* Final computation to ensure program does something useful */
    int final_result = global_volatile * 2 + argc;
    
    return final_result > 0 ? 0 : 1;
}
