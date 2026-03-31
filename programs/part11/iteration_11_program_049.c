/* test_i386_condition_codes.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent optimization */
extern void sink(int);
static volatile int checksum = 0;

/* Helper to create NaN values */
static double make_nan(void) {
    volatile double zero = 0.0;
    return zero / zero;
}

static float make_nanf(void) {
    volatile float zero = 0.0f;
    return zero / zero;
}

static long double make_nanl(void) {
    volatile long double zero = 0.0L;
    return zero / zero;
}

/* Pattern 1: UNORDERED comparisons */
__attribute__((noinline))
static int test_unordered(void) {
    volatile double d1 = make_nan();
    volatile double d2 = 3.14;
    volatile float f1 = make_nanf();
    volatile float f2 = 2.71f;
    volatile long double ld1 = make_nanl();
    volatile long double ld2 = 4.2L;
    
    int result = 0;
    
    /* Direct unordered checks */
    if (isunordered(d1, d2)) {
        result |= 1;
    }
    
    if (isunordered(f1, f2)) {
        result |= 2;
    }
    
    if (isunordered(ld1, ld2)) {
        result |= 4;
    }
    
    /* NaN self-comparison for unordered */
    if (d1 != d1) {
        result |= 8;
    }
    
    /* Inline assembly to force condition code use */
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : unordered_label
    );
    
    result |= 16;
    goto skip_unordered;
    
unordered_label:
    result |= 32;
    
skip_unordered:
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 2: ORDERED comparisons */
__attribute__((noinline))
static int test_ordered(void) {
    volatile double d1 = 1.0;
    volatile double d2 = 2.0;
    volatile double d_nan = make_nan();
    
    int result = 0;
    
    if (isordered(d1, d2)) {
        result |= 1;
    }
    
    if (isordered(d1, d_nan)) {
        result |= 2;
    }
    
    /* Normal comparison should be ordered */
    if (d1 == d1) {
        result |= 4;
    }
    
    /* Force ordered condition code */
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : ordered_label
    );
    
    result |= 8;
    goto skip_ordered;
    
ordered_label:
    result |= 16;
    
