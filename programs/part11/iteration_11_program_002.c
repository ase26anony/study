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
volatile long double ldarr[4] = {0.0L, 1.0L, 2.0L, 3.0L};

/* Struct with volatile members */
struct volatile_floats {
    volatile float f;
    volatile double d;
    volatile long double ld;
};
volatile struct volatile_floats vf = {0.0f, 0.0, 0.0L};

/* Pattern 1: UNORDERED comparisons */
__attribute__((noinline))
int test_unordered(void) {
    volatile double d1 = g_nan;
    volatile double d2 = g_one;
    volatile float f1 = farr[0];
    f1 = g_nan;
    
    int result = 0;
    
    /* Direct NaN comparison for unordered */
    if (d1 != d1) {
        result |= 1;  /* unordered */
    }
    
    /* Using isunordered() */
    if (isunordered(d1, d2)) {
        result |= 2;
    }
    
    /* Complex expression with unordered */
    result |= (isunordered(f1, farr[1]) ? 4 : 0);
    
    /* Memory operand unordered check */
    if (darr[0] != darr[0]) {
        result |= 8;
    }
    
    /* Struct member unordered */
    if (vf.d != vf.d) {
        result |= 16;
    }
    
    sink(result);
    return result;
}

/* Pattern 2: ORDERED comparisons */
__attribute__((noinline))
int test_ordered(void) {
    volatile double d1 = g_one;
    volatile double d2 = g_two;
    volatile double d3 = g_nan;
    
    int result = 0;
    
    /* Direct ordered comparison */
    if (d1 == d1) {
        result |= 1;  /* ordered */
    }
    
    /* Using isordered() */
    if (isordered(d1, d2)) {
        result |= 2;
    }
    
    /* Ordered with NaN operand */
    if (!isordered(d1, d3)) {
        result |= 4;
    }
    
    /* Complex ordered expression */
    result |= (isordered(darr[1], darr[2]) ? 8 : 0);
    
    /* Long double ordered */
    volatile long double ld1 = ldarr[0];
    if (ld1 == ld1) {
        result |= 16;
    }
    
    sink(result);
    return result;
}

/* Pattern 3: UNEQ (unordered or equal) */
__attribute__((noinline))
int test_uneq(void) {
    volatile double a = g_one;
    volatile double b = g_one;
    volatile double c = g_nan;
    
    int result = 0;
    
    /* Direct unordered or equal */
    if (!(a > b) && !(a < b)) {  /* a == b or unordered */
        result |= 1;
    }
    
    /* Using inline asm to force condition code */
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "jne %l[not_equal]\n\t"
        "jp %l[unordered]\n\t"
        : : : "cc", "st", "st(1)" : equal, not_equal, unordered);
    
equal:
    result |= 2;
    goto done;
    
unordered:
    result |= 4;
    goto done;
    
not_equal:
    /* Not equal and ordered */
    result |= 8;
    
done:
    /* Complex UNEQ expression */
    volatile double x = darr[0];
    volatile double y = darr[1];
    if (!(x > y) && !(x < y)) {
        result |= 16;
    }
    
    sink(result);
    return result;
}

/* Pattern 4: UNGE (not less than) = !(a < b) */
__attribute__((noinline))
int test_unge(void) {
    volatile double a = g_two;
    volatile double b = g_one;
    volatile double c = g_nan;
    
    int result = 0;
    
    /* Direct !(a < b) */
    if (!(a < b)) {
        result |= 1;
    }
    
    /* UNGE with NaN */
    if (!(c < b)) {
        result |= 2;
    }
    
    /* Complex expression generating UNGE */
    result |= (!(darr[2] < darr[1]) ? 4 : 0);
    
    /* Using inline asm with condition code */
    volatile double x = vf.d;
    volatile double y = g_one;
    __asm__ goto (
        "comisd %1, %0\n\t"
        "jnb %l[not_below]\n\t"
        : : "x"(x), "x"(y) : "cc" : not_below, below);
    
below:
    result |= 8;
    goto done2;
    
not_below:  /* UNGE: not below = greater or equal or unordered */
    result |= 16;
    
done2:
    sink(result);
    return result;
}

/* Pattern 5: UNGT (not less or equal) = !(a <= b) */
__attribute__((noinline))
int test_ungt(void) {
    volatile double a = g_two;
    volatile double b = g_one;
    
    int result = 0;
    
    /* Direct !(a <= b) */
    if (!(a <= b)) {
        result |= 1;
    }
    
    /* Using > for UNGT */
    if (a > b) {
        result |= 2;
    }
    
    /* Complex !( <= ) expression */
    volatile double x = darr[3];
    volatile double y = darr[2];
    result |= (!(x <= y) ? 4 : 0);
    
    /* Nested ternary with UNGT */
    int r = (x > y) ? 8 : ((x <= y) ? 0 : 16);
    result |= r;
    
    sink(result);
    return result;
}

