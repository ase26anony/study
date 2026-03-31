/* Test program to trigger specific RTL patterns in GCC's resource.cc:
 * - ZERO_EXTRACT (bit-field operations)
 * - STRICT_LOW_PART (partial register updates)
 * - SUBREG (sub-register accesses)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimizations from removing our test code */
static volatile int control = 0;

/* ========== ZERO_EXTRACT patterns (bit-fields) ========== */

struct bitfield_struct {
    volatile unsigned int flag:1;
    unsigned int value:10;
    unsigned int pad:21;
    volatile unsigned int another:4;
};

struct nested_bitfield {
    struct {
        volatile unsigned int low:8;
        unsigned int high:8;
    } parts;
    unsigned int full;
};

void test_zero_extract(void) {
    struct bitfield_struct s1 = {0};
    struct bitfield_struct s2 = {0};
    struct nested_bitfield n = {0};
    
    /* Basic bit-field assignments */
    s1.flag = 1;
    s1.value = 511;  /* Max 10-bit value */
    s1.another = 7;
    
    /* Cross assignments between bit-fields */
    s2.value = s1.value;
    s2.flag = s1.flag ^ 1;
    
    /* Complex expression with bit-fields */
    unsigned int temp = s1.value + s2.value;
    s1.another = temp & 0xF;
    
    /* Nested structure bit-field access */
    n.parts.low = 0xAB;
    n.parts.high = 0xCD;
    n.full = (n.parts.high << 8) | n.parts.low;
    
    /* Bit-field in conditional */
    if (s1.flag && s2.value > 100) {
        s1.another = 3;
    }
    
    /* Loop with bit-field modification */
    for (int i = 0; i < 4; i++) {
        s1.value += i;
    }
}

/* ========== STRICT_LOW_PART patterns (partial register updates) ========== */

void test_strict_low_part(void) {
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    int i, j;
    
    /* Assignments to sub-word types */
    i = 1000;
    vs1 = (short)i + 123;  /* Should generate partial register update */
    vs2 = vs1 * 2;
    
    j = 50000;
    vc1 = (char)(j & 0xFF);  /* Explicit truncation */
    vc2 = vc1 + 32;
    
    /* Arithmetic on sub-word types */
    vs1 = vs1 + vs2 - 100;
    vc1 = vc1 * 2;
    
    /* Mixed-size operations */
    vs1 = (short)(vc1 * vc2);
    
    /* Pointer to sub-word type */
    volatile short *ps = &vs1;
    *ps = 0x1234;
    
    /* Array of sub-word types */
    volatile char arr[8];
    for (int k = 0; k < 8; k++) {
        arr[k] = k * 10;
    }
    
    /* Conditional partial update */
    if (control) {
        vs1 = 999;
    } else {
        vs2 = 888;
    }
}

/* ========== SUBREG patterns (sub-register accesses) ========== */

/* Vector type using GCC extension */
typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));

union mixed_types {
    unsigned int full;
    struct {
        unsigned short low;
        unsigned short high;
    } halves;
    unsigned char bytes[4];
};

void test_subreg(void) {
    /* Vector operations */
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3;
    
    /* Vector element access - should generate SUBREG */
    int elem = v1[2];
    v1[0] = elem + 10;
    
    /* Vector arithmetic */
    v3 = v1 + v2;
    elem = v3[1];
    
    /* Mixed vector types */
    v8hi vh1 = {1, 2, 3, 4, 5, 6, 7, 8};
    short selem = vh1[3];
    vh1[4] = selem * 2;
    
    /* Union type-punning */
    union mixed_types u;
    u.full = 0x12345678;
    
    /* Access different views of the same data */
    unsigned short low_half = u.halves.low;   /* Should be 0x5678 on LE */
    unsigned char first_byte = u.bytes[0];    /* Should be 0x78 on LE */
    
    u.halves.high = 0xABCD;
    u.bytes[2] = 0xEF;
    
    /* Float/int conversions (architecture dependent) */
    float f = 3.14159f;
    unsigned int *fp = (unsigned int*)&f;
    unsigned int fbits = *fp;  /* Type punning through pointer */
    
    /* Double word access */
    long long ll = 0x1122334455667788LL;
    int *llp = (int*)&ll;
    int low_word = llp[0];  /* SUBREG access to part of long long */
}

