/* test_resources.c - Program to trigger specific RTL patterns in GCC's resource tracking */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_int = 0;
volatile short g_volatile_short = 0;
volatile char g_volatile_char = 0;

/* ==================== BIT-FIELD PATTERNS (ZERO_EXTRACT) ==================== */

/* Structure with various bit-field widths */
struct BitFieldStruct {
    volatile unsigned int flag:1;
    volatile unsigned int small:3;
    volatile unsigned int medium:10;
    volatile unsigned int large:18;
    unsigned int padding:31;  /* Ensure total size is at least 32 bits */
};

/* Union with bit-field and full integer for type-punning */
union BitFieldUnion {
    struct {
        volatile unsigned int low:8;
        volatile unsigned int high:8;
    } bits;
    volatile unsigned int full;
};

/* Test 1: Basic bit-field assignments */
void test_bitfield_basic(void) {
    struct BitFieldStruct s = {0};
    
    /* Multiple bit-field assignments */
    s.flag = 1;
    s.small = 5;
    s.medium = 255;
    s.large = 131071;
    
    /* Bit-field reads */
    g_volatile_int = s.flag;
    g_volatile_int = s.small;
    g_volatile_int = s.medium;
    g_volatile_int = s.large;
    
    /* Cross assignments between bit-fields */
    s.small = s.flag;
    s.medium = s.small;
    s.large = s.medium;
}

/* Test 2: Complex bit-field expressions */
void test_bitfield_complex(void) {
    union BitFieldUnion u;
    u.full = 0xABCD;
    
    /* Access through both union members */
    g_volatile_int = u.full;
    g_volatile_int = u.bits.low;
    g_volatile_int = u.bits.high;
    
    /* Modify through bit-fields */
    u.bits.low = 0xEF;
    u.bits.high = 0x12;
    
    /* Compound expressions with bit-fields */
    struct BitFieldStruct s1 = {0}, s2 = {0};
    s1.flag = 1;
    s2.medium = 100;
    
    /* Mixed operations */
    s1.small = s2.medium & 0x7;
    s1.large = (s2.medium << 5) | s1.small;
}

/* Test 3: Bit-fields in loops and conditionals */
void test_bitfield_loops(void) {
    struct BitFieldStruct arr[4];
    
    for (int i = 0; i < 4; i++) {
        arr[i].flag = i & 1;
        arr[i].small = i & 0x7;
        arr[i].medium = i * 64;
        arr[i].large = i * 16384;
    }
    
    /* Conditional bit-field access */
    volatile int cond = g_volatile_int;
    for (int i = 0; i < 4; i++) {
        if (cond & (1 << i)) {
            g_volatile_int = arr[i].medium;
            arr[i].small = g_volatile_int & 0x7;
        }
    }
}

/* ==================== PARTIAL REGISTER PATTERNS (STRICT_LOW_PART) ==================== */

/* Test 4: Sub-word type operations */
void test_partial_register(void) {
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    
    /* Assignments to partial registers */
    vs1 = (short)g_volatile_int;
    vc1 = (char)g_volatile_int;
    
    /* Arithmetic on sub-word types */
    vs2 = vs1 + 100;
    vc2 = vc1 - 50;
    
    /* Compound assignments */
    vs1 += vs2;
    vc1 *= vc2;
    
    /* Mixed-size operations */
    int temp = vs1;
    temp += vc1;
    vs2 = (short)temp;
    
    /* Pointer to sub-word type */
    short *ps = &vs1;
    *ps = *ps + 1;
}

/* Test 5: Architecture-specific partial register patterns */
void test_arch_partial(void) {
    /* These patterns are architecture-dependent */
    
#ifdef __i386__ || __x86_64__
    /* x86 has explicit partial register access */
    volatile unsigned short us;
    volatile unsigned char uc;
    
    /* Operations that might use AX/AL vs EAX */
    us = g_volatile_int & 0xFFFF;
    uc = g_volatile_int & 0xFF;
    
    /* Mixed-size arithmetic */
    us = us + uc;
#endif
    
#ifdef __arm__
    /* ARM may generate STRICT_LOW_PART for 16-bit operations */
    volatile int16_t i16;
    volatile int8_t i8;
    
    i16 = g_volatile_int;
    i8 = i16 >> 8;
#endif
}

/* ==================== SUBREG PATTERNS ==================== */

/* Test 6: Vector extensions (likely to generate SUBREG) */
void test_vector_subreg(void) {
    /* GCC vector extension */
    typedef int v4si __attribute__ ((vector_size (16)));
    typedef short v8hi __attribute__ ((vector_size (16)));
    
    v4si v1 = {1, 2, 3, 4};
    v8hi v2 = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Access vector elements (generates SUBREG) */
    g_volatile_int = v1[2];
    g_volatile_short = v2[5];
    
    /* Vector operations */
    v1[0] = g_volatile_int;
    v2[3] = g_volatile_short;
    
    /* Type punning between vector types */
    v8hi v3 = *(v8hi*)&v1;
    g_volatile_short = v3[1];
}

