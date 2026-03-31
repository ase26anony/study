/* Test program to trigger specific RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimization of critical variables */
#define NOOPT __attribute__((optimize("O0")))
#define VOLATILE volatile

/* ==================== Bit-field patterns for ZERO_EXTRACT ==================== */

NOOPT void test_bitfields(void) {
    /* Structure with various bit-field widths */
    struct BitFieldStruct {
        VOLATILE unsigned int flag:1;
        VOLATILE unsigned int value:10;
        VOLATILE unsigned int mode:3;
        unsigned int pad:18;
    } s1, s2;
    
    /* Union with bit-fields and full integer for type-punning */
    union BitFieldUnion {
        struct {
            VOLATILE unsigned int low:8;
            VOLATILE unsigned int high:8;
        } parts;
        VOLATILE unsigned int whole;
    } u;
    
    /* Initialize with non-zero values */
    s1.flag = 1;
    s1.value = 511;  /* Max 10-bit value */
    s1.mode = 7;
    
    /* Bit-field to bit-field assignment */
    s2.flag = s1.flag;
    s2.value = s1.value;
    s2.mode = s1.mode;
    
    /* Bit-field to/from integer */
    unsigned int temp = s1.value;
    s2.value = temp + 1;
    
    /* Union bit-field access */
    u.whole = 0x1234;
    unsigned int low_val = u.parts.low;
    unsigned int high_val = u.parts.high;
    u.parts.high = low_val;
    
    /* Complex expression with bit-fields */
    s1.value = (s1.flag ? s2.value : s1.value) & 0x1FF;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(&s1), "r"(&s2), "r"(&u));
}

/* ==================== Partial register patterns for STRICT_LOW_PART ==================== */

NOOPT void test_partial_registers(void) {
    /* Volatile sub-word types */
    VOLATILE char vc;
    VOLATILE short vs;
    VOLATILE unsigned char vuc;
    VOLATILE unsigned short vus;
    
    int i = 100;
    long l = 1000;
    
    /* Assignments to partial registers */
    vc = (char)i;
    vs = (short)l;
    vuc = (unsigned char)(i + 50);
    vus = (unsigned short)(l - 500);
    
    /* Arithmetic on partial registers */
    vc = vc + 1;
    vs = vs * 2;
    vuc = vuc - 1;
    vus = vus / 2;
    
    /* Complex expressions with partial registers */
    vc = (char)((i & 0xFF) + (vc * 2));
    vs = (short)((l >> 8) | (vs & 0x00FF));
    
    /* Mixed-size operations */
    vus = (unsigned short)((unsigned int)vus + 256);
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(&vc), "r"(&vs), "r"(&vuc), "r"(&vus));
}

/* ==================== Sub-register patterns for SUBREG ==================== */

NOOPT void test_subregisters(void) {
    /* Vector extension for SUBREG patterns */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    VOLATILE v4si v1 = {1, 2, 3, 4};
    VOLATILE v8hi v2 = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Access vector elements (creates SUBREG) */
    int elem1 = v1[0];
    int elem2 = v1[2];
    short selem1 = v2[1];
    short selem3 = v2[3];
    
    /* Modify vector elements */
    v1[1] = elem1 + elem2;
    v2[4] = (short)(selem1 * selem3);
    
    /* Type punning through union */
    union TypePun {
        float f;
        int i;
        char c[4];
    } tp;
    
    tp.f = 3.14159f;
    int int_from_float = tp.i;  /* SUBREG from float to int */
    tp.i = 0x40490FDA;          /* SUBREG from int to float */
    float float_from_int = tp.f;
    
    /* Packed structure */
    struct __attribute__((packed)) PackedStruct {
        char a;
        int b;
        short c;
    } ps;
    
    ps.a = 'X';
    ps.b = 0x12345678;
    ps.c = 0x9ABC;
    
    /* Access causes SUBREG due to misalignment */
    int b_val = ps.b;
    short c_val = ps.c;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(&v1), "r"(&v2), "r"(&tp), "r"(&ps));
}

/* ==================== Combined patterns ==================== */

NOOPT void test_combined_patterns(void) {
    /* Structure with bit-fields */
    struct {
        VOLATILE unsigned int data:12;
        VOLATILE unsigned int control:4;
    } device_reg;
    
    /* Volatile partial register */
    VOLATILE short output;
    
    /* Bit-field read to partial register (ZERO_EXTRACT -> SUBREG/STRICT_LOW_PART) */
    device_reg.data = 0xABC;
    output = (short)device_reg.data;
    
    /* Complex expression combining patterns */
    device_reg.control = (device_reg.data >> 8) & 0xF;
    output = (short)((device_reg.control << 4) | (device_reg.data & 0xF));
    
    /* Union with bit-fields and vector */
    union {
        struct {
            unsigned int a:4;
            unsigned int b:4;
            unsigned int c:4;
            unsigned int d:4;
        } bits;
        unsigned short words[2];
        unsigned int whole;
    } comb;
    
    comb.whole = 0x12345678;
    unsigned short low_word = comb.words[0];  /* SUBREG access */
    comb.bits.a = low_word & 0xF;             /* ZERO_EXTRACT */
    
    /* Loop to ensure RTL generation */
    for (int i = 0; i < 4; i++) {
        device_reg.data = (device_reg.data << 3) | (device_reg.control & 0x7);
        output = (short)(output + device_reg.data);
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(&device_reg), "r"(&output), "r"(&comb));
}

/* ==================== Architecture-specific patterns ==================== */

NOOPT void test_arch_specific(void) {
    /* Use builtins that may generate bit manipulation RTL */
    unsigned int x = 0x12345678;
    int count_leading_zeros = __builtin_clz(x);
    int parity = __builtin_parity(x);
    int popcount = __builtin_popcount(x);
    
    /* Bit extraction builtins */
    unsigned int extracted = __builtin_extract(x, 4, 8);  /* Extract 8 bits from position 4 */
    
    /* Architecture-specific inline assembly */
#ifdef __i386__
    /* x86 partial register access */
    asm volatile (
        "movw %1, %%ax\n\t"
        "addw $1, %%ax\n\t"
        "movw %%ax, %0"
        : "=m" (*(VOLATILE short*)&x)
        : "i" (100)
        : "ax"
    );
#endif
    
#ifdef __arm__
    /* ARM bit-field operations */
    asm volatile (
        "bfi %0, %1, #4, #8"
        : "+r" (x)
        : "r" (0xFF)
    );
#endif
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(&x), "r"(&count_leading_zeros), 
                 "r"(&parity), "r"(&popcount), "r"(&extracted));
}

/* ==================== Main test driver ==================== */

typedef void (*test_func_t)(void);

const test_func_t test_functions[] = {
    test_bitfields,
    test_partial_registers,
    test_subregisters,
    test_combined_patterns,
    test_arch_specific,
    NULL
};

int main(int argc, char *argv[]) {
    VOLATILE int test_to_run = 0;
    
    /* Use command line or environment to control which tests run */
    if (argc > 1) {
        test_to_run = atoi(argv[1]) % 5;
    }
    
    /* Run all tests or specific test based on input */
    if (test_to_run == 0) {
        /* Run all tests */
        for (int i = 0; test_functions[i] != NULL; i++) {
            test_functions[i]();
        }
    } else {
        /* Run specific test */
        test_functions[test_to_run - 1]();
    }
    
    /* Final computation to ensure program does something */
    VOLATILE int result = 0;
    for (int i = 0; i < 100; i++) {
        result += i * i;
    }
    
    printf("Test completed. Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