/* ========== Combined patterns ========== */

struct combined {
    volatile unsigned int bits:5;
    volatile short partial;
    int full;
};

void test_combined(void) {
    struct combined c = {0};
    
    /* Bit-field to partial register */
    c.bits = 0x1F;  /* Max 5-bit value */
    c.partial = (short)c.bits;  /* ZERO_EXTRACT -> SUBREG/STRICT_LOW_PART */
    
    /* Partial register to bit-field */
    c.partial = 0x1234;
    c.bits = c.partial & 0x1F;  /* Multiple conversions */
    
    /* Complex expression combining patterns */
    c.full = (c.bits << 16) | (c.partial & 0xFFFF);
    
    /* Array of structs with mixed accesses */
    struct combined arr[4];
    for (int i = 0; i < 4; i++) {
        arr[i].bits = i;
        arr[i].partial = i * 100;
        arr[i].full = arr[i].bits + arr[i].partial;
    }
    
    /* Pointer arithmetic with mixed types */
    volatile short *ps = &c.partial;
    *ps = (*ps + 1) & 0x7FFF;
}

/* ========== Architecture-specific patterns ========== */

#ifdef __i386__
void test_i386_specific(void) {
    /* x86-specific inline assembly for partial registers */
    unsigned int eax_val;
    unsigned short ax_val;
    unsigned char al_val;
    
    /* Simulate partial register updates */
    asm volatile (
        "movl $0x12345678, %%eax\n\t"
        "movw $0xABCD, %%ax\n\t"
        "movb $0xEF, %%al\n\t"
        : "=a" (eax_val)
        :
        : "eax"
    );
    
    /* Separate outputs to force different partial regs */
    asm volatile (
        "movw %1, %%ax\n\t"
        "movw %%ax, %0\n\t"
        : "=r" (ax_val)
        : "r" ((unsigned short)0x1234)
        : "ax"
    );
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    /* ARM-specific patterns */
    volatile unsigned int reg;
    volatile unsigned short half;
    
    /* Use builtins that might generate interesting patterns */
    reg = __builtin_clz(0x80000000);
    half = (unsigned short)__builtin_parity(reg);
}
#endif

/* ========== Main test driver ========== */

typedef void (*test_func_t)(void);

const test_func_t test_functions[] = {
    test_zero_extract,
    test_strict_low_part,
    test_subreg,
    test_combined,
#ifdef __i386__
    test_i386_specific,
#endif
#ifdef __arm__
    test_arm_specific,
#endif
    NULL
};

int main(int argc, char *argv[]) {
    int num_tests = 0;
    
    /* Count available tests */
    while (test_functions[num_tests] != NULL) {
        num_tests++;
    }
    
    /* Run tests based on control input */
    int start_test = 0;
    if (argc > 1) {
        start_test = atoi(argv[1]) % num_tests;
    }
    
    /* Run a sequence of tests to ensure code generation */
    for (int i = start_test; i < num_tests; i++) {
        if (control || (i % 2 == 0)) {
            test_functions[i]();
        }
    }
    
    /* Also run all tests in reverse to increase coverage */
    for (int i = num_tests - 1; i >= 0; i--) {
        if (!control || (i % 3 == 0)) {
            test_functions[i]();
        }
    }
    
    /* Final computation to ensure program does something useful */
    int result = 0;
    for (int i = 0; i < 100; i++) {
        result += i * (control + 1);
    }
    
    printf("Test completed with result: %d\n", result);
    return result == 4950 ? 0 : 1;  /* 0+1+2+...+99 = 4950 */
}
