/* test_i386_condition_codes.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent optimization */
extern void sink(int);
static volatile int checksum = 0;

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

/* UNORDERED comparisons */
__attribute__((noinline))
static int test_unordered(void) {
    volatile double nan1 = make_nan();
    volatile double nan2 = make_nan();
    volatile double normal = 3.14;
    int result = 0;
    
    /* Direct unordered checks */
    if (isunordered(nan1, normal)) {
        result |= 1;
    }
    
    /* Using volatile comparison */
    if (nan1 != nan1) {  /* Should be true for NaN */
        result |= 2;
    }
    
    /* Complex expression with unordered */
    volatile float f1 = make_nan();
    volatile float f2 = 2.0f;
    result |= (isunordered(f1, f2) ? 4 : 0);
    
    sink(result);
    checksum += result;
    return result;
}

/* ORDERED comparisons */
__attribute__((noinline))
static int test_ordered(void) {
    volatile double d1 = 1.5;
    volatile double d2 = 2.5;
    volatile double nan = make_nan();
    int result = 0;
    
    if (isordered(d1, d2)) {
        result |= 1;
    }
    
    /* Ordered check via equality */
    if (d1 == d1) {  /* Should be true for non-NaN */
        result |= 2;
    }
    
    /* Mixed ordered/unordered */
    volatile long double ld1 = 3.14159L;
    volatile long double ld2 = make_nan();
    result |= (isordered(ld1, ld2) ? 0 : 4);
    
    sink(result);
    checksum += result;
    return result;
}

/* UNEQ (unordered or equal) */
__attribute__((noinline))
static int test_uneq(void) {
    volatile double a = 5.0;
    volatile double b = 5.0;
    volatile double nan = make_nan();
    int result = 0;
    
    /* a == b */
    if (!(a != b)) {  /* Equivalent to a == b */
        result |= 1;
    }
    
    /* nan == nan (unordered case) */
    if (!(nan != nan)) {  /* Should be true (unordered) */
        result |= 2;
    }
    
    /* Complex UNEQ expression */
    volatile float f1 = 7.0f;
    volatile float f2 = 7.0f;
    result |= ((!(f1 != f2)) ? 4 : 0);
    
    sink(result);
    checksum += result;
    return result;
}

/* UNGE (not less than) = !(a < b) */
__attribute__((noinline))
static int test_unge(void) {
    volatile double x = 3.0;
    volatile double y = 2.0;
    volatile double nan = make_nan();
    int result = 0;
    
    /* Direct: !(x < y) when x >= y */
    if (!(x < y)) {
        result |= 1;
    }
    
    /* With NaN operand */
    if (!(nan < x)) {  /* Should be true (unordered) */
        result |= 2;
    }
    
    /* Complex expression */
    volatile float f1 = 4.0f;
    volatile float f2 = 4.0f;
    result |= ((!(f1 < f2)) ? 4 : 0);
    
    sink(result);
    checksum += result;
    return result;
}

/* UNGT (not less than or equal) = !(a <= b) */
__attribute__((noinline))
static int test_ungt(void) {
    volatile double a = 5.0;
    volatile double b = 4.0;
    volatile double nan = make_nan();
    int result = 0;
    
    /* !(a <= b) when a > b */
    if (!(a <= b)) {
        result |= 1;
    }
    
    /* With NaN */
    if (!(nan <= b)) {  /* Should be true (unordered) */
        result |= 2;
    }
    
    /* Using memory operands */
    volatile double arr[2] = {6.0, 5.0};
    result |= ((!(arr[0] <= arr[1])) ? 4 : 0);
    
    sink(result);
    checksum += result;
    return result;
}

/* UNLE (unordered or less than or equal) */
__attribute__((noinline))
static int test_unle(void) {
    volatile double p = 2.0;
    volatile double q = 3.0;
    volatile double nan = make_nan();
    int result = 0;
    
    /* p <= q */
    if (!(p > q)) {  /* Equivalent to p <= q */
        result |= 1;
    }
    
    /* NaN <= anything is unordered */
    if (!(nan > q)) {  /* Should be true */
        result |= 2;
    }
    
    /* Complex with struct member */
    struct { volatile double x; volatile double y; } s = {1.0, 1.0};
    result |= ((!(s.x > s.y)) ? 4 : 0);
    
    sink(result);
    checksum += result;
    return result;
}

