/* Test program to trigger specific RTL patterns in GCC's resource.cc:
 * - ZERO_EXTRACT for bit-field operations
 * - STRICT_LOW_PART for partial register updates  
 * - SUBREG for sub-register accesses
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimizations from removing our test code */
static volatile int control = 0;

/* ========== BIT-FIELD TESTS (ZERO_EXTRACT) ========== */

struct bitfield_struct {
    volatile unsigned int flag:1;
    volatile unsigned int value:10;
    unsigned int padding:21;
    volatile unsigned int mode:4;
    unsigned int :0; /* force alignment */
};

struct nested_bitfields {
    struct {
        volatile unsigned int a:3;
        volatile unsigned int b:5;
    } inner;
    volatile unsigned int c:8;
};

void test_bitfields(void) {
    struct bitfield_struct s1 = {0};
    struct nested_bitfields s2 = {{0}, 0};
    
    /* Various bit-field assignments that should generate ZERO_EXTRACT */
    s1.flag = 1;
    s1.value = 511; /* Max 10-bit value */
    s1.mode = 7;
    
    s2.inner.a = 5;
    s2.inner.b = 20;
    s2.c = s2.inner.a + s2.inner.b; /* Read and write bit-fields */
    
    /* Cross assignments between bit-fields */
    unsigned int temp = s1.value;
    s1.value = s2.c;
    s2.c = temp & 0xFF;
    
    /* Complex expression with bit-fields */
    s1.flag = (s1.value > 100) && (s2.inner.b < 10);
}

/* ========== PARTIAL REGISTER TESTS (STRICT_LOW_PART) ========== */

void test_partial_registers(void) {
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    volatile int vi1, vi2;
    
    /* Assignments to sub-word types */
    vi1 = 1000;
    vs1 = (short)vi1 + 50;           /* Should generate partial register update */
    vc1 = (char)(vs1 * 2);           /* Even smaller partial update */
    
    /* Arithmetic on partial registers */
    vs2 = vs1 * 2;
    vc2 = vc1 + 10;
    
    /* Store back to int through partial register */
    vi2 = vs2;
    vi2 = vc2;
    
    /* Mixed-size operations */
    vs1 = (short)((vi1 & 0xFFFF) + (vi2 & 0xFFFF));
    
    /* Loop with partial register updates (prevents optimization) */
    for (volatile char i = 0; i < 10; i++) {
        vc1 = i * 2;
        vs1 = vc1 * 100;
    }
}

/* ========== SUBREG TESTS ========== */

/* Packed structure for sub-register access */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    short c;
};

/* Union for type-punning */
union type_pun {
    float f;
    int i;
    short s[2];
};

/* GCC vector extension */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

void test_subregs(void) {
    /* Packed structure access */
    struct packed_struct ps = {1, 1000, 500};
    volatile int x1 = ps.b;  /* May require unaligned access -> SUBREG */
    volatile short x2 = ps.c;
    
    /* Type-punning through union */
    union type_pun up;
    up.f = 3.14f;
    volatile int xi = up.i;      /* Float bits as int */
    volatile short xs = up.s[0]; /* Part of int as short */
    
    /* Vector operations */
    v4si v1 = {1, 2, 3, 4};
    v8hi v2 = {10, 20, 30, 40, 50, 60, 70, 80};
    
    volatile int elem1 = v1[2];     /* Vector element extraction */
    volatile short elem2 = v2[5];
    
    /* Vector store to scalar */
    v1[0] = control;
    v2[3] = elem2;
    
    /* Mixed vector/scalar operations */
    for (int i = 0; i < 4; i++) {
        v1[i] = v1[i] * 2 + i;
    }
}

/* ========== COMBINED PATTERN TESTS ========== */

struct combined {
    volatile unsigned int bits:12;
    volatile short half;
    volatile char byte;
};

void test_combined_patterns(void) {
    struct combined c = {0};
    
    /* Bit-field to partial register */
    c.bits = 2047;
    c.half = (short)c.bits;      /* ZERO_EXTRACT -> STRICT_LOW_PART/SUBREG */
    
    /* Partial register to bit-field */
    c.byte = 127;
    c.bits = c.byte * 2;         /* SUBREG -> ZERO_EXTRACT */
    
    /* Complex nested expression */
    volatile int temp = 0;
    for (int i = 0; i < 8; i++) {
        c.bits = (c.bits << 1) | ((c.byte >> i) & 1);
        c.half = c.half + (short)c.bits;
        temp += c.half;
    }
    
    /* Union with bit-fields */
    union {
        struct {
            volatile unsigned int low:16;
            volatile unsigned int high:16;
        } bits;
        volatile int full;
    } u;
    
    u.full = 0x12345678;
    c.half = u.bits.low;         /* Bit-field extract to partial register */
    u.bits.high = c.byte;        /* Partial register to bit-field store */
}

/* ========== ARCHITECTURE-SPECIFIC TESTS ========== */

#ifdef __i386__
void test_x86_specific(void) {
    volatile int result;
    
    /* Inline assembly that uses partial registers */
    __asm__ volatile (
        "movw $0x1234, %%ax\n\t"
        "movb $0x56, %%ah\n\t"
        "movw %%ax, %0"
        : "=r" (result)
        :
        : "%ax"
    );
    
    /* Bit test and set */
    volatile unsigned int value = 0xF0F0F0F0;
    volatile unsigned char bit;
    
    __asm__ volatile (
        "btl $5, %1\n\t"
        "setc %0"
        : "=r" (bit)
        : "r" (value)
        : "cc"
    );
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    volatile int val = 0x12345678;
    volatile short half_val;
    
    /* ARM has specific instructions for byte/halfword access */
    __asm__ volatile (
        "ldrh %0, [%1]"
        : "=r" (half_val)
        : "r" (&val)
        : "memory"
    );
}
#endif

/* ========== BUILTIN FUNCTION TESTS ========== */

void test_builtins(void) {
    volatile unsigned int x = 0x12345678;
    volatile int count, parity;
    
    /* Builtins that may involve bit extraction */
    count = __builtin_popcount(x);      /* Population count */
    parity = __builtin_parity(x);       /* Parity */
    
    /* Bit field extraction builtin */
    volatile unsigned int extracted = __builtin_ibit_extract(x, 8, 4);
    
    /* Byte swap - may involve SUBREG patterns */
    volatile unsigned int swapped = __builtin_bswap32(x);
    
    /* Count leading zeros */
    volatile int lz = __builtin_clz(x);
}

/* ========== MAIN TEST DRIVER ========== */

typedef void (*test_func_t)(void);

const test_func_t test_functions[] = {
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
};

int main(int argc, char *argv[]) {
    int num_tests = sizeof(test_functions) / sizeof(test_functions[0]);
    
    /* Use command line or control variable to select tests */
    int start_test = 0;
    int end_test = num_tests;
    
    if (argc > 1) {
        start_test = atoi(argv[1]) % num_tests;
        if (argc > 2) {
            end_test = atoi(argv[2]) % (num_tests + 1);
            if (end_test <= start_test) end_test = start_test + 1;
        }
    }
    
    /* Run selected tests */
    for (int i = start_test; i < end_test && i < num_tests; i++) {
        control = i;  /* Prevent optimization */
        test_functions[i]();
    }
    
    /* Ensure program produces observable output */
    printf("Tests completed. Control = %d\n", control);
    
    /* Simple computation to ensure program runs */
    volatile int result = 0;
    for (int i = 0; i < 100; i++) {
        result += i * control;
    }
    
    return result > 0 ? 0 : 1;
}
