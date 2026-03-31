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

/* Volatile arrays for memory operands */
volatile float farr[4] = {0.0f, 1.0f, 2.0f, 3.0f};
volatile double darr[4] = {0.0, 1.0, 2.0, 3.0};
volatile long double ldarr[2] = {0.0L, 1.0L};

/* Struct with volatile members */
struct volatile_floats {
    volatile float f1;
    volatile float f2;
    volatile double d1;
    volatile double d2;
};

/* Function prototypes with noinline to isolate patterns */
__attribute__((noinline)) int test_unordered(void);
__attribute__((noinline)) int test_ordered(void);
__attribute__((noinline)) int test_uneq(void);
__attribute__((noinline)) int test_unge(void);
__attribute__((noinline)) int test_ungt(void);
__attribute__((noinline)) int test_unle(void);
__attribute__((noinline)) int test_unlt(void);
__attribute__((noinline)) int test_ltgt(void);
__attribute__((noinline)) int test_mixed_types(void);
__attribute__((noinline)) int test_complex_expressions(void);
__attribute__((noinline)) int test_memory_operands(void);

/* Helper to generate NaN */
static inline double make_nan(void) {
    volatile double x = 0.0;
    return x / x;
}

/* Test UNORDERED condition code */
__attribute__((noinline)) int test_unordered(void) {
    volatile double a = make_nan();
    volatile double b = 1.0;
    int result = 0;
    
    /* Direct NaN comparison */
    if (a != a) {
        result |= 1;
    }
    
    /* Using isunordered */
    if (isunordered(a, b)) {
        result |= 2;
    }
    
    /* Inline assembly to force condition code use */
    volatile double c = make_nan();
    volatile double d = 2.0;
    
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l1"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)"
        : unordered_label
    );
    
    result |= 4;
    goto end_unordered;
    
unordered_label:
    result |= 8;
    
end_unordered:
    sink(result);
    return result;
}

/* Test ORDERED condition code */
__attribute__((noinline)) int test_ordered(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    int result = 0;
    
    /* Direct ordered comparison */
    if (a == a) {
        result |= 1;
    }
    
    /* Using isordered */
    if (isordered(a, b)) {
        result |= 2;
    }
    
    /* Complex ordered check */
    volatile double c = 3.0;
    if (!isunordered(a, c)) {
        result |= 4;
    }
    
    /* Inline assembly with ordered condition */
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l1"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)"
        : ordered_label
    );
    
    result |= 8;
    goto end_ordered;
    
ordered_label:
    result |= 16;
    
end_ordered:
    sink(result);
    return result;
}

/* Test UNEQ (unordered or equal) */
__attribute__((noinline)) int test_uneq(void) {
    volatile double a = make_nan();
    volatile double b = make_nan();
    volatile double c = 1.0;
    volatile double d = 1.0;
    int result = 0;
    
    /* Generate UNEQ: !(a < b) && !(a > b) */
    if (!(a < b) && !(a > b)) {
        result |= 1;
    }
    
    /* Alternative: a == b (including NaN case) */
    if (!(a != b)) {
        result |= 2;
    }
    
    /* With inline assembly */
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l1"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)"
        : uneq_label
    );
    
    result |= 4;
    goto end_uneq;
    
uneq_label:
    result |= 8;
    
end_uneq:
    sink(result);
    return result;
}

