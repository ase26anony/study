/* test_resources.c - Program to trigger uncovered RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_int = 0;
volatile short g_volatile_short = 0;
volatile char g_volatile_char = 0;

/* ==================== BIT-FIELD PATTERNS (ZERO_EXTRACT) ==================== */

struct BitFieldStruct {
    volatile unsigned int flag:1;
    volatile unsigned int value:10;
    volatile unsigned int mode:3;
    unsigned int padding:18;
};

struct NestedBitField {
    struct {
        volatile unsigned int a:4;
        volatile unsigned int b:4;
    } nibbles;
    volatile unsigned int full:8;
};

void test_bitfields(void) {
    struct BitFieldStruct s1 = {0};
    struct BitFieldStruct s2 = {0};
    struct NestedBitField nbf = {0};
    
    /* Basic bit-field assignments - should generate ZERO_EXTRACT */
    s1.flag = 1;
    s1.value = 511;  /* Max for 10 bits */
    s1.mode = 3;
    
    /* Cross-structure bit-field assignment */
    s2.value = s1.value;
    s2.flag = s1.flag ^ 1;
    
    /* Bit-field in conditional */
    if (s1.flag) {
        s1.mode = (s1.mode + 1) & 0x7;
    }
    
    /* Nested bit-field access */
    nbf.nibbles.a = 5;
    nbf.nibbles.b = 10;
    nbf.full = (nbf.nibbles.a << 4) | nbf.nibbles.b;
    
    /* Complex expression with bit-fields */
    g_volatile_int = (s1.value << 3) | s1.mode;
    
    /* Loop with bit-field updates */
    for (int i = 0; i < 3; i++) {
        s1.value = (s1.value + i) & 0x3FF;  /* Keep within 10 bits */
    }
}

/* ==================== PARTIAL REGISTER PATTERNS (STRICT_LOW_PART) ==================== */

void test_partial_registers(void) {
    volatile short vs = 0;
    volatile char vc = 0;
    int temp;
    
    /* Cast to smaller types - may generate STRICT_LOW_PART */
    vs = (short)(g_volatile_int + 100);
    vc = (char)(vs * 2);
    
    /* Arithmetic on sub-word types */
    g_volatile_short = g_volatile_short + 50;
    g_volatile_char = g_volatile_char - 25;
    
    /* Mixed-type expressions */
    temp = g_volatile_int;
    vs = (short)(temp & 0xFFFF);
    vc = (char)(temp & 0xFF);
    
    /* Partial updates in loops */
    for (int i = 0; i < 5; i++) {
        vs = (short)(vs + i);
        vc = (char)(vc - i);
    }
    
    /* Pointer to partial type */
    volatile short *ps = &vs;
    *ps = *ps + 100;
    
    volatile char *pc = &vc;
    *pc = *pc - 50;
}

/* ==================== SUB-REGISTER PATTERNS (SUBREG) ==================== */

/* Vector type using GCC extension */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

/* Packed structure for SUBREG generation */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

union TypePun {
    float f;
    int i;
    char bytes[4];
};

void test_subregisters(void) {
    /* Vector operations - should generate SUBREG */
    v4si vec_int = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Vector element access */
    int element = vec_int[2];
    g_volatile_int = element;
    
    /* Vector-scalar operations */
    vec_int[0] = g_volatile_int;
    vec_short[3] = g_volatile_short;
    
    /* Packed structure access */
    struct PackedStruct ps = {0};
    ps.b = g_volatile_int;  /* Misaligned access may use SUBREG */
    ps.c = g_volatile_short;
    
    /* Type punning through union */
    union TypePun pun;
    pun.f = 3.14159f;
    g_volatile_int = pun.i;  /* Bit pattern reinterpretation */
    
    /* Access individual bytes */
    pun.bytes[1] = g_volatile_char;
    
    /* Float/double conversions */
    float f = (float)g_volatile_int;
    double d = (double)f;
    int i = (int)d;
    g_volatile_int = i;
}

/* ==================== COMBINED PATTERNS ==================== */

struct Combined {
    volatile unsigned int low:8;
    volatile unsigned int high:8;
    volatile short full_short;
};

