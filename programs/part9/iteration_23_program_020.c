/* Test program to trigger specific RTL patterns in GCC's resource.cc */
#include <stdint.h>
#include <string.h>

/* Prevent optimization of critical variables */
#define NOOPT __attribute__((optimize("O0")))
#define VOLATILE_VAR volatile

/* ==================== Bit-field patterns (ZERO_EXTRACT) ==================== */

NOOPT void test_bitfields(void) {
    /* Basic bit-field structure */
    struct BitFieldStruct {
        VOLATILE_VAR unsigned int flag:1;
        VOLATILE_VAR unsigned int value:10;
        VOLATILE_VAR unsigned int mode:3;
        unsigned int padding:18;
    };
    
    struct BitFieldStruct s1 = {0};
    struct BitFieldStruct s2 = {0};
    
    /* Multiple bit-field assignments to trigger ZERO_EXTRACT */
    s1.flag = 1;
    s1.value = 511;  /* Max for 10 bits */
    s1.mode = 3;
    
    /* Cross-structure bit-field copy */
    s2.flag = s1.flag;
    s2.value = s1.value + 1;
    s2.mode = s1.mode ^ 1;
    
    /* Bit-field in conditional */
    if (s1.flag && s2.value > 100) {
        s1.mode = 2;
    }
    
    /* Complex expression with bit-field */
    unsigned int temp = (s1.value << 3) | s1.mode;
    s2.value = temp & 0x3FF;  /* Mask to 10 bits */
}

NOOPT void test_nested_bitfields(void) {
    /* Nested bit-field access in larger expression */
    struct NestedBF {
        VOLATILE_VAR unsigned int a:4;
        VOLATILE_VAR unsigned int b:4;
        VOLATILE_VAR unsigned int c:4;
        VOLATILE_VAR unsigned int d:4;
    };
    
    struct NestedBF n = {0};
    VOLATILE_VAR unsigned int result = 0;
    
    /* Chain of bit-field operations */
    n.a = 5;
    n.b = n.a + 3;
    n.c = n.b >> 1;
    n.d = n.c ^ n.a;
    
    /* Combine bit-fields */
    result = (n.a << 12) | (n.b << 8) | (n.c << 4) | n.d;
    
    /* Bit-field in loop */
    for (int i = 0; i < 4; i++) {
        n.a = i;
        result += n.a;
    }
}

/* ==================== Partial register patterns (STRICT_LOW_PART) ==================== */

NOOPT void test_partial_registers(void) {
    /* Operations on sub-word types */
    VOLATILE_VAR char c1 = 10, c2 = 20;
    VOLATILE_VAR short s1 = 100, s2 = 200;
    VOLATILE_VAR int i1 = 1000;
    
    /* Partial register assignments */
    c1 = c2 + 5;              /* char operation */
    s1 = s2 - c1;             /* mixed size */
    c2 = (char)(i1 >> 4);     /* truncation to char */
    
    /* Arithmetic with partial results */
    s1 = (short)(s1 * 2 + c1);
    
    /* Complex expression with partial types */
    VOLATILE_VAR int result = 0;
    result = (int)c1 + (int)s1 * 2;
    c1 = (char)(result & 0xFF);
    
    /* Pointer to partial type */
    VOLATILE_VAR char *pc = &c1;
    *pc = *pc + 1;
}

NOOPT void test_mixed_size_ops(void) {
    /* Mixed-size operations that may generate STRICT_LOW_PART */
    VOLATILE_VAR uint16_t u16 = 0x1234;
    VOLATILE_VAR uint32_t u32 = 0xABCD1234;
    VOLATILE_VAR uint8_t u8 = 0x42;
    
    /* Operations that truncate */
    u16 = (uint16_t)(u32 >> 16);
    u8 = (uint8_t)(u16 + u8);
    
    /* Store partial results */
    u32 = (u32 & 0xFFFF0000) | u16;
    
    /* Byte manipulation */
    VOLATILE_VAR char buffer[4] = {1, 2, 3, 4};
    buffer[0] = buffer[1] + buffer[2];
}

