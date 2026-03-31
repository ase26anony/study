/* Test program to trigger uncovered RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_int = 0;
volatile short g_volatile_short = 0;
volatile char g_volatile_char = 0;

/* ==================== ZERO_EXTRACT patterns (bit-fields) ==================== */

struct BitFieldStruct {
    volatile unsigned int flag:1;
    volatile unsigned int value:10;
    volatile unsigned int mode:3;
    unsigned int padding:18;
};

struct NestedBitField {
    struct {
        volatile unsigned int low:4;
        volatile unsigned int high:4;
    } nibbles;
    volatile unsigned int full:16;
};

void test_zero_extract(void) {
    struct BitFieldStruct s1 = {0};
    struct BitFieldStruct s2 = {0};
    
    /* Basic bit-field assignments */
    s1.flag = 1;
    s1.value = 511;  /* Max for 10 bits */
    s1.mode = 3;
    
    /* Bit-field to bit-field assignment */
    s2.value = s1.value;
    s2.flag = s1.flag;
    
    /* Bit-field in expression */
    unsigned int temp = s1.value + s2.value;
    g_volatile_int = temp;
    
    /* Complex bit-field expression */
    s1.mode = (s1.value >> 5) & 0x7;
    
    /* Nested bit-field structure */
    struct NestedBitField nbf = {0};
    nbf.nibbles.low = 0xA;
    nbf.nibbles.high = 0xB;
    nbf.full = (nbf.nibbles.high << 4) | nbf.nibbles.low;
    
    /* Bit-field in loop */
    for (int i = 0; i < 4; i++) {
        s1.value = (s1.value << 1) | s1.flag;
    }
    
    /* Bit-field with conditional */
    s2.flag = (s1.value > 256) ? 1 : 0;
}

/* ==================== STRICT_LOW_PART patterns (partial registers) ==================== */

void test_strict_low_part(void) {
    volatile short vs = 0;
    volatile char vc = 0;
    volatile int vi = 0;
    
    /* Cast to smaller types */
    vi = 0x12345678;
    vs = (short)vi + 100;
    vc = (char)(vi >> 16) - 50;
    
    /* Arithmetic on sub-word types */
    short s1 = 1000, s2 = 2000;
    vs = s1 + s2;
    vs = vs * 2;
    vs = vs / 3;
    
    char c1 = 100, c2 = 50;
    vc = c1 - c2;
    vc = vc * 2;
    
    /* Partial register update in expression */
    vi = (vs << 16) | (vc & 0xFF);
    
    /* Mixed-size operations */
    int result = vs + vc;
    g_volatile_int = result;
    
    /* Loop with partial register updates */
    for (char i = 0; i < 10; i++) {
        vc = vc + i;
    }
    
    /* Conditional partial update */
    if (vi > 0) {
        vs = (short)(vi & 0xFFFF);
    } else {
        vs = (short)(-vi & 0xFFFF);
    }
}

/* ==================== SUBREG patterns (sub-register accesses) ==================== */

/* Vector extension for SUBREG patterns */
typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));

/* Packed structure for SUBREG */
struct PackedStruct {
    char a;
    short b;
    int c;
} __attribute__((packed));

union TypePun {
    int i;
    float f;
    short s[2];
};

void test_subreg(void) {
    /* Vector operations */
    v4si vec_int = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Vector element access (likely generates SUBREG) */
    int elem = vec_int[2];
    vec_int[1] = elem * 2;
    
    short selem = vec_short[3];
    vec_short[4] = selem + 100;
    
    /* Vector to scalar conversion */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += vec_int[i];
    }
    g_volatile_int = sum;
    
    /* Packed structure access */
    struct PackedStruct ps;
    ps.a = 0xAA;
    ps.b = 0xBBBB;
    ps.c = 0xCCCCCCCC;
    
    /* Accessing misaligned members (may generate SUBREG) */
    short b_val = ps.b;
    int c_val = ps.c;
    
    /* Type punning through union */
    union TypePun pun;
    pun.i = 0x3F800000;  /* 1.0f in float */
    float f = pun.f;
    short s0 = pun.s[0];
    short s1 = pun.s[1];
    
    /* Mixed float/int operations */
    float f1 = 3.14f;
    int if1 = *(int*)&f1;  /* Type punning */
    float f2 = *(float*)&if1;
    
    /* Array of different types */
    char buffer[16];
    *(int*)(buffer + 1) = 0xDEADBEEF;  /* Misaligned store */
    int read_back = *(int*)(buffer + 1);
}

