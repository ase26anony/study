/* test_i386_condition_codes.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent optimization */
extern void sink(int);
extern void sink_ptr(void*);

/* Global volatile variables to prevent constant folding */
volatile double g_nan = 0.0/0.0;
volatile double g_inf = 1.0/0.0;
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile double g_two = 2.0;

/* Volatile arrays for memory operand testing */
volatile float farr[4] = {0.0f, 1.0f, 2.0f, 3.0f};
volatile double darr[4] = {0.0, 1.0, 2.0, 3.0};
volatile long double ldarr[4] = {0.0L, 1.0L, 2.0L, 3.0L};

/* Struct with volatile members for complex addressing */
struct fp_struct {
    volatile float f;
    volatile double d;
    volatile long double ld;
};

volatile struct fp_struct fps = {0.0f, 0.0, 0.0L};

/* ========== Pattern 1: UNORDERED comparisons ========== */
__attribute__((noinline))
int test_unordered(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile float c = farr[0];
    volatile long double d = ldarr[1];
    
    int result = 0;
    
    /* Direct NaN comparison for unordered */
    if (a != a) {  /* Should generate UNORDERED */
        result |= 1;
    }
    
    /* Using isunordered() */
    if (isunordered(a, b)) {
        result |= 2;
    }
    
    /* Complex expression with unordered */
    if (isunordered(c, d) || (b > g_zero)) {
        result |= 4;
    }
    
    /* Inline assembly to force condition code use */
    __asm__ goto (
        "fcom %%st(1)\n\t"
        "fstsw %%ax\n\t"
        "sahf\n\t"
        "j%c0 %l1"
        : /* no outputs */
        : /* no inputs */
        : "ax", "cc"
        : unordered_label
    );
    
    result |= 8;
    goto skip_unordered;
    
unordered_label:
    result |= 16;
    
skip_unordered:
    sink(result);
    return result;
}

/* ========== Pattern 2: ORDERED comparisons ========== */
__attribute__((noinline))
int test_ordered(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile float c = farr[1];
    
    int result = 0;
    
    /* Direct ordered comparison */
    if (a == a) {  /* Should generate ORDERED */
        result |= 1;
    }
    
    /* Using isordered() */
    if (isordered(a, b)) {
        result |= 2;
    }
    
    /* Complex ordered expression */
    if (isordered(c, b) && (a < b)) {
        result |= 4;
    }
    
    /* Inline assembly with ordered condition */
    __asm__ goto (
        "fcomi %%st(1)\n\t"
        "j%c0 %l1"
        : /* no outputs */
        : /* no inputs */
        : "cc"
        : ordered_label
    );
    
    result |= 8;
    goto skip_ordered;
    
ordered_label:
    result |= 16;
    
skip_ordered:
    sink(result);
    return result;
}

/* ========== Pattern 3: UNEQ (unordered or equal) ========== */
__attribute__((noinline))
int test_uneq(void) {
    volatile double a = g_nan;
    volatile double b = g_nan;
    volatile double c = g_one;
    volatile double d = g_one;
    
    int result = 0;
    
    /* UNEQ: !(a < b) && !(a > b) which is (a == b) || unordered */
    if (!(a < b) && !(a > b)) {
        result |= 1;
    }
    
    /* Alternative: (a == b) || (a != a) */
    if ((c == d) || (a != a)) {
        result |= 2;
    }
    
    /* Complex expression with memory operands */
    volatile double* pa = &darr[0];
    volatile double* pb = &darr[1];
    if (!(*pa < *pb) && !(*pa > *pb)) {
        result |= 4;
    }
    
    /* Inline assembly for UNEQ */
    __asm__ goto (
        "fcomip %%st(1)\n\t"
        "fstp %%st(0)\n\t"
        "j%c0 %l1"
        : /* no outputs */
        : /* no inputs */
        : "cc"
        : uneq_label
    );
    
    result |= 8;
    goto skip_uneq;
    
uneq_label:
    result |= 16;
    
skip_uneq:
    sink(result);
    return result;
}

/* ========== Pattern 4: UNGE (not less than) ========== */
__attribute__((noinline))
int test_unge(void) {
    volatile double a = g_two;
    volatile double b = g_one;
    volatile float c = farr[2];
    volatile float d = farr[1];
    
    int result = 0;
    
    /* UNGE: !(a < b) */
    if (!(a < b)) {
        result |= 1;
    }
    
    /* Alternative: (a >= b) || (a != a) */
    if ((a >= b) || (g_nan != g_nan)) {
        result |= 2;
    }
    
    /* With struct member access */
    fps.d = 3.0;
    volatile double* pb = &darr[0];
    if (!(fps.d < *pb)) {
        result |= 4;
    }
    
    /* Complex nested expression */
    result = (c >= d) ? (result | 8) : (result & ~8);
    
    sink(result);
    return result;
}

