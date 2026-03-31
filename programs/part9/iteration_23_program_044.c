/* Test program to trigger uncovered RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimization of critical variables */
#define NOOPT __attribute__((optimize("O0")))
#define VOLATILE volatile

/* ==================== BIT-FIELD PATTERNS (ZERO_EXTRACT) ==================== */

NOOPT void test_bitfields(void) {
    /* Structure with various bit-field widths */
    struct BitStruct {
        VOLATILE unsigned int flag:1;
        VOLATILE unsigned int value:10;
        VOLATILE unsigned int mode:3;
        unsigned int pad:18;
    } bs;
    
    /* Union with bit-fields and full integer for type-punning */
    union BitUnion {
        struct {
            VOLATILE unsigned int low:8;
            VOLATILE unsigned int high:8;
            VOLATILE unsigned int ext:16;
        } bits;
        VOLATILE unsigned int full;
    } bu;
    
    /* Initialize */
    bs.flag = 1;
    bs.value = 511;  /* Max 10-bit value */
    bs.mode = 7;
    
    bu.bits.low = 0xAA;
    bu.bits.high = 0xBB;
    bu.bits.ext = 0xCCDD;
    
    /* Complex bit-field operations */
    unsigned int temp = bs.value;
    bs.mode = (temp & 0x7);  /* Extract lower 3 bits */
    
    /* Cross-structure bit-field assignment */
    struct BitStruct bs2 = {0};
    bs2.flag = bs.flag;
    bs2.value = bs.value >> 2;
    
    /* Bit-field in conditional */
    if (bs.flag && (bs.value > 100)) {
        bs.mode = 3;
    }
    
    /* Union access through both bit-field and full integer */
    unsigned int full_val = bu.full;
    bu.bits.low = (full_val >> 4) & 0xF;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(&bs), "r"(&bu));
}

/* ==================== PARTIAL REGISTER PATTERNS (STRICT_LOW_PART) ==================== */

NOOPT void test_partial_registers(void) {
    /* Volatile sub-word types */
    VOLATILE char vc;
    VOLATILE short vs;
    VOLATILE signed char sc;
    
    int i = 100;
    long l = 1000L;
    
    /* Assignments to partial registers */
    vc = (char)i + 5;
    vs = (short)l - 50;
    sc = (signed char)(i * 2);
    
    /* Arithmetic on partial registers */
    vs = vs + (short)10;
    vc = vc * 2;
    
    /* Complex expression with partial register result */
    int result = (int)vc + (int)vs;
    vs = (short)(result & 0xFFFF);
    
    /* Pointer to partial register type */
    short *ps = &vs;
    *ps = (short)(*ps + 1);
    
    /* Array of sub-word types */
    VOLATILE char arr[10];
    for (int j = 0; j < 10; j++) {
        arr[j] = (char)(j * 11);
    }
    
    /* Mixed-size operations */
    char c1 = arr[0];
    short s1 = (short)c1 * 10;
    arr[1] = (char)(s1 >> 2);
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(&vc), "r"(&vs), "r"(&sc), "r"(arr));
}

/* ==================== SUB-REGISTER PATTERNS (SUBREG) ==================== */

NOOPT void test_subregisters(void) {
    /* GCC vector extension */
    typedef int v4si __attribute__ ((vector_size (16)));
    typedef short v8hi __attribute__ ((vector_size (16)));
    
    v4si v = {1, 2, 3, 4};
    v8hi w = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Vector element access (creates SUBREG) */
    int elem = v[2];
    short selem = w[3];
    
    /* Vector operations */
    v4si v2 = v + (v4si){5, 5, 5, 5};
    v[0] = v2[1];
    
    /* Type punning through union */
    union TypePun {
        float f;
        int i;
        char c[4];
    } tp;
    
    tp.f = 3.14159f;
    int int_from_float = tp.i;  /* SUBREG from float to int access */
    char byte = tp.c[2];        /* SUBREG for byte access */
    
    /* Packed structure */
    struct __attribute__((packed)) Packed {
        char a;
        int b;
        short c;
    } p;
    
    p.a = 'X';
    p.b = 0x12345678;
    p.c = 0x9ABC;
    
    /* Access different-sized members */
    short c_val = p.c;
    p.b = p.b + (int)c_val;
    
    /* Float/double conversions */
    VOLATILE float fv = 1.5f;
    VOLATILE double dv = 2.71828;
    
    /* Conversions that may create SUBREG */
    int int_from_double = (int)dv;
    float from_int = (float)(int_from_double + 10);
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(&v), "r"(&w), "r"(&tp), "r"(&p), "r"(&fv), "r"(&dv));
}