/* UNLT (unordered or less than) */
__attribute__((noinline))
static int test_unlt(void) {
    volatile double u = 1.0;
    volatile double v = 2.0;
    volatile double nan = make_nan();
    int result = 0;
    
    /* u < v */
    if (!(u >= v)) {  /* Equivalent to u < v */
        result |= 1;
    }
    
    /* NaN < anything */
    if (!(nan >= v)) {  /* Should be true (unordered) */
        result |= 2;
    }
    
    /* Nested in ternary */
    volatile float f1 = 0.5f;
    volatile float f2 = 1.5f;
    result |= ((!(f1 >= f2)) ? 4 : 0);
    
    sink(result);
    checksum += result;
    return result;
}

/* LTGT (less than or greater than - not equal and ordered) */
__attribute__((noinline))
static int test_ltgt(void) {
    volatile double m = 3.0;
    volatile double n = 4.0;
    volatile double nan = make_nan();
    int result = 0;
    
    /* m < n || m > n  (when m != n) */
    if ((m < n) || (m > n)) {
        result |= 1;
    }
    
    /* With equal values - should not trigger */
    volatile double e1 = 5.0;
    volatile double e2 = 5.0;
    if (!((e1 < e2) || (e1 > e2))) {
        result |= 2;
    }
    
    /* With NaN - should be false */
    if (!((nan < m) || (nan > m))) {
        result |= 4;
    }
    
    sink(result);
    checksum += result;
    return result;
}

/* Functions using inline assembly to force condition code usage */
__attribute__((noinline))
static int test_asm_unordered(void) {
    volatile double a = make_nan();
    volatile double b = 1.0;
    int result = 0;
    
    /* Force unordered check with inline asm goto */
    if (isunordered(a, b)) {
        __asm__ goto (
            "j%c0 %l[label]\n"
            : /* no outputs */
            : "i" (0)  /* condition code will be filled */
            : /* no clobbers */
            : label
        );
        return 0;
    label:
        result = 1;
    }
    
    checksum += result;
    return result;
}

__attribute__((noinline))
static int test_asm_ordered(void) {
    volatile double x = 2.0;
    volatile double y = 3.0;
    int result = 0;
    
    if (isordered(x, y)) {
        __asm__ goto (
            "j%c0 %l[label]\n"
            : /* no outputs */
            : "i" (0)
            : /* no clobbers */
            : label
        );
        return 0;
    label:
        result = 1;
    }
    
    checksum += result;
    return result;
}

/* Complex expression combining multiple condition codes */
__attribute__((noinline))
static int test_complex_expression(void) {
    volatile double v1 = make_nan();
    volatile double v2 = 10.0;
    volatile double v3 = 20.0;
    volatile double v4 = 10.0;
    
    int r = 0;
    
    /* Complex nested condition */
    r = (v1 != v1) ? 1 : 
        ((v2 < v3) ? 2 : 
         ((!(v3 >= v4)) ? 3 : 
          (((v2 < v3) || (v2 > v3)) ? 4 : 5)));
    
    /* Another complex check */
    if ((!(v2 > v3)) && (isunordered(v1, v2) || !(v3 <= v4))) {
        r |= 8;
    }
    
    sink(r);
    checksum += r;
    return r;
}

/* Test with various operand types and memory locations */
__attribute__((noinline))
static int test_mixed_operands(void) {
    volatile float fa[4] = {make_nan(), 1.0f, 2.0f, 3.0f};
    volatile double db[3] = {4.0, make_nan(), 6.0};
    volatile long double ld[2] = {7.0L, make_nan()};
    
    int result = 0;
    
    /* Mixed type comparisons */
    if (isunordered(fa[0], fa[1])) result |= 1;
    if (!(db[0] < db[2])) result |= 2;
    if ((fa[2] < fa[3]) || (fa[2] > fa[3])) result |= 4;
    if (!(ld[0] >= ld[1])) result |= 8;
    
    /* Struct with mixed types */
    struct Mixed {
        volatile float f;
        volatile double d;
        volatile long double ld;
    } m = {make_nan(), 5.0, make_nan()};
    
    if (isunordered(m.f, m.d)) result |= 16;
    if (!(m.d <= m.ld)) result |= 32;
    
    sink(result);
    checksum += result;
    return result;
}

int main(void) {
    printf("Testing i386 condition codes...\n");
    
    /* Initialize volatile seed */
    volatile int seed = 42;
    checksum = seed;
    
    /* Call all test functions */
    test_unordered();
    test_ordered();
    test_uneq();
    test_unge();
    test_ungt();
    test_unle();
    test_unlt();
    test_ltgt();
    
    /* Assembly tests */
    test_asm_unordered();
    test_asm_ordered();
    
    /* Complex tests */
    test_complex_expression();
    test_mixed_operands();
    
    /* Print checksum to ensure all code is live */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