/* Pattern 6: UNLE (unordered or less or equal) */
__attribute__((noinline))
int test_unle(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    
    int result = 0;
    
    /* Direct !(a > b) */
    if (!(a > b)) {
        result |= 1;
    }
    
    /* UNLE with NaN */
    if (!(c > b)) {
        result |= 2;
    }
    
    /* Complex UNLE expression */
    volatile float f1 = farr[0];
    volatile float f2 = farr[3];
    if (!(f1 > f2)) {
        result |= 4;
    }
    
    /* Using inline asm */
    volatile double x = g_one;
    volatile double y = g_two;
    __asm__ goto (
        "ucomisd %1, %0\n\t"
        "jna %l[not_above]\n\t"
        : : "x"(x), "x"(y) : "cc" : not_above, above);
    
above:
    result |= 8;
    goto done3;
    
not_above:  /* UNLE: not above = less or equal or unordered */
    result |= 16;
    
done3:
    sink(result);
    return result;
}

/* Pattern 7: UNLT (unordered or less than) */
__attribute__((noinline))
int test_unlt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    
    int result = 0;
    
    /* Direct !(a >= b) */
    if (!(a >= b)) {
        result |= 1;
    }
    
    /* Using < for UNLT when unordered possible */
    volatile double c = g_nan;
    if (c < b) {  /* false for NaN, but may generate UNLT */
        result |= 2;
    }
    
    /* Complex !( >= ) with memory */
    volatile double x = darr[0];
    volatile double y = darr[3];
    result |= (!(x >= y) ? 4 : 0);
    
    /* Nested comparisons */
    int r = (x < y) ? 8 : ((x >= y) ? 0 : 16);
    result |= r;
    
    sink(result);
    return result;
}

/* Pattern 8: LTGT (not equal and ordered) */
__attribute__((noinline))
int test_ltgt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    
    int result = 0;
    
    /* Direct (a < b) || (a > b) */
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    /* LTGT with != for ordered values */
    if (a != b) {
        result |= 2;
    }
    
    /* LTGT should be false for NaN */
    if ((c < b) || (c > b)) {
        result |= 4;  /* Should not be taken */
    }
    
    /* Complex LTGT expression */
    volatile double x = vf.d;
    volatile double y = g_one;
    vf.d = g_two;
    result |= (((x < y) || (x > y)) ? 8 : 0);
    
    /* Using inline asm to check ordered not equal */
    __asm__ goto (
        "fucomi %%st(1), %%st(0)\n\t"
        "jne %l[not_equal2]\n\t"
        "jp %l[unordered2]\n\t"
        : : : "cc", "st", "st(1)" : equal2, not_equal2, unordered2);
    
equal2:
    result |= 16;
    goto done4;
    
unordered2:
    result |= 32;
    goto done4;
    
not_equal2:  /* LTGT: not equal and ordered */
    result |= 64;
    
done4:
    sink(result);
    return result;
}

/* Pattern 9: Mixed condition codes in complex expression */
__attribute__((noinline))
int test_mixed(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_two;
    
    int result = 0;
    
    /* Complex nested ternary with multiple condition codes */
    result = (a != a) ? 1 : 
             ((b < c) ? 2 : 
             ((b > c) ? 3 : 
             ((b == c) ? 4 : 
             (!(b < c) ? 5 : 
             (!(b > c) ? 6 : 7)))));
    
    /* Logical AND/OR of comparisons */
    if ((b < c) && (c > b) && !(a == a)) {
        result |= 8;
    }
    
    /* Memory operands with mixed types */
    volatile float* fp = (volatile float*)&farr[0];
    volatile double* dp = (volatile double*)&darr[0];
    
    if ((*fp < farr[1]) || (*dp > darr[2])) {
        result |= 16;
    }
    
    /* Struct member comparisons */
    if (!(vf.f < 1.0f) || (vf.d > 0.0)) {
        result |= 32;
    }
    
    sink(result);
    return result;
}

/* Pattern 10: Long double condition codes */
__attribute__((noinline))
int test_long_double(void) {
    volatile long double a = 0.0L / 0.0L;  /* NaN */
    volatile long double b = 1.0L;
    volatile long double c = 2.0L;
    
    int result = 0;
    
    /* Long double unordered */
    if (a != a) {
        result |= 1;
    }
    
    /* Long double ordered */
    if (b == b) {
        result |= 2;
    }
    
    /* Long double comparisons */
    if (b < c) {
        result |= 4;
    }
    
    if (!(b > c)) {
        result |= 8;
    }
    
    /* Long double from array */
    if (ldarr[0] != ldarr[1]) {
        result |= 16;
    }
    
    sink(result);
    return result;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize volatile struct */
    vf.f = 0.0f;
    vf.d = 1.0;
    vf.ld = 2.0L;
    
    /* Initialize arrays with potential NaN */
    darr[0] = g_nan;
    farr[0] = g_nan;
    
    /* Call all test functions */
    checksum += test_unordered();
    checksum += test_ordered();
    checksum += test_uneq();
    checksum += test_unge();
    checksum += test_ungt();
    checksum += test_unle();
    checksum += test_unlt();
    checksum += test_ltgt();
    checksum += test_mixed();
    checksum += test_long_double();
    
    /* Print checksum to ensure all code is live */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
