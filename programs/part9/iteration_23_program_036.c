/* Comprehensive test for GCC RTL resource tracking patterns */
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
    volatile unsigned int small:3;
};

struct NestedBitField {
    struct {
        volatile unsigned int a:4;
        unsigned int b:4;
    } inner;
    volatile unsigned int c:8;
};

void test_bitfields(void) {
    struct BitFieldStruct s1 = {0};
    struct NestedBitField s2 = {{0, 0}, 0};
    
    /* Simple bit-field assignments */
    s1.flag = 1;
    s1.value = 511;  /* Max for 10 bits */
    s1.small = 5;    /* Actually 5 & 0x7 = 5 */
    
    /* Cross assignments */
    unsigned int temp = s1.value;
    s1.small = temp & 0x7;
    
    /* Complex expressions with bit-fields */
    s2.inner.a = (s1.flag << 1) | (s1.small & 0x3);
    s2.inner.b = s2.inner.a ^ 0xF;
    s2.c = s2.inner.a + s2.inner.b;
    
    /* Bit-field in conditional */
    if (s1.flag && (s1.value > 100)) {
        s1.small = 2;
    }
    
    /* Loop with bit-field updates */
    for (int i = 0; i < 4; i++) {
        s2.c = (s2.c << 1) | (s2.inner.a & 0x1);
        s2.inner.a >>= 1;
    }
}

/* ========== PARTIAL REGISTER TESTS (STRICT_LOW_PART) ========== */

void test_partial_registers(void) {
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    volatile int vi = 100;
    
    /* Casts to smaller types */
    vs1 = (short)vi + 50;
    vc1 = (char)(vi * 2);
    
    /* Arithmetic on sub-word types */
    vs2 = vs1 * 2;
    vc2 = vc1 + 10;
    
    /* Complex partial updates */
    for (int i = 0; i < 10; i++) {
        vs1 = (short)(vs1 + vs2);
        vc1 = (char)(vc1 - vc2);
    }
    
    /* Mixed-size operations */
    int result = (int)vs1 + (int)vc1 * 256;
    vs2 = (short)(result & 0xFFFF);
    
    /* Pointer casts for partial access */
    volatile int *pvi = &vi;
    vs1 = *(volatile short *)pvi;  /* Load low half */
    *(volatile char *)((char *)pvi + 2) = vc1;  /* Store to middle byte */
}

/* ========== SUBREG TESTS ========== */

/* Vector types for SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Packed structure for SUBREG access */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

union TypePunning {
    float f;
    int i;
    char bytes[4];
};

void test_subregs(void) {
    /* Vector operations */
    v4si vec_int = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    v4sf vec_float = {1.0f, 2.0f, 3.0f, 4.0f};
    
    /* Vector element access (generates SUBREG) */
    int elem_int = vec_int[2];
    short elem_short = vec_short[5];
    float elem_float = vec_float[1];
    
    /* Vector operations with mixing */
    vec_int[0] = elem_int + 10;
    vec_short[3] = (short)elem_float;
    
    /* Packed structure access */
    struct PackedStruct ps;
    ps.a = 0x12;
    ps.b = 0x12345678;
    ps.c = 0x9ABC;
    
    /* These generate SUBREG due to misalignment */
    int b_val = ps.b;  /* May be unaligned access */
    short c_val = ps.c;
    
    /* Type punning through union */
    union TypePunning up;
    up.f = 3.14159f;
    
    /* Access different views of same memory */
    int int_view = up.i;
    char byte_view = up.bytes[2];
    
    /* Float/int conversions */
    float f = (float)int_view;
    int i = (int)f;
    
    /* Mixed vector/scalar operations */
    vec_float[0] = f;
    vec_int[1] = i;
}

/* ========== COMBINED PATTERN TESTS ========== */

struct Combined {
    volatile unsigned int bits:8;
    volatile short partial;
    int full;
};

void test_combined_patterns(void) {
    struct Combined c = {0};
    volatile short vs;
    
    /* Bit-field to partial register */
    c.bits = 0xAB;
    vs = (short)c.bits;  /* ZERO_EXTRACT + SUBREG/STRICT_LOW_PART */
    
    /* Partial register to bit-field */
    c.bits = vs & 0xFF;
    
    /* Complex expression chain */
    for (int i = 0; i < 8; i++) {
        c.bits = (c.bits << 1) | ((vs >> i) & 0x1);
        vs = (short)((vs * 3) + c.bits);
    }
    
    /* Union with bit-field and full int */
    union {
        struct {
            volatile unsigned int low:16;
            volatile unsigned int high:16;
        } bits;
        volatile int full;
    } u;
    
    u.full = 0x12345678;
    u.bits.low = u.bits.high;  /* Bit-field to bit-field */
    vs = (short)u.bits.low;    /* To partial register */
    
    /* Nested extractions */
    c.partial = (short)((u.full >> 8) & 0xFF);
    c.bits = (c.partial & 0xF) | ((vs & 0xF) << 4);
}

/* ========== ARCHITECTURE-SPECIFIC TESTS ========== */

#ifdef __i386__
void test_x86_specific(void) {
    volatile int result;
    
    /* Inline assembly that might generate partial register ops */
    __asm__ volatile (
        "movw $0x1234, %%ax\n\t"
        "addw $0x5678, %%ax\n\t"
        "movw %%ax, %0"
        : "=m" (result)
        : 
        : "ax"
    );
    
    /* Bit test operations */
    unsigned int value = 0x87654321;
    unsigned int bit = __builtin_parity(value);  /* May use bit extraction */
    result = __builtin_clz(value);               /* Count leading zeros */
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    volatile int result;
    
    /* ARM-specific bit-field operations */
    unsigned int value = 0xA5A5A5A5;
    
    /* Use ARM bit-field insert/extract intrinsics if available */
    result = __builtin_arm_bfx(value, 4, 8);  /* Extract bits 4-11 */
    
    /* Load/store with byte or halfword */
    volatile short hs;
    __asm__ volatile (
        "ldrh %0, [%1]\n\t"
        : "=r" (hs)
        : "r" (&value)
    );
}
#endif

/* ========== BUILTIN BIT OPERATIONS ========== */

void test_builtins(void) {
    unsigned int x = 0xDEADBEEF;
    volatile int result;
    
    /* Builtins that may generate extraction patterns */
    result = __builtin_popcount(x);          /* Population count */
    result = __builtin_ffs(x);               /* Find first set */
    result = __builtin_ctz(x);               /* Count trailing zeros */
    
    /* Parity - may use bit extraction */
    result = __builtin_parity(x);
    
    /* Byte swap - involves SUBREG operations */
    result = __builtin_bswap32(x);
    
    /* Rotate operations */
    result = __builtin_rotateright32(x, 3);
    result = __builtin_rotateleft32(x, 5);
}

/* ========== MAIN TEST DRIVER ========== */

typedef void (*test_func_t)(void);

static test_func_t test_functions[] = {
    test_bitfields,
    test_partial_registers,
    test_subregs,
    test_combined_patterns,
    test_builtins,
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
    
    /* Use command line or control variable to select test */
    if (argc > 1) {
        test_to_run = atoi(argv[1]) % 8;
    } else {
        test_to_run = control % 8;
    }
    
    /* Run selected test(s) */
    for (int i = 0; i <= test_to_run && test_functions[i]; i++) {
        test_functions[i]();
    }
    
    /* Ensure some computation happens regardless */
    volatile int checksum = 0;
    for (int i = 0; i < argc; i++) {
        for (char *p = argv[i]; *p; p++) {
            checksum += *p;
        }
    }
    
    printf("Test completed with checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