/* ==================== COMBINED PATTERNS ==================== */

NOOPT void test_combined_patterns(void) {
    /* Structure with bit-fields */
    struct {
        VOLATILE unsigned int data:12;
        VOLATILE unsigned int control:4;
    } device;
    
    device.data = 0xFFF;
    device.control = 0xA;
    
    /* Bit-field to partial register */
    VOLATILE short partial;
    partial = (short)device.data;  /* ZERO_EXTRACT to SUBREG/STRICT_LOW_PART */
    
    /* Modify and store back */
    device.data = (unsigned int)partial + 1;
    
    /* Union with bit-fields and vector */
    union Complex {
        struct {
            unsigned int a:4;
            unsigned int b:4;
            unsigned int c:4;
            unsigned int d:4;
        } bits;
        short halves[2];
        int full;
    } comp;
    
    comp.bits.a = 1;
    comp.bits.b = 2;
    comp.bits.c = 3;
    comp.bits.d = 4;
    
    /* Access through different views */
    short first_half = comp.halves[0];  /* SUBREG access */
    comp.full = comp.full >> 4;         /* May create ZERO_EXTRACT */
    
    /* Nested bit-field operations */
    struct Nested {
        struct {
            VOLATILE unsigned int x:5;
            VOLATILE unsigned int y:5;
        } inner;
        VOLATILE unsigned int z:6;
    } nested;
    
    nested.inner.x = 10;
    nested.inner.y = 20;
    nested.z = nested.inner.x + nested.inner.y;
    
    /* Complex expression combining patterns */
    VOLATILE char final_result;
    final_result = (char)((device.data & 0xFF) + (partial & 0xFF));
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(&device), "r"(&partial), "r"(&comp), "r"(&nested));
}

/* ==================== ARCHITECTURE-SPECIFIC PATTERNS ==================== */

NOOPT void test_arch_specific(void) {
    /* Use architecture-specific builtins */
    
    /* Bit manipulation builtins (may create ZERO_EXTRACT) */
    unsigned int x = 0x12345678;
    int count = __builtin_popcount(x);      /* Population count */
    int parity = __builtin_parity(x);       /* Parity */
    int leading_zeros = __builtin_clz(x);   /* Count leading zeros */
    
    /* Byte swap (may involve SUBREG) */
    unsigned int swapped = __builtin_bswap32(x);
    
    /* Conditional mix */
    VOLATILE short arch_partial;
    arch_partial = (short)(swapped & 0xFFFF);
    
#ifdef __i386__
    /* x86-specific inline assembly for partial register access */
    asm volatile (
        "movw %1, %%ax\n\t"
        "addw $1, %%ax\n\t"
        "movw %%ax, %0"
        : "=r" (arch_partial)
        : "r" (arch_partial)
        : "ax"
    );
#endif
    
#ifdef __arm__
    /* ARM-specific patterns */
    asm volatile (
        "uxth %0, %1"
        : "=r" (arch_partial)
        : "r" (x)
    );
#endif
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(&x), "r"(&swapped), "r"(&arch_partial));
}

/* ==================== MAIN TEST DRIVER ==================== */

typedef void (*test_func_t)(void);

static const test_func_t test_functions[] = {
    test_bitfields,
    test_partial_registers,
    test_subregisters,
    test_combined_patterns,
    test_arch_specific,
    NULL
};

int main(int argc, char *argv[]) {
    VOLATILE int test_to_run = 0;
    
    /* Use command-line argument to select test, preventing optimization */
    if (argc > 1) {
        test_to_run = atoi(argv[1]) % 5;
    }
    
    /* Run all tests or specific one based on input */
    if (test_to_run == 0) {
        /* Run all tests */
        for (int i = 0; test_functions[i] != NULL; i++) {
            test_functions[i]();
        }
    } else {
        /* Run specific test */
        test_functions[test_to_run - 1]();
    }
    
    /* Final computation to ensure program does something useful */
    VOLATILE int final_result = 0;
    for (int i = 0; i < 100; i++) {
        final_result += i * i;
    }
    
    printf("Test completed. Final result: %d\n", final_result);
    
    return final_result > 0 ? 0 : 1;
}