/* ========== Pattern 5: UNGT (not less than or equal) ========== */
__attribute__((noinline))
int test_ungt(void) {
    volatile double a = g_two;
    volatile double b = g_one;
    volatile long double c = ldarr[2];
    volatile long double d = ldarr[0];
    
    int result = 0;
    
    /* UNGT: !(a <= b) */
    if (!(a <= b)) {
        result |= 1;
    }
    
    /* Alternative: (a > b) || (a != a) */
    if ((a > b) || (g_nan != g_nan)) {
        result |= 2;
    }
    
    /* Using array elements with different types */
    if (!(darr[2] <= darr[1])) {
        result |= 4;
    }
    
    /* Inline assembly for UNGT */
    __asm__ goto (
        "fucomip %%st(1)\n\t"
        "fstp %%st(0)\n\t"
        "j%c0 %l1"
        : /* no outputs */
        : /* no inputs */
        : "cc"
        : ungt_label
    );
    
    result |= 8;
    goto skip_ungt;
    
ungt_label:
    result |= 16;
    
skip_ungt:
    sink(result);
    return result;
}

/* ========== Pattern 6: UNLE (unordered or less than or equal) ========== */
__attribute__((noinline))
int test_unle(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    
    int result = 0;
    
    /* UNLE: !(a > b) */
    if (!(a > b)) {
        result |= 1;
    }
    
    /* Complex expression */
    if ((a <= b) || (c != c)) {
        result |= 2;
    }
    
    /* Nested ternary with UNLE */
    result = !(farr[0] > farr[1]) ? (result | 4) : (result & ~4);
    
    /* Multiple comparisons in expression */
    int temp = (!(a > b)) + (!(b > a)) * 2;
    result |= (temp << 3);
    
    sink(result);
    return result;
}

/* ========== Pattern 7: UNLT (unordered or less than) ========== */
__attribute__((noinline))
int test_unlt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile float c = farr[0];
    volatile float d = farr[3];
    
    int result = 0;
    
    /* UNLT: !(a >= b) */
    if (!(a >= b)) {
        result |= 1;
    }
    
    /* Alternative: (a < b) || (a != a) */
    if ((a < b) || (g_nan != g_nan)) {
        result |= 2;
    }
    
    /* With struct member */
    fps.f = 1.0f;
    if (!(fps.f >= d)) {
        result |= 4;
    }
    
    /* Inline assembly for UNLT */
    __asm__ goto (
        "fucomi %%st(1)\n\t"
        "j%c0 %l1"
        : /* no outputs */
        : /* no inputs */
        : "cc"
        : unlt_label
    );
    
    result |= 8;
    goto skip_unlt;
    
unlt_label:
    result |= 16;
    
skip_unlt:
    sink(result);
    return result;
}

/* ========== Pattern 8: LTGT (less than or greater than, ordered) ========== */
__attribute__((noinline))
int test_ltgt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    
    int result = 0;
    
    /* LTGT: (a < b) || (a > b) */
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    /* Alternative: !(a == b) && !(a != a) */
    if (!(a == b) && !(a != a)) {
        result |= 2;
    }
    
    /* Complex expression with memory */
    volatile double* pa = &darr[1];
    volatile double* pb = &darr[2];
    if ((*pa < *pb) || (*pa > *pb)) {
        result |= 4;
    }
    
    /* Nested in larger expression */
    result = ((a < b) || (a > b)) ? (result | 8) : (result & ~8);
    
    /* Inline assembly for LTGT */
    __asm__ goto (
        "fcomip %%st(1)\n\t"
        "j%c0 %l1"
        : /* no outputs */
        : /* no inputs */
        : "cc"
        : ltgt_label
    );
    
    result |= 16;
    goto skip_ltgt;
    
ltgt_label:
    result |= 32;
    
skip_ltgt:
    sink(result);
    return result;
}

/* ========== Combined test with all patterns ========== */
__attribute__((noinline))
int test_combined(void) {
    volatile double x = g_nan;
    volatile double y = g_one;
    volatile double z = g_two;
    volatile float f1 = farr[0];
    volatile float f2 = farr[1];
    
    int result = 0;
    
    /* Mix of different condition codes in one function */
    if (x != x) {  /* UNORDERED */
        result += 1;
    }
    
    if (!(y < z)) {  /* UNGE */
        result += 2;
    }
    
    if ((f1 < f2) || (f1 > f2)) {  /* LTGT */
        result += 4;
    }
    
    if (!(y > z)) {  /* UNLE */
        result += 8;
    }
    
    /* Ternary with mixed conditions */
    result += (isunordered(x, y) ? 16 : 32);
    result += (!(z <= y) ? 64 : 128);
    
    /* Complex logical expression */
    if ((!(x == x) && isordered(y, z)) || (!(y >= z) && !isunordered(f1, f2))) {
        result += 256;
    }
    
    sink(result);
    return result;
}

/* ========== Main function ========== */
int main(void) {
    int checksum = 0;
    
    /* Initialize with potentially NaN values */
    g_nan = 0.0/0.0;
    g_inf = 1.0/0.0;
    
    /* Call all test functions */
    checksum ^= test_unordered();
    checksum ^= test_ordered();
    checksum ^= test_uneq();
    checksum ^= test_unge();
    checksum ^= test_ungt();
    checksum ^= test_unle();
    checksum ^= test_unlt();
    checksum ^= test_ltgt();
    checksum ^= test_combined();
    
    /* Additional calls with different arguments */
    for (int i = 0; i < 3; i++) {
        darr[0] = (i == 0) ? g_nan : (i == 1) ? g_inf : g_one;
        checksum += test_combined();
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

/* Dummy sink functions to prevent optimization */
void sink(int x) {
    volatile static int dummy;
    dummy = x;
}

void sink_ptr(void* p) {
    volatile static void* dummy;
    dummy = p;
}
