/* Test program to trigger uncovered RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_int = 0;
volatile short g_volatile_short = 0;
volatile char g_volatile_char = 0;

/* ==================== BIT-FIELD TESTS (ZERO_EXTRACT) ==================== */

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

void test_bitfield_zero_extract(void) {
    struct BitFieldStruct s1 = {0};
    struct BitFieldStruct s2 = {0};
    
    /* Direct bit-field assignments - should generate ZERO_EXTRACT */
    s1.flag = 1;
    s1.value = 511;  /* Max 10-bit value */
    s1.mode = 3;
    
    /* Bit-field to bit-field copy */
    s2.flag = s1.flag;
    s2.value = s1.value + 1;
    s2.mode = s1.mode ^ 1;
    
    /* Bit-field to integer extraction */
    int extracted = s1.value;
    g_volatile_int = extracted;
    
    /* Integer to bit-field with masking */
    unsigned int temp = g_volatile_int;
    s1.value = temp & 0x3FF;  /* 10-bit mask */
    
    /* Nested bit-field structure */
    struct NestedBitField nbf;
    nbf.nibbles.a = 5;
    nbf.nibbles.b = 10;
    nbf.full = nbf.nibbles.a + nbf.nibbles.b;
    
    /* Complex bit-field expression */
    s1.value = (s1.value * 2) & 0x3FF;
    s1.flag = (s1.value > 500) ? 1 : 0;
}

/* ==================== PARTIAL REGISTER TESTS (STRICT_LOW_PART) ==================== */

void test_partial_register_strict_low_part(void) {
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    
    /* Direct assignments to partial types */
    vs1 = (short)g_volatile_int;
    vc1 = (char)(g_volatile_int + 1);
    
    /* Arithmetic on partial types */
    vs2 = vs1 + 100;
    vc2 = vc1 * 2;
    
    /* Mixed-size operations */
    int temp_int = g_volatile_int;
    vs1 = (short)(temp_int >> 8);
    vc1 = (char)(temp_int & 0xFF);
    
    /* Store partial results back through pointers */
    short *ps = &vs1;
    *ps = (short)(*ps + 1);
    
    char *pc = &vc1;
    *pc = (char)(*pc - 1);
    
    /* Loop with partial register updates */
    for (volatile short i = 0; i < 10; i = (short)(i + 1)) {
        vs1 = (short)(vs1 + i);
    }
}

/* ==================== SUBREG TESTS ==================== */

/* Vector type for SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

/* Packed structure for SUBREG access */
struct PackedData {
    unsigned char a;
    unsigned short b;
    unsigned int c;
} __attribute__((packed));

union TypePun {
    float f;
    int i;
    short s[2];
};

void test_subreg_patterns(void) {
    /* Vector operations - should generate SUBREG for element access */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;
    
    /* Vector element extraction - likely SUBREG */
    int element = vec3[2];
    g_volatile_int = element;
    
    /* Vector with smaller elements */
    v8hi vec_short = {1, 2, 3, 4, 5, 6, 7, 8};
    vec_short[3] = (short)g_volatile_int;
    
    /* Packed structure access - may generate SUBREG */
    struct PackedData packed;
    packed.a = 0xAA;
    packed.b = 0xBBBB;
    packed.c = 0xCCCCCCCC;
    
    unsigned short b_val = packed.b;  /* Unaligned access */
    g_volatile_short = b_val;
    
    /* Type punning through union */
    union TypePun pun;
    pun.f = 3.14159f;
    pun.i = pun.i + 1;  /* Integer operation on float bits */
    pun.s[0] = (short)(pun.s[0] ^ 0x5555);  /* Partial register update */
    
    /* Float/double conversions that may involve SUBREG */
    float f = (float)g_volatile_int;
    int i = (int)f;
    g_volatile_int = i;
}

/* ==================== COMBINED PATTERN TESTS ==================== */

struct Combined {
    volatile unsigned int low:8;
    volatile unsigned int high:8;
    volatile short middle;
};

