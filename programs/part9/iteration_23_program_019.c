/* Test program to trigger uncovered RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_int = 0;
volatile short g_volatile_short = 0;
volatile char g_volatile_char = 0;

/* ==================== ZERO_EXTRACT patterns ==================== */

/* Bit-field structures for ZERO_EXTRACT */
struct BitField1 {
    volatile unsigned int flag:1;
    unsigned int value:10;
    unsigned int pad:21;
};

struct BitField2 {
    unsigned int a:3;
    unsigned int b:5;
    unsigned int c:8;
    unsigned int d:16;
};

/* Union with bit-field and full integer */
union BitFieldUnion {
    struct {
        volatile unsigned int low:8;
        unsigned int high:24;
    } bits;
    unsigned int full;
};

void test_zero_extract(void) {
    struct BitField1 bf1 = {0};
    struct BitField2 bf2 = {0};
    union BitFieldUnion u = {0};
    
    /* Basic bit-field assignments */
    bf1.flag = 1;
    bf1.value = 511; /* Max for 10 bits */
    
    /* Cross assignments */
    unsigned int temp = bf1.value;
    bf2.a = temp & 0x7;
    bf2.b = (temp >> 3) & 0x1F;
    
    /* Union bit-field access */
    u.bits.low = 0xFF;
    u.full = u.full + 1;
    
    /* Complex bit-field expression */
    bf2.c = (bf1.value + bf2.b) & 0xFF;
    
    /* Prevent dead code elimination */
    g_volatile_int = bf1.flag + bf2.a + u.bits.low;
}

/* ==================== STRICT_LOW_PART patterns ==================== */

void test_strict_low_part(void) {
    volatile short vs;
    volatile char vc;
    int i = g_volatile_int;
    
    /* Partial register assignments */
    vs = (short)(i + 100);
    vc = (char)(i * 2);
    
    /* Arithmetic on sub-word types */
    short s1 = 1000;
    short s2 = 2000;
    vs = s1 + s2;  /* May overflow short range */
    
    char c1 = 50;
    char c2 = 60;
    vc = c1 * c2;  /* May overflow char range */
    
    /* Mixed-size operations */
    int result = vs + vc;
    vs = (short)(result & 0xFFFF);
    
    /* Store to global volatile */
    g_volatile_short = vs;
    g_volatile_char = vc;
}

/* ==================== SUBREG patterns ==================== */

/* Vector type using GCC extension */
typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));

/* Packed structure */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

void test_subreg(void) {
    /* Vector operations */
    v4si v = {1, 2, 3, 4};
    v8hi vh = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Vector element access - generates SUBREG */
    int element = v[2];
    short helement = vh[3];
    
    /* Vector operations */
    v4si v2 = v + (v4si){5, 5, 5, 5};
    element = v2[0] + v2[1];
    
    /* Packed structure access */
    struct PackedStruct ps;
    ps.a = 1;
    ps.b = 0x12345678;
    ps.c = 0x9ABC;
    
    /* Accessing misaligned members */
    int b_val = ps.b;  /* May involve SUBREG due to packing */
    short c_val = ps.c;
    
    /* Float/int conversions */
    float f = 3.14159f;
    int *int_ptr = (int*)&f;
    int int_val = *int_ptr;  /* Type punning */
    
    /* Store results to prevent optimization */
    g_volatile_int = element + b_val + int_val;
    g_volatile_short = helement + c_val;
}

/* ==================== COMBINED patterns ==================== */

void test_combined_patterns(void) {
    struct BitField1 bf = {0};
    union BitFieldUnion u = {0};
    volatile short vs;
    
    /* Bit-field to partial register */
    bf.value = 0x3FF;  /* 10 bits all 1 */
    vs = (short)bf.value;  /* ZERO_EXTRACT + SUBREG/STRICT_LOW_PART */
    
    /* Union with bit-field and type punning */
    u.bits.low = 0x80;
    u.bits.high = 0x00ABCD;
    
    /* Access through different union members */
    unsigned int full_val = u.full;
    char low_byte = u.bits.low;
    
    /* Complex expression with bit-fields */
    bf.flag = (full_val >> 31) & 1;
    bf.value = (full_val >> 8) & 0x3FF;
    
    /* Store to volatile of different size */
    g_volatile_char = low_byte;
    g_volatile_short = vs + bf.value;
    g_volatile_int = full_val;
}

/* ==================== ARCHITECTURE-SPECIFIC patterns ==================== */

#ifdef __i386__
void test_x86_specific(void) {
    int result;
    
    /* Inline assembly that might generate STRICT_LOW_PART */
    __asm__ volatile (
        "movw %1, %%ax\n\t"
        "addw $100, %%ax\n\t"
        "movw %%ax, %0\n\t"
        : "=r" (g_volatile_short)
        : "r" (g_volatile_short)
        : "%ax"
    );
    
    /* Bit manipulation builtins */
    unsigned int x = g_volatile_int;
    int count = __builtin_popcount(x);  /* May involve bit extraction */
    int parity = __builtin_parity(x);
    
    g_volatile_int = count + parity;
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    /* ARM-specific patterns */
    unsigned int x = g_volatile_int;
    
    /* ARM has good bit-field support */
    struct {
        unsigned int a:4;
        unsigned int b:4;
        unsigned int c:24;
    } arm_bf;
    
    arm_bf.a = (x >> 0) & 0xF;
    arm_bf.b = (x >> 4) & 0xF;
    arm_bf.c = (x >> 8) & 0xFFFFFF;
    
    g_volatile_int = arm_bf.a + arm_bf.b + arm_bf.c;
}
#endif

/* ==================== MAIN with execution control ==================== */

typedef void (*test_func_t)(void);

/* Array of test functions */
test_func_t test_functions[] = {
    test_zero_extract,
    test_strict_low_part,
    test_subreg,
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
    volatile int test_selector = 0;
    
    /* Use command line or environment to control which tests run */
    if (argc > 1) {
        test_selector = atoi(argv[1]);
    } else {
        /* Use a volatile source to prevent compile-time optimization */
        test_selector = g_volatile_int;
    }
    
    /* Run all tests in a loop to ensure code generation */
    for (int i = 0; test_functions[i] != NULL; i++) {
        if (test_selector == 0 || test_selector == i + 1) {
            test_functions[i]();
        }
    }
    
    /* Additional complex loop to encourage optimization */
    for (int i = 0; i < 100; i++) {
        struct BitField2 bf;
        bf.a = i & 0x7;
        bf.b = (i >> 3) & 0x1F;
        bf.c = (i >> 8) & 0xFF;
        bf.d = i * 2;
        
        g_volatile_int += bf.a + bf.b + bf.c + bf.d;
    }
    
    /* Final computation to ensure program does something */
    int result = g_volatile_int + g_volatile_short + g_volatile_char;
    printf("Result: %d\n", result);
    
    return result != 0;
}