void test_combined_patterns(void) {
    struct Combined c = {0};
    
    /* Bit-field to partial register */
    c.low = 0xAB;
    c.high = 0xCD;
    g_volatile_short = (c.high << 8) | c.low;  /* ZERO_EXTRACT + SUBREG/STRICT_LOW_PART */
    
    /* Partial register to bit-field */
    c.full_short = g_volatile_short;
    c.low = c.full_short & 0xFF;
    c.high = (c.full_short >> 8) & 0xFF;
    
    /* Complex nested expression */
    g_volatile_char = (char)((c.low + c.high) & 0x7F);
    
    /* Loop with combined patterns */
    for (int i = 0; i < 4; i++) {
        c.low = (c.low + i) & 0xFF;
        c.full_short = (c.full_short << 1) | (c.low & 1);
        g_volatile_short = c.full_short;
    }
}

/* ==================== ARCHITECTURE-SPECIFIC PATTERNS ==================== */

#ifdef __i386__
void test_x86_specific(void) {
    int result;
    
    /* Inline assembly that might generate partial register ops */
    __asm__ volatile (
        "movw %1, %%ax\n\t"
        "addw $100, %%ax\n\t"
        "movw %%ax, %0"
        : "=r" (g_volatile_short)
        : "r" (g_volatile_short)
        : "ax"
    );
    
    /* Bit test and set */
    __asm__ volatile (
        "btsl $5, %0"
        : "+r" (g_volatile_int)
        :
        : "cc"
    );
}
#elif defined(__arm__)
void test_arm_specific(void) {
    /* ARM-specific patterns */
    unsigned int val = g_volatile_int;
    
    /* Use ARM bit-field instructions if available */
    __asm__ volatile (
        "bfi %0, %1, #4, #8"
        : "+r" (val)
        : "r" (g_volatile_short)
    );
    
    g_volatile_int = val;
}
#else
void test_x86_specific(void) {
    /* Generic fallback */
    g_volatile_short = (g_volatile_short + 100) & 0xFFFF;
}
void test_arm_specific(void) {
    /* Generic fallback */
    g_volatile_int = (g_volatile_int << 4) | (g_volatile_int >> 28);
}
#endif

/* ==================== BUILTIN FUNCTIONS ==================== */

void test_builtins(void) {
    unsigned int x = g_volatile_int;
    
    /* Builtins that may involve bit manipulation */
    int leading_zeros = __builtin_clz(x);
    int trailing_zeros = __builtin_ctz(x);
    int parity = __builtin_parity(x);
    int popcount = __builtin_popcount(x);
    
    g_volatile_int = (leading_zeros << 24) | 
                     (trailing_zeros << 16) | 
                     (parity << 8) | 
                     popcount;
    
    /* Bit extraction builtin */
    unsigned int extracted = __builtin_bitfield_extract_u32(x, 4, 8);
    g_volatile_int = extracted;
}

/* ==================== MAIN TEST DRIVER ==================== */

typedef void (*test_func_t)(void);

const test_func_t test_functions[] = {
    test_bitfields,
    test_partial_registers,
    test_subregisters,
    test_combined_patterns,
    test_x86_specific,
    test_arm_specific,
    test_builtins
};

const char* test_names[] = {
    "Bit-fields (ZERO_EXTRACT)",
    "Partial registers (STRICT_LOW_PART)",
    "Sub-registers (SUBREG)",
    "Combined patterns",
    "x86-specific patterns",
    "ARM-specific patterns",
    "Builtin functions"
};

#define NUM_TESTS (sizeof(test_functions)/sizeof(test_functions[0]))

int main(int argc, char *argv[]) {
    int test_to_run = 0;
    
    /* Use command line or volatile to control test selection */
    if (argc > 1) {
        test_to_run = atoi(argv[1]) % NUM_TESTS;
    } else {
        test_to_run = g_volatile_int % NUM_TESTS;
    }
    
    printf("Running test: %s\n", test_names[test_to_run]);
    
    /* Run selected test */
    test_functions[test_to_run]();
    
    /* Run all tests in a loop to ensure code generation */
    for (unsigned int i = 0; i < NUM_TESTS; i++) {
        if (i != test_to_run) {
            /* Call through volatile pointer to prevent dead code elimination */
            volatile test_func_t fp = test_functions[i];
            if (g_volatile_int & (1 << i)) {
                fp();
            }
        }
    }
    
    /* Final computation to ensure program does something */
    int result = g_volatile_int + g_volatile_short + g_volatile_char;
    printf("Final result: %d\n", result);
    
    return result != 0;
}