void test_combined_patterns(void) {
    struct Combined comb = {0};
    
    /* Bit-field to partial register */
    comb.low = 0x55;
    comb.high = 0xAA;
    g_volatile_short = (short)((comb.high << 8) | comb.low);
    
    /* Partial register to bit-field */
    short temp_short = g_volatile_short;
    comb.low = temp_short & 0xFF;
    comb.high = (temp_short >> 8) & 0xFF;
    
    /* Complex expression mixing patterns */
    comb.middle = (short)((comb.low * 3) + (comb.high * 2));
    
    /* Use vector with bit-field-like extraction */
    v4si vec = {comb.low, comb.high, comb.middle, 0};
    vec[1] = vec[0] + vec[2];  /* SUBREG access and update */
    
    /* Nested operations */
    union {
        struct Combined c;
        int full;
    } u;
    u.c = comb;
    u.full = u.full ^ 0x12345678;  /* Whole register op affecting bit-fields */
}

/* ==================== ARCHITECTURE-SPECIFIC TESTS ==================== */

#ifdef __i386__
void test_x86_specific(void) {
    /* Inline assembly that might generate partial register ops */
    int result;
    short s_result;
    
    __asm__ volatile (
        "movw %1, %%ax\n\t"
        "addw $100, %%ax\n\t"
        "movw %%ax, %0"
        : "=r" (s_result)
        : "r" (g_volatile_short)
        : "ax"
    );
    
    g_volatile_short = s_result;
    
    /* Bit test and set */
    unsigned int value = g_volatile_int;
    __asm__ volatile (
        "btsl $5, %0"
        : "+r" (value)
        :
        : "cc"
    );
    g_volatile_int = value;
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    /* ARM may generate interesting patterns for byte/halfword access */
    volatile int val = g_volatile_int;
    volatile short half = (short)val;
    volatile char byte = (char)(val >> 16);
    
    /* Force partial register loads/stores */
    half = (short)(half + byte);
    g_volatile_short = half;
}
#endif

/* ==================== BUILTIN FUNCTION TESTS ==================== */

void test_builtin_functions(void) {
    unsigned int x = g_volatile_int;
    
    /* Builtins that work with bit fields */
    int leading_zeros = __builtin_clz(x);
    int parity = __builtin_parity(x);
    int popcount = __builtin_popcount(x);
    
    /* Use results to prevent optimization */
    g_volatile_int = leading_zeros + parity + popcount;
    
    /* Bit extraction builtin */
    int extracted_bit = (x >> 5) & 1;
    g_volatile_int = extracted_bit;
}

/* ==================== MAIN TEST DRIVER ==================== */

typedef void (*test_func_t)(void);

struct test_case {
    const char *name;
    test_func_t func;
};

static struct test_case tests[] = {
    {"bitfield_zero_extract", test_bitfield_zero_extract},
    {"partial_register_strict_low_part", test_partial_register_strict_low_part},
    {"subreg_patterns", test_subreg_patterns},
    {"combined_patterns", test_combined_patterns},
    {"builtin_functions", test_builtin_functions},
#ifdef __i386__
    {"x86_specific", test_x86_specific},
#endif
#ifdef __arm__
    {"arm_specific", test_arm_specific},
#endif
};

int main(int argc, char *argv[]) {
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int test_to_run = 0;
    
    /* Use command line or volatile to control test selection */
    if (argc > 1) {
        test_to_run = atoi(argv[1]) % num_tests;
    } else {
        test_to_run = g_volatile_int % num_tests;
    }
    
    printf("Running test: %s\n", tests[test_to_run].name);
    
    /* Run all tests in sequence to maximize RTL pattern generation */
    for (int i = 0; i < num_tests; i++) {
        tests[i].func();
    }
    
    /* Additional loop to ensure complex control flow */
    for (volatile int i = 0; i < 3; i++) {
        test_bitfield_zero_extract();
        test_combined_patterns();
    }
    
    /* Final computation using all volatile globals */
    int result = g_volatile_int + g_volatile_short + g_volatile_char;
    printf("Final result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