/* Test 7: Packed structures and unions */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

union MixedUnion {
    float f;
    int i;
    short s[2];
};

void test_packed_union(void) {
    struct PackedStruct ps;
    union MixedUnion mu;
    
    /* Access packed structure members */
    ps.a = 1;
    ps.b = 0x12345678;
    ps.c = 0x9ABC;
    
    /* These accesses may involve SUBREG */
    g_volatile_char = ps.a;
    g_volatile_int = ps.b;
    g_volatile_short = ps.c;
    
    /* Union type-punning */
    mu.f = 3.14159f;
    g_volatile_int = mu.i;          /* SUBREG for float->int reinterpretation */
    g_volatile_short = mu.s[0];     /* SUBREG for partial access */
    
    mu.i = 0x40490FDA;  /* Approx pi in float representation */
    g_volatile_short = mu.s[1];
}

/* Test 8: Float/integer conversions */
void test_float_conversions(void) {
    volatile float f1, f2;
    volatile double d1, d2;
    
    f1 = (float)g_volatile_int;
    d1 = (double)g_volatile_short;
    
    /* Conversions that may involve SUBREG */
    g_volatile_int = (int)f1;
    g_volatile_short = (short)d1;
    
    /* Mixed operations */
    f2 = f1 + (float)g_volatile_short;
    d2 = d1 - (double)g_volatile_int;
    
    /* Memory access with different types */
    *(volatile float*)&g_volatile_int = 1.5f;  /* Type punning */
}

/* ==================== COMBINED PATTERNS ==================== */

/* Test 9: Combined bit-field and partial register */
void test_combined_patterns(void) {
    struct BitFieldStruct s;
    s.medium = 500;
    s.small = 3;
    
    /* Bit-field read to partial register */
    g_volatile_short = (short)s.medium;  /* ZERO_EXTRACT + SUBREG/STRICT_LOW_PART */
    
    /* Partial register write to bit-field */
    s.small = g_volatile_char & 0x7;
    
    /* Complex expression */
    volatile int temp = s.large;
    temp = (temp << 16) | (g_volatile_short & 0xFFFF);
    s.medium = (temp >> 8) & 0x3FF;
}

/* Test 10: Builtin bit operations */
void test_builtin_bitops(void) {
    unsigned int x = g_volatile_int;
    
    /* Builtins that may involve bit extraction */
    g_volatile_int = __builtin_popcount(x);
    g_volatile_int = __builtin_ctz(x);
    g_volatile_int = __builtin_clz(x);
    g_volatile_int = __builtin_parity(x);
    
    /* Bit manipulation builtins */
    g_volatile_int = __builtin_bswap32(x);
    
    /* Extract bit-field using builtins */
    g_volatile_int = (x >> 5) & 0x1F;  /* Manual extraction */
}

/* ==================== MAIN TEST DRIVER ==================== */

typedef void (*test_func_t)(void);

const test_func_t test_functions[] = {
    test_bitfield_basic,
    test_bitfield_complex,
    test_bitfield_loops,
    test_partial_register,
    test_arch_partial,
    test_vector_subreg,
    test_packed_union,
    test_float_conversions,
    test_combined_patterns,
    test_builtin_bitops,
    NULL
};

const char* test_names[] = {
    "bitfield_basic",
    "bitfield_complex",
    "bitfield_loops",
    "partial_register",
    "arch_partial",
    "vector_subreg",
    "packed_union",
    "float_conversions",
    "combined_patterns",
    "builtin_bitops"
};

int main(int argc, char *argv[]) {
    int num_tests = sizeof(test_names) / sizeof(test_names[0]);
    
    /* Use command-line argument to select test, or run all */
    if (argc > 1) {
        int test_num = atoi(argv[1]);
        if (test_num >= 0 && test_num < num_tests) {
            printf("Running test %d: %s\n", test_num, test_names[test_num]);
            test_functions[test_num]();
        } else {
            printf("Invalid test number. Use 0-%d\n", num_tests - 1);
            return 1;
        }
    } else {
        /* Run all tests */
        printf("Running all %d tests...\n", num_tests);
        for (int i = 0; i < num_tests; i++) {
            if (test_functions[i]) {
                printf("Test %d: %s\n", i, test_names[i]);
                test_functions[i]();
            }
        }
    }
    
    /* Final computation to ensure program does something useful */
    int result = g_volatile_int + g_volatile_short + g_volatile_char;
    printf("Final result (prevent optimization): %d\n", result);
    
    return 0;
}