/* ==================== Combined patterns ==================== */

void test_combined_patterns(void) {
    /* Structure with bit-fields */
    struct {
        volatile unsigned int low:8;
        volatile unsigned int high:8;
    } bf;
    
    /* Bit-field to partial register */
    bf.low = 0xAA;
    bf.high = 0xBB;
    
    volatile short vs;
    vs = (short)((bf.high << 8) | bf.low);  /* ZERO_EXTRACT + SUBREG/STRICT_LOW_PART */
    
    /* Union with bit-field and full integer */
    union {
        struct {
            unsigned int a:4;
            unsigned int b:4;
            unsigned int c:4;
            unsigned int d:4;
        } bits;
        unsigned short full;
    } u;
    
    u.bits.a = 0xF;
    u.bits.b = 0xE;
    u.bits.c = 0xD;
    u.bits.d = 0xC;
    
    /* Access through both views */
    unsigned short temp = u.full;
    u.bits.a = temp & 0xF;
    
    /* Complex expression with all patterns */
    struct BitFieldStruct s = {0};
    s.value = 500;
    s.flag = 1;
    
    v4si vec = {0};
    vec[0] = s.value;
    
    volatile char vc = (char)(s.value & 0xFF);  /* ZERO_EXTRACT + partial register */
    
    /* Loop combining patterns */
    for (int i = 0; i < 8; i++) {
        s.value = (s.value >> 1) | (s.flag << 9);
        vs = (short)s.value;  /* Partial register update */
        vec[i % 4] = vs;      /* Vector element store */
    }
}

/* ==================== Architecture-specific patterns ==================== */

#ifdef __i386__
void test_x86_specific(void) {
    /* Inline assembly that might generate partial register updates */
    int a = 100, b = 200;
    int result;
    
    /* Assembly with byte register output */
    __asm__ volatile (
        "addl %1, %0\n"
        "movb %%al, %2\n"
        : "+r"(a)
        : "r"(b), "m"(g_volatile_char)
        : "cc"
    );
    
    /* Bit test and set */
    __asm__ volatile (
        "btsl $5, %0\n"
        : "+m"(g_volatile_int)
        :
        : "cc"
    );
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    /* ARM-specific patterns */
    int val = 0x12345678;
    short half;
    
    /* ARM has specific instructions for byte/halfword access */
    __asm__ volatile (
        "strh %1, [%0]\n"
        : 
        : "r"(&g_volatile_short), "r"(val)
        : "memory"
    );
}
#endif

/* ==================== Builtin functions ==================== */

void test_builtins(void) {
    unsigned int x = 0x12345678;
    
    /* Builtins that may involve bit manipulation */
    int leading_zeros = __builtin_clz(x);
    int trailing_zeros = __builtin_ctz(x);
    int parity = __builtin_parity(x);
    int popcount = __builtin_popcount(x);
    
    /* Bit extraction builtin */
    unsigned int extracted = __builtin_extract_bits(x, 0x0F00);  /* Extract bits 8-11 */
    
    g_volatile_int = leading_zeros + trailing_zeros + parity + popcount + extracted;
}

/* ==================== Main test driver ==================== */

typedef void (*test_func_t)(void);

static test_func_t test_functions[] = {
    test_zero_extract,
    test_strict_low_part,
    test_subreg,
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
    volatile int test_selector = 0;
    
    /* Use command line or environment to select tests */
    if (argc > 1) {
        test_selector = atoi(argv[1]);
    }
    
    /* Run all tests or selected test */
    if (test_selector == 0) {
        /* Run all tests */
        for (int i = 0; test_functions[i] != NULL; i++) {
            test_functions[i]();
        }
    } else {
        /* Run specific test */
        int idx = test_selector - 1;
        if (idx >= 0 && test_functions[idx] != NULL) {
            test_functions[idx]();
        }
    }
    
    /* Final computation to ensure program does something */
    int final_result = g_volatile_int + g_volatile_short + g_volatile_char;
    
    printf("Test completed. Result: %d\n", final_result);
    
    /* Use result to prevent dead code elimination */
    if (final_result > 1000000) {
        printf("Unexpected large result!\n");
    }
    
    return final_result & 0xFF;  /* Return non-zero to indicate execution */
}