skip_ordered:
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 3: UNEQ (unordered or equal) */
__attribute__((noinline))
static int test_uneq(void) {
    volatile double a = 5.0;
    volatile double b = 5.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    /* UNEQ: !(a != b) which is a == b or unordered */
    if (!(a != b)) {
        result |= 1;
    }
    
    /* NaN comparison should also trigger UNEQ */
    if (!(nan != nan)) {
        result |= 2;
    }
    
    /* Complex expression */
    result |= ((a == b) || (nan != nan)) ? 4 : 0;
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 4: UNGE (not less than) */
__attribute__((noinline))
static int test_unge(void) {
    volatile double x = 7.0;
    volatile double y = 3.0;
    volatile double z = 7.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    /* UNGE: !(x < y) */
    if (!(x < y)) {
        result |= 1;
    }
    
    /* Equal case */
    if (!(x < z)) {
        result |= 2;
    }
    
    /* With NaN */
    if (!(nan < y)) {
        result |= 4;
    }
    
    /* Complex: (x >= y) using negation */
    result |= (!(x < y) && !(y > x)) ? 8 : 0;
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 5: UNGT (not less than or equal) */
__attribute__((noinline))
static int test_ungt(void) {
    volatile double p = 10.0;
    volatile double q = 5.0;
    volatile double r = 10.0;
    
    int result = 0;
    
    /* UNGT: !(p <= q) */
    if (!(p <= q)) {
        result |= 1;
    }
    
    /* Equal case should not trigger */
    if (!(p <= r)) {
        result |= 2;
    }
    
    /* Using > directly */
    if (p > q) {
        result |= 4;
    }
    
    /* Complex: !(p <= q) && (p != q) */
    result |= (!(p <= q) && (p != q)) ? 8 : 0;
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 6: UNLE (unordered or less than or equal) */
__attribute__((noinline))
static int test_unle(void) {
    volatile double m = 2.0;
    volatile double n = 8.0;
    volatile double o = 8.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    /* UNLE: !(m > n) */
    if (!(m > n)) {
        result |= 1;
    }
    
    /* Equal case */
    if (!(o > n)) {
        result |= 2;
    }
    
    /* With NaN */
    if (!(nan > n)) {
        result |= 4;
    }
    
    /* Using <= operator */
    if (m <= n) {
        result |= 8;
    }
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 7: UNLT (unordered or less than) */
__attribute__((noinline))
static int test_unlt(void) {
    volatile double u = 3.0;
    volatile double v = 9.0;
    volatile double w = 9.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    /* UNLT: !(u >= v) */
    if (!(u >= v)) {
        result |= 1;
    }
    
    /* Equal case should not trigger */
    if (!(w >= v)) {
        result |= 2;
    }
    
    /* With NaN */
    if (!(nan >= v)) {
        result |= 4;
    }
    
    /* Using < operator */
    if (u < v) {
        result |= 8;
    }
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 8: LTGT (not equal and ordered) */
__attribute__((noinline))
static int test_ltgt(void) {
    volatile double a = 4.0;
    volatile double b = 6.0;
    volatile double c = 6.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    /* LTGT: (a < b) || (a > b) */
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    /* Equal case should not trigger */
    if ((c < b) || (c > b)) {
        result |= 2;
    }
    
    /* With NaN should not trigger */
    if ((nan < b) || (nan > b)) {
        result |= 4;
    }
    
    /* Alternative: !(a == b) && !isunordered(a, b) */
    if (!(a == b) && !isunordered(a, b)) {
        result |= 8;
    }
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 9: Mixed types in arrays */
__attribute__((noinline))
static int test_array_comparisons(void) {
    volatile double arr_d[4] = {1.0, make_nan(), 3.0, 4.0};
    volatile float arr_f[4] = {1.0f, make_nanf(), 3.0f, 4.0f};
    
    int result = 0;
    
    /* Compare array elements */
    if (isunordered(arr_d[0], arr_d[1])) {
        result |= 1;
    }
    
    if (!(arr_d[2] < arr_d[3])) {
        result |= 2;
    }
    
    if ((arr_f[0] < arr_f[2]) || (arr_f[0] > arr_f[2])) {
        result |= 4;
    }
    
    /* Complex ternary with array access */
    result |= (arr_d[1] != arr_d[1]) ? 8 : 
              ((arr_d[2] > arr_d[0]) ? 16 : 32);
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 10: Struct with floating point members */
__attribute__((noinline))
static int test_struct_comparisons(void) {
    struct fp_pair {
        volatile double x;
        volatile double y;
    };
    
    struct fp_pair p1 = {1.5, make_nan()};
    struct fp_pair p2 = {2.5, 2.5};
    
    int result = 0;
    
    /* Compare struct members */
    if (isunordered(p1.x, p1.y)) {
        result |= 1;
    }
    
    if (!(p1.x >= p2.x)) {
        result |= 2;
    }
    
    if (!(p2.y <= p2.x)) {
        result |= 4;
    }
    
    /* Nested comparisons */
    result |= ((p1.x != p1.x) || (p2.x == p2.y)) ? 8 : 16;
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 11: Complex nested expressions */
__attribute__((noinline))
static int test_nested_expressions(void) {
    volatile double v1 = 2.0;
    volatile double v2 = make_nan();
    volatile double v3 = 5.0;
    volatile double v4 = 5.0;
    
    int result = 0;
    
    /* Deeply nested ternary */
    result = (v1 != v1) ? 1 : 
             ((v3 > v1) ? 2 : 
              ((!(v4 >= v3)) ? 3 : 
               (((v2 < v3) || (v2 > v3)) ? 4 : 5)));
    
    /* Logical combination */
    if ((!(v1 < v3) && (v3 != v4)) || isunordered(v2, v1)) {
        result |= 8;
    }
    
    /* Multiple comparisons in one expression */
    int temp = ((v1 == v1) << 0) |
               ((!(v3 <= v1)) << 1) |
               ((!(v1 >= v3)) << 2) |
               (((v3 < v4) || (v3 > v4)) << 3);
    
    result |= temp;
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 12: Inline assembly with condition codes */
__attribute__((noinline))
static int test_asm_condition_codes(void) {
    volatile double a = 3.0;
    volatile double b = 7.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    /* Test various condition codes via inline asm */
    
    /* For UNORDERED */
    __asm__ volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "fstp %%st(0)\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "m" (nan), "m" (a)
        : "st", "st(1)", "cc", "al"
    );
    
    checksum += result;
    
    /* For ORDERED */
    int result2;
    __asm__ volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "fstp %%st(0)\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result2)
        : "m" (a), "m" (b)
        : "st", "st(1)", "cc", "al"
    );
    
    checksum += result2;
    
    /* Combine results */
    return result | (result2 << 4);
}

int main(void) {
    printf("Testing i386 condition codes...\n");
    
    /* Initialize checksum */
    checksum = 0;
    
    /* Run all test patterns */
    int r1 = test_unordered();
    int r2 = test_ordered();
    int r3 = test_uneq();
    int r4 = test_unge();
    int r5 = test_ungt();
    int r6 = test_unle();
    int r7 = test_unlt();
    int r8 = test_ltgt();
    int r9 = test_array_comparisons();
    int r10 = test_struct_comparisons();
    int r11 = test_nested_expressions();
    int r12 = test_asm_condition_codes();
    
    /* Print results to ensure all code is live */
    printf("Results: %d %d %d %d %d %d %d %d %d %d %d %d\n",
           r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12);
    
    /* Final checksum to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