/* ==================== Sub-register patterns (SUBREG) ==================== */

NOOPT void test_vector_subreg(void) {
    /* GCC vector extension for SUBREG patterns */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v8hi v3 = {1, 2, 3, 4, 5, 6, 7, 8};
    
    /* Vector operations that generate SUBREG */
    v4si vsum = v1 + v2;
    VOLATILE_VAR int element = vsum[2];  /* SUBREG access */
    
    /* Cross-type vector access */
    VOLATILE_VAR short half = v3[3];
    
    /* Vector element manipulation */
    for (int i = 0; i < 4; i++) {
        v1[i] = v2[i] * 2;
    }
    
    /* Mixed vector operations */
    v8hi v4;
    for (int i = 0; i < 8; i++) {
        v4[i] = (short)(v1[i/2] + i);
    }
}

NOOPT void test_union_subreg(void) {
    /* Union for type-punning, often generates SUBREG */
    union Punned {
        VOLATILE_VAR uint32_t full;
        struct {
            VOLATILE_VAR uint16_t low;
            VOLATILE_VAR uint16_t high;
        } parts;
        VOLATILE_VAR uint8_t bytes[4];
    };
    
    union Punned u;
    u.full = 0xDEADBEEF;
    
    /* Access different views of same data */
    VOLATILE_VAR uint16_t low_part = u.parts.low;
    VOLATILE_VAR uint16_t high_part = u.parts.high;
    VOLATILE_VAR uint8_t first_byte = u.bytes[0];
    
    /* Modify through one view, read through another */
    u.parts.low = 0x1234;
    VOLATILE_VAR uint8_t modified_byte = u.bytes[0];
    
    /* Mixed-size operations on union members */
    u.full = (u.parts.high << 16) | u.parts.low;
}

NOOPT void test_float_subreg(void) {
    /* Float/integer conversions often use SUBREG */
    VOLATILE_VAR float f1 = 3.14f, f2 = 2.71f;
    VOLATILE_VAR int i1, i2;
    VOLATILE_VAR double d1;
    
    /* Type conversions */
    i1 = (int)f1;              /* float to int */
    f2 = (float)i1;            /* int to float */
    
    /* Bit-level manipulation */
    i2 = *(VOLATILE_VAR int*)&f1;  /* type punning */
    d1 = (double)f1 + (double)f2;
    
    /* Mixed precision */
    VOLATILE_VAR float result = f1 * 2.0f + (float)i1;
}

/* ==================== Combined patterns ==================== */

NOOPT void test_combined_patterns(void) {
    /* Structure combining bit-fields and regular members */
    struct Combined {
        VOLATILE_VAR unsigned int bf1:3;
        VOLATILE_VAR unsigned int bf2:5;
        VOLATILE_VAR short partial;
        VOLATILE_VAR int full;
    };
    
    struct Combined c = {0};
    VOLATILE_VAR int temp;
    
    /* Chain operations through different patterns */
    c.bf1 = 3;
    c.partial = (short)c.bf1 * 100;  /* ZERO_EXTRACT -> STRICT_LOW_PART */
    
    c.bf2 = c.partial & 0x1F;        /* STRICT_LOW_PART -> ZERO_EXTRACT */
    c.full = (c.bf1 << 8) | c.bf2;
    
    /* Access through pointer with cast */
    VOLATILE_VAR char *ptr = (VOLATILE_VAR char*)&c;
    temp = ptr[1] + ptr[2];
    
    /* Union with bit-field */
    union BitUnion {
        struct {
            VOLATILE_VAR unsigned int low:16;
            VOLATILE_VAR unsigned int high:16;
        } bits;
        VOLATILE_VAR uint32_t full;
    };
    
    union BitUnion bu;
    bu.full = 0x12345678;
    bu.bits.low = bu.bits.high & 0xFF;  /* Combined patterns */
}

/* ==================== Architecture-specific patterns ==================== */

