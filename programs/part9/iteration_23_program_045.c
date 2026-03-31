/* Test program to trigger uncovered RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimizations from removing our test code */
volatile int g_volatile_int = 0;
volatile char g_volatile_char = 0;
volatile short g_volatile_short = 0;

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
    s1.value = 511;  /* Max for 10 bits */
    s1.mode = 3;
    
    /* Bit-field to bit-field copy */
    s2.flag = s1.flag;
    s2.value = s1.value;
    s2.mode = s1.mode;
    
    /* Bit-field to integer extraction */
    unsigned int extracted = s1.value;
    g_volatile_int = extracted;
    
    /* Integer to bit-field with masking */
    unsigned int source = 0xFFFF;
    s1.value = source & 0x3FF;  /* Should generate ZERO_EXTRACT with AND */
    
    /* Complex expression with bit-fields */
    s1.flag = (s1.value > 100) ? 1 : 0;
    
    /* Nested bit-field structure */
    struct NestedBitField nbf = {0};
    nbf.nibbles.a = 5;
    nbf.nibbles.b = 10;
    nbf.full = (nbf.nibbles.a << 4) | nbf.nibbles.b;
    
    g_volatile_int = nbf.full;
}

/* ==================== PARTIAL REGISTER TESTS (STRICT_LOW_PART) ==================== */

void test_partial_register_strict_low_part(void) {
    volatile short vs;
    volatile char vc;
    volatile int vi;
    
    /* Cast to smaller types - may generate STRICT_LOW_PART */
    vi = 0x12345678;
    vs = (short)vi;          /* Truncation to 16 bits */
    vc = (char)vi;           /* Truncation to 8 bits */
    
    /* Arithmetic on partial types */
    short s1 = 100;
    short s2 = 200;
    vs = s1 + s2;            /* Operation on 16-bit types */
    
    /* Increment partial type */
    char c = 50;
    vc = c + 1;
    
    /* Mixed-size operations */
    int i = 1000;
    vs = (short)(i + 500);   /* Operation then truncation */
    
    /* Pointer dereference with partial type */
    short *ps = &vs;
    *ps = 0xABCD;
    
    /* Array with partial types */
    char arr[4] = {1, 2, 3, 4};
    vc = arr[2];
    
    /* Loop with partial type update */
    for (char loop_c = 0; loop_c < 10; loop_c++) {
        vc = loop_c;
    }
}

/* ==================== SUB-REGISTER TESTS (SUBREG) ==================== */

/* Vector type for SUBREG generation */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

/* Packed structure for SUBREG access */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

union TypePunningUnion {
    int i;
    float f;
    struct {
        short s1;
        short s2;
    } halves;
};

void test_subreg_patterns(void) {
    /* Vector operations - should generate SUBREG for element access */
    v4si vec = {1, 2, 3, 4};
    int element = vec[2];           /* SUBREG access to vector element */
    g_volatile_int = element;
    
    /* Vector to scalar conversion */
    v8hi vec_short = {1, 2, 3, 4, 5, 6, 7, 8};
    short s_element = vec_short[3];
    g_volatile_short = s_element;
    
    /* Packed structure access - misaligned access may use SUBREG */
    struct PackedStruct ps;
    ps.a = 0xAA;
    ps.b = 0x12345678;
    ps.c = 0xBBBB;
    
    int b_val = ps.b;  /* May generate SUBREG due to packed misalignment */
    g_volatile_int = b_val;
    
    /* Type punning through union */
    union TypePunningUnion u;
    u.i = 0x40000000;  /* ~2.0 in float */
    float f = u.f;     /* SUBREG may be used for type conversion */
    
    /* Access halves of integer */
    u.i = 0x12345678;
    short half1 = u.halves.s1;
    short half2 = u.halves.s2;
    g_volatile_short = half1 + half2;
    
    /* Float to int bit manipulation */
    float fval = 3.14f;
    int ival;
    memcpy(&ival, &fval, sizeof(int));  /* Avoid strict aliasing violation */
    ival &= 0x7FFFFFFF;                 /* Clear sign bit */
    g_volatile_int = ival;
}

/* ==================== COMBINED PATTERNS ==================== */

struct Combined {
    volatile unsigned int low:8;
    volatile unsigned int high:8;
    volatile int full;
};

void test_combined_patterns(void) {
    struct Combined c = {0};
    
    /* Bit-field to partial register */
    c.low = 0xAB;
    c.high = 0xCD;
    
    /* Extract bit-field to short (combines ZERO_EXTRACT and SUBREG/STRICT_LOW_PART) */
    unsigned int combined = (c.high << 8) | c.low;
    volatile short vs_combined = (short)combined;
    
    /* Complex expression with multiple patterns */
    for (int i = 0; i < 4; i++) {
        c.low = (c.low + 1) & 0xFF;      /* ZERO_EXTRACT with arithmetic */
        vs_combined = (short)(c.low | (c.high << 8));  /* Combined patterns */
        
        /* Conditional based on bit-field */
        if (c.low & 0x80) {
            c.high ^= 0xFF;              /* Another ZERO_EXTRACT */
        }
    }
    
    /* Use builtins that may involve bit manipulation */
    int popcnt = __builtin_popcount(c.full);
    g_volatile_int = popcnt;
    
    /* Parity check on bit-field */
    unsigned int parity = __builtin_parity(c.low);
    c.low ^= (parity << 7);  /* Set high bit based on parity */
}

/* ==================== ARCHITECTURE-SPECIFIC TESTS ==================== */

#ifdef __i386__
void test_x86_specific(void) {
    /* x86 inline assembly that uses partial registers */
    int result;
    short s_result;
    
    /* Assembly with 8-bit output */
    __asm__ volatile (
        "movb $0x42, %%al\n\t"
        "movb %%al, %0"
        : "=r" (g_volatile_char)
        :
        : "%al"
    );
    
    /* Assembly with 16-bit output */
    __asm__ volatile (
        "movw $0x1234, %%ax\n\t"
        "movw %%ax, %0"
        : "=r" (s_result)
        :
        : "%ax"
    );
    g_volatile_short = s_result;
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    /* ARM may use STRICT_LOW_PART for byte/short operations */
    volatile char arm_char;
    volatile short arm_short;
    
    /* These operations might generate partial register patterns on ARM */
    arm_char = (char)g_volatile_int;
    arm_short = (short)(g_volatile_int * 2);
    
    g_volatile_char = arm_char;
    g_volatile_short = arm_short;
}
#endif

/* ==================== MAIN TEST DRIVER ==================== */

typedef void (*test_func_t)(void);

const test_func_t test_functions[] = {
    test_bitfield_zero_extract,
    test_partial_register_strict_low_part,
    test_subreg_patterns,
    test_combined_patterns,
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
    
    /* Use command line or volatile to control which tests run */
    if (argc > 1) {
        test_to_run = atoi(argv[1]) % 10;
    } else {
        test_to_run = g_volatile_int % 5;
    }
    
    /* Run all tests in sequence to ensure all code paths are compiled */
    for (int i = 0; test_functions[i] != NULL; i++) {
        if (test_to_run == 0 || test_to_run == i + 1) {
            test_functions[i]();
        }
    }
    
    /* Ensure the program does real work so it's not optimized away entirely */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += i * g_volatile_int;
    }
    
    printf("Test completed. Result: %d\n", sum);
    return sum != 0 ? 0 : 1;
}