/* Test UNGE (not less than) */
__attribute__((noinline)) int test_unge(void) {
    volatile double a = 2.0;
    volatile double b = 1.0;
    volatile double c = make_nan();
    int result = 0;
    
    /* Direct: !(a < b) */
    if (!(a < b)) {
        result |= 1;
    }
    
    /* With NaN operand */
    if (!(c < b)) {
        result |= 2;
    }
    
    /* Using >= with volatile */
    volatile double x = 3.0;
    volatile double y = 2.0;
    if (x >= y) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Test UNGT (not less than or equal) */
__attribute__((noinline)) int test_ungt(void) {
    volatile double a = 3.0;
    volatile double b = 2.0;
    volatile double c = make_nan();
    int result = 0;
    
    /* Direct: !(a <= b) */
    if (!(a <= b)) {
        result |= 1;
    }
    
    /* With NaN */
    if (!(c <= b)) {
        result |= 2;
    }
    
    /* Using > */
    if (a > b) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Test UNLE (unordered or less than or equal) */
__attribute__((noinline)) int test_unle(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double c = make_nan();
    int result = 0;
    
    /* Direct: !(a > b) */
    if (!(a > b)) {
        result |= 1;
    }
    
    /* With NaN */
    if (!(c > b)) {
        result |= 2;
    }
    
    /* Using <= */
    if (a <= b) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Test UNLT (unordered or less than) */
__attribute__((noinline)) int test_unlt(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double c = make_nan();
    int result = 0;
    
    /* Direct: !(a >= b) */
    if (!(a >= b)) {
        result |= 1;
    }
    
    /* With NaN */
    if (!(c >= b)) {
        result |= 2;
    }
    
    /* Using < */
    if (a < b) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Test LTGT (less than or greater than - not equal and ordered) */
__attribute__((noinline)) int test_ltgt(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double c = make_nan();
    int result = 0;
    
    /* Direct: (a < b) || (a > b) */
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    /* Negated equality: !(a == b) */
    if (!(a == b)) {
        result |= 2;
    }
    
    /* With NaN (should be false for ordered requirement) */
    if ((c < b) || (c > b)) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Test with mixed floating-point types */
__attribute__((noinline)) int test_mixed_types(void) {
    volatile float f1 = 1.0f;
    volatile float f2 = 2.0f;
    volatile double d1 = 3.0;
    volatile double d2 = 4.0;
    volatile long double ld1 = 5.0L;
    volatile long double ld2 = 6.0L;
    int result = 0;
    
    /* Float comparisons */
    if (!(f1 >= f2)) {  /* UNLT */
        result |= 1;
    }
    
    if (!(d1 <= d2)) {  /* UNGT */
        result |= 2;
    }
    
    /* Long double */
    if (!(ld1 > ld2)) {  /* UNLE */
        result |= 4;
    }
    
    /* Mixed type comparison */
    if ((double)f1 != d1) {  /* LTGT */
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* Complex nested expressions */
__attribute__((noinline)) int test_complex_expressions(void) {
    volatile double a = make_nan();
    volatile double b = 1.0;
    volatile double c = 2.0;
    volatile double d = 3.0;
    int result = 0;
    
    /* Ternary with unordered check */
    int r1 = (a != a) ? 100 : ((b < c) ? 200 : 300);
    result += r1;
    
    /* Logical AND/OR combination */
    if ((!(a >= b) && (c > d)) || (isunordered(c, d))) {
        result += 50;
    }
    
    /* Nested comparisons */
    int r2 = (!(b <= c)) ? 1 : ((!(c >= d)) ? 2 : 3);
    result += r2;
    
    /* Complex condition */
    if (((a != a) || (b == b)) && (!(c < d) || (d != d))) {
        result += 25;
    }
    
    sink(result);
    return result;
}

/* Test with memory operands */
__attribute__((noinline)) int test_memory_operands(void) {
    struct volatile_floats vf = {1.0f, 2.0f, 3.0, 4.0};
    int result = 0;
    
    /* Struct member comparisons */
    if (!(vf.f1 >= vf.f2)) {  /* UNLT */
        result |= 1;
    }
    
    if (!(vf.d1 <= vf.d2)) {  /* UNGT */
        result |= 2;
    }
    
    /* Array element comparisons */
    if (!(farr[0] > farr[1])) {  /* UNLE */
        result |= 4;
    }
    
    if ((darr[2] < darr[3]) || (darr[2] > darr[3])) {  /* LTGT */
        result |= 8;
    }
    
    /* Load and compare */
    volatile double x = darr[0];
    volatile double y = darr[1];
    if (!(x == y)) {  /* LTGT */
        result |= 16;
    }
    
    sink(result);
    sink_ptr(&vf);
    return result;
}

int main(void) {
    int checksum = 0;
    
    printf("Testing i386 condition codes...\n");
    
    /* Initialize volatile seed */
    volatile int seed = 42;
    srand(seed);
    
    /* Call all test functions */
    checksum += test_unordered();
    checksum += test_ordered();
    checksum += test_uneq();
    checksum += test_unge();
    checksum += test_ungt();
    checksum += test_unle();
    checksum += test_unlt();
    checksum += test_ltgt();
    checksum += test_mixed_types();
    checksum += test_complex_expressions();
    checksum += test_memory_operands();
    
    /* Print checksum to ensure all code is live */
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
