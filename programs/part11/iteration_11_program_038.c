/* test_i386_condition_codes.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent optimization */
extern void sink(int);
extern void sink_ptr(void*);

/* Volatile variables to prevent constant folding */
volatile double vd1, vd2, vd3;
volatile float vf1, vf2, vf3;
volatile long double vld1, vld2, vld3;

/* Volatile arrays for memory operands */
volatile double arr_d[4] = {0.0, 1.0, 2.0, 3.0};
volatile float arr_f[4] = {0.0f, 1.0f, 2.0f, 3.0f};

/* Struct for complex memory access */
struct FloatPair {
    volatile double a;
    volatile double b;
};
volatile struct FloatPair fp1, fp2;

/* Function prototypes with noinline to isolate patterns */
__attribute__((noinline)) int test_unordered(void);
__attribute__((noinline)) int test_ordered(void);
__attribute__((noinline)) int test_uneq(void);
__attribute__((noinline)) int test_unge(void);
__attribute__((noinline)) int test_ungt(void);
__attribute__((noinline)) int test_unle(void);
__attribute__((noinline)) int test_unlt(void);
__attribute__((noinline)) int test_ltgt(void);
__attribute__((noinline)) int test_mixed_float_double(void);
__attribute__((noinline)) int test_long_double_comparisons(void);
__attribute__((noinline)) int test_complex_expressions(void);
__attribute__((noinline)) int test_memory_operands(void);

/* Helper to generate NaN */
static double make_nan(void) {
    volatile double zero = 0.0;
    return zero / zero;
}

/* Helper to generate infinity */
static double make_inf(void) {
    volatile double large = 1e308;
    return large * large;
}

/* Test UNORDERED comparisons with NaN */
__attribute__((noinline)) int test_unordered(void) {
    int result = 0;
    volatile double nan = make_nan();
    volatile double normal = 3.14;
    
    /* Direct unordered check */
    if (nan != nan) {
        result |= 1;
    }
    
    /* Using isunordered() */
    if (isunordered(nan, normal)) {
        result |= 2;
    }
    
    /* Inline assembly to force condition code use */
    volatile double a = nan;
    volatile double b = normal;
    
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l1"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)"
        : unordered_label
    );
    
    /* Not taken path */
    result |= 4;
    goto end_unordered;
    
unordered_label:
    /* Taken path - unordered */
    result |= 8;
    
end_unordered:
    sink(result);
    return result;
}