#ifdef __i386__
NOOPT void test_x86_specific(void) {
    /* x86-specific patterns that may generate target RTL */
    VOLATILE_VAR uint32_t eax_val, ebx_val;
    VOLATILE_VAR uint16_t ax_val, bx_val;
    VOLATILE_VAR uint8_t al_val, bl_val;
    
    /* Simulate partial register usage patterns */
    eax_val = 0x12345678;
    ax_val = (uint16_t)eax_val;      /* Likely SUBREG */
    al_val = (uint8_t)eax_val;       /* Another SUBREG */
    
    /* Operations that might use STRICT_LOW_PART */
    bx_val = ax_val + 0x100;
    bl_val = al_val * 2;
    
    /* Recombine partial registers */
    eax_val = (eax_val & 0xFFFF0000) | ax_val;
    
    /* Inline assembly for explicit partial register ops */
    __asm__ volatile (
        "movw %1, %%ax\n\t"
        "addw $1, %%ax\n\t"
        "movw %%ax, %0"
        : "=r" (ax_val)
        : "r" (bx_val)
        : "%ax"
    );
}
#endif

#ifdef __arm__
NOOPT void test_arm_specific(void) {
    /* ARM may have different patterns for partial registers */
    VOLATILE_VAR uint32_t word;
    VOLATILE_VAR uint16_t halfword;
    VOLATILE_VAR uint8_t byte;
    
    word = 0xA5A5A5A5;
    halfword = (uint16_t)word;       /* SUBREG access */
    byte = (uint8_t)(word >> 16);    /* Another SUBREG */
    
    /* Byte manipulation common in ARM */
    halfword = (halfword << 8) | byte;
    byte = (byte + 1) & 0x7F;        /* Potential STRICT_LOW_PART */
}
#endif

/* ==================== Builtin functions ==================== */

NOOPT void test_builtins(void) {
    /* GCC builtins that may generate interesting RTL */
    VOLATILE_VAR unsigned int x = 0x12345678;
    VOLATILE_VAR int count;
    
    /* Bit manipulation builtins */
    count = __builtin_popcount(x);           /* May use bit extraction */
    count = __builtin_ctz(x);                /* Count trailing zeros */
    count = __builtin_parity(x);             /* Parity calculation */
    
    /* Bit field builtins */
    unsigned int y = __builtin_bf_insert(0, x, 4, 8, 16);  /* If available */
    unsigned int z = __builtin_bf_extract(x, 8, 4);
    
    /* Prevent optimization */
    VOLATILE_VAR int result = count + y + z;
}

/* ==================== Main test driver ==================== */

/* Array of test functions */
typedef void (*test_func_t)(void);

static test_func_t test_functions[] = {
    test_bitfields,
    test_nested_bitfields,
    test_partial_registers,
    test_mixed_size_ops,
    test_vector_subreg,
    test_union_subreg,
    test_float_subreg,
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
    VOLATILE_VAR int test_to_run = 0;
    
    /* Use command line or computation to determine which tests to run */
    if (argc > 1) {
        test_to_run = argv[1][0] - '0';
    } else {
        /* Compute based on something non-constant */
        test_to_run = (argc + (int)argv[0][0]) % 10;
    }
    
    /* Run selected test to ensure code generation */
    int num_tests = sizeof(test_functions)/sizeof(test_functions[0]) - 1;
    if (test_to_run >= 0 && test_to_run < num_tests && test_functions[test_to_run]) {
        test_functions[test_to_run]();
    } else {
        /* Run all tests if no specific test selected */
        for (int i = 0; test_functions[i] != NULL; i++) {
            test_functions[i]();
        }
    }
    
    /* Final computation to ensure program does something */
    VOLATILE_VAR int final_result = 0;
    for (int i = 0; i < argc; i++) {
        for (char *p = argv[i]; *p; p++) {
            final_result += *p;
        }
    }
    
    return final_result & 0xFF;  /* Return non-zero but bounded value */
}