/* Test ORDERED comparisons */
__attribute__((noinline)) int test_ordered(void) {
    int result = 0;
    volatile double a = 1.5;
    volatile double b = 2.5;
    
    /* Direct ordered check */
    if (a == a && b == b) {
        result |= 1;
    }
    
    /* Using isordered() */
    if (isordered(a, b)) {
        result |= 2;
    }
    
    /* Complex expression with ordered check */
    volatile double c = a + b;
    if (!isunordered(a, b) && (c > 0.0)) {
        result |= 4;
    }
    
    /* Inline assembly for ordered */
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
    int result = 0;
    volatile double a = 2.0;
    volatile double b = 2.0;
    volatile double nan = make_nan();
    
    /* Generate UNEQ: !(a != b) which is a == b */
    if (!(a != b)) {
        result |= 1;
    }
    
    /* UNEQ with NaN (unordered case) */
    if (!(nan != nan)) {  /* This is true for NaN */
        result |= 2;
    }
    
    /* Using || to create UNEQ pattern */
    if (isunordered(a, b) || (a == b)) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Test UNGE (not less than) */
__attribute__((noinline)) int test_unge(void) {
    int result = 0;
    volatile double a = 3.0;
    volatile double b = 2.0;
    volatile double c = 3.0;
    
    /* UNGE: !(a < b) which is a >= b */
    if (!(a < b)) {
        result |= 1;
    }
    
    /* With equal values */
    if (!(c < a)) {  /* 3.0 < 3.0 is false, so !false is true */
        result |= 2;
    }
    
    /* Complex expression */
    if (!(arr_d[0] < arr_d[1])) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Test UNGT (not less than or equal) */
__attribute__((noinline)) int test_ungt(void) {
    int result = 0;
    volatile double a = 3.0;
    volatile double b = 2.0;
    volatile double c = 3.0;
    
    /* UNGT: !(a <= b) which is a > b */
    if (!(a <= b)) {
        result |= 1;
    }
    
    /* With equal values (should be false) */
    if (!(c <= a)) {  /* 3.0 <= 3.0 is true, so !true is false */
        result |= 2;
    }
    
    /* Using memory operand */
    if (!(fp1.a <= fp2.b)) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Test UNLE (unordered or less than or equal) */
__attribute__((noinline)) int test_unle(void) {
    int result = 0;
    volatile double a = 2.0;
    volatile double b = 3.0;
    volatile double nan = make_nan();
    
    /* UNLE: (a <= b) || isunordered(a, b) */
    if ((a <= b) || isunordered(a, b)) {
        result |= 1;
    }
    
    /* With NaN */
    if ((nan <= a) || isunordered(nan, a)) {
        result |= 2;
    }
    
    /* Negated form: !(a > b) */
    if (!(a > b)) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Test UNLT (unordered or less than) */
__attribute__((noinline)) int test_unlt(void) {
    int result = 0;
    volatile double a = 2.0;
    volatile double b = 3.0;
    volatile double nan = make_nan();
    
    /* UNLT: (a < b) || isunordered(a, b) */
    if ((a < b) || isunordered(a, b)) {
        result |= 1;
    }
    
    /* With NaN */
    if ((nan < a) || isunordered(nan, a)) {
        result |= 2;
    }
    
    /* Negated form: !(a >= b) */
    if (!(a >= b)) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Test LTGT (not equal and ordered) */
__attribute__((noinline)) int test_ltgt(void) {
    int result = 0;
    volatile double a = 2.0;
    volatile double b = 3.0;
    volatile double c = 2.0;
    volatile double nan = make_nan();
    
    /* LTGT: (a < b) || (a > b)  (ordered and not equal) */
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    /* With equal values (should be false) */
    if ((c < a) || (c > a)) {
        result |= 2;
    }
    
    /* With NaN (should be false) */
    if ((nan < a) || (nan > a)) {
        result |= 4;
    }
    
    /* Alternative: !(a == b) && !isunordered(a, b) */
    if (!(a == b) && !isunordered(a, b)) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* Test mixed float/double comparisons */
__attribute__((noinline)) int test_mixed_float_double(void) {
    int result = 0;
    volatile float f1 = 1.5f;
    volatile float f2 = 2.5f;
    volatile double d1 = 1.5;
    volatile double d2 = 2.5;
    volatile float nan_f = make_nan();
    
    /* Float comparisons */
    if (!(f1 < f2)) {  /* UNGE for floats */
        result |= 1;
    }
    
    if (!(f1 <= f2)) {  /* UNGT for floats */
        result |= 2;
    }
    
    /* Mixed type comparisons */
    if ((d1 < f2) || isunordered(d1, f2)) {  /* UNLT with mixed types */
        result |= 4;
    }
    
    if (!(f1 > d2) && !isunordered(f1, d2)) {  /* Complex expression */
        result |= 8;
    }
    
    /* Ternary operator with float comparison */
    result += (f1 != f1) ? 16 : ((f2 > f1) ? 32 : 64);
    
    sink(result);
    return result;
}

/* Test long double comparisons */
__attribute__((noinline)) int test_long_double_comparisons(void) {
    int result = 0;
    volatile long double ld1 = 1.5L;
    volatile long double ld2 = 2.5L;
    volatile long double ld_nan = make_nan();
    
    /* Long double unordered check */
    if (ld_nan != ld_nan) {
        result |= 1;
    }
    
    /* Long double ordered comparisons */
    if (isordered(ld1, ld2)) {
        result |= 2;
    }
    
    /* Long double UNLE */
    if (!(ld1 > ld2)) {
        result |= 4;
    }
    
    /* Long double LTGT */
    if ((ld1 < ld2) || (ld1 > ld2)) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* Test complex nested expressions */
__attribute__((noinline)) int test_complex_expressions(void) {
    int result = 0;
    volatile double x = 1.0;
    volatile double y = 2.0;
    volatile double z = 3.0;
    volatile double nan = make_nan();
    
    /* Nested ternary with multiple comparisons */
    result = (x != x) ? 1 : ((y > z) ? 2 : 3);
    
    /* Complex logical expression */
    if ((!(x < y) || (z > y)) && !isunordered(x, nan)) {
        result += 4;
    }
    
    /* Multiple comparisons combined */
    int temp = 0;
    temp |= (!(x >= y)) ? 0x10 : 0;
    temp |= ((y < z) || isunordered(y, z)) ? 0x20 : 0;
    temp |= (!(x == x) && !isunordered(x, x)) ? 0x40 : 0;
    
    result += temp;
    
    sink(result);
    return result;
}

/* Test with memory operands in structs and arrays */
__attribute__((noinline)) int test_memory_operands(void) {
    int result = 0;
    
    /* Initialize struct members */
    fp1.a = 1.0;
    fp1.b = 2.0;
    fp2.a = 3.0;
    fp2.b = 4.0;
    
    /* Comparisons with struct members */
    if (!(fp1.a < fp2.a)) {  /* UNGE with memory */
        result |= 1;
    }
    
    if ((fp1.b <= fp2.b) || isunordered(fp1.b, fp2.b)) {  /* UNLE with memory */
        result |= 2;
    }
    
    /* Array element comparisons */
    if ((arr_d[0] < arr_d[1]) || (arr_d[0] > arr_d[1])) {  /* LTGT with array */
        result |= 4;
    }
    
    if (!(arr_f[2] >= arr_f[3])) {  /* UNLT with float array */
        result |= 8;
    }
    
    /* Mixed memory and register */
    volatile double reg = 2.5;
    if (!(arr_d[1] <= reg)) {  /* UNGT with mixed */
        result |= 16;
    }
    
    sink(result);
    return result;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize volatile values with potential NaN/Inf */
    vd1 = make_nan();
    vd2 = 3.14;
    vd3 = make_inf();
    
    vf1 = 1.0f / 0.0f;  /* Infinity */
    vf2 = 0.0f / 0.0f;  /* NaN */
    vf3 = 2.5f;
    
    vld1 = 1.5L;
    vld2 = make_nan();
    vld3 = 3.0L;
    
    /* Initialize arrays */
    arr_d[0] = 0.0;
    arr_d[1] = 1.0;
    arr_d[2] = make_nan();
    arr_d[3] = make_inf();
    
    /* Call all test functions */
    checksum += test_unordered();
    checksum += test_ordered();
    checksum += test_uneq();
    checksum += test_unge();
    checksum += test_ungt();
    checksum += test_unle();
    checksum += test_unlt();
    checksum += test_ltgt();
    checksum += test_mixed_float_double();
    checksum += test_long_double_comparisons();
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
