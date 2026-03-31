/* test_i386_condition_codes.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent optimization */
extern void sink(int);
extern void sink_ptr(void*);

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

/* UNORDERED condition - using NaN comparisons */
__attribute__((noinline))
int test_unordered(void) {
    volatile double nan1 = make_nan();
    volatile double nan2 = make_nan();
    volatile double normal = 3.14;
    int result = 0;
    
    /* Direct NaN comparison */
    if (nan1 != nan1) {
        result |= 1;
    }
    
    /* Using isunordered */
    if (isunordered(nan1, normal)) {
        result |= 2;
    }
    
    /* Complex expression with unordered */
    if (isunordered(nan1, nan2) || (normal > 0)) {
        result |= 4;
    }
    
    /* Force condition code use with inline asm */
    volatile double a = nan1;
    volatile double b = normal;
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l[unordered_label]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
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

/* ORDERED condition */
__attribute__((noinline))
int test_ordered(void) {
    volatile double normal1 = 1.5;
    volatile double normal2 = 2.5;
    volatile double nan = make_nan();
    int result = 0;
    
    /* Direct ordered comparison */
    if (normal1 == normal1) {
        result |= 1;
    }
    
    /* Using isordered */
    if (isordered(normal1, normal2)) {
        result |= 2;
    }
    
    /* Complex ordered check */
    if (isordered(normal1, normal2) && !isunordered(normal1, nan)) {
        result |= 4;
    }
    
    /* Inline asm with ordered condition */
    volatile double a = normal1;
    volatile double b = normal2;
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l[ordered_label]\n\t"
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
    return result;
}

/* UNEQ (unordered or equal) */
__attribute__((noinline))
int test_uneq(void) {
    volatile double a = 1.0;
    volatile double b = 1.0;
    volatile double nan = make_nan();
    int result = 0;
    
    /* Generate UNEQ: !(a != b) which becomes UNEQ */
    if (!(a != b)) {
        result |= 1;
    }
    
    /* Another UNEQ pattern */
    if (a == b) {
        result |= 2;
    }
    
    /* With NaN - should be unordered */
    if (!(nan != nan)) {  /* NaN != NaN is true, so !(true) is false */
        result |= 4;      /* This branch shouldn't be taken */
    }
    
    /* Complex expression */
    volatile double x = 5.0;
    volatile double y = 5.0;
    if ((x == y) || isunordered(x, y)) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* UNGE (not less than: !(a < b)) */
__attribute__((noinline))
int test_unge(void) {
    volatile double a = 3.0;
    volatile double b = 2.0;
    volatile double c = 3.0;
    volatile double nan = make_nan();
    int result = 0;
    
    /* Direct UNGE: !(a < b) */
    if (!(a < b)) {
        result |= 1;
    }
    
    /* Equal case: !(c < c) should be true */
    if (!(c < c)) {
        result |= 2;
    }
    
    /* With NaN: !(nan < b) - should be true (unordered) */
    if (!(nan < b)) {
        result |= 4;
    }
    
    /* Complex: !(a < b) && (a > 0) */
    if (!(a < b) && (a > 0)) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* UNGT (not less than or equal: !(a <= b)) */
__attribute__((noinline))
int test_ungt(void) {
    volatile double a = 3.0;
    volatile double b = 2.0;
    volatile double c = 3.0;
    int result = 0;
    
    /* Direct UNGT: !(a <= b) */
    if (!(a <= b)) {
        result |= 1;
    }
    
    /* Equal case: !(c <= c) should be false */
    if (!(c <= c)) {
        result |= 2;
    }
    
    /* Another pattern */
    if (a > b) {  /* Equivalent to !(a <= b) for ordered values */
        result |= 4;
    }
    
    /* Complex expression */
    volatile double x = 10.0;
    volatile double y = 5.0;
    if (!(x <= y) && (x != y)) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* UNLE (unordered or less than or equal) */
__attribute__((noinline))
int test_unle(void) {
    volatile double a = 2.0;
    volatile double b = 3.0;
    volatile double c = 3.0;
    volatile double nan = make_nan();
    int result = 0;
    
    /* Direct UNLE: !(a > b) */
    if (!(a > b)) {
        result |= 1;
    }
    
    /* Equal case: !(c > c) should be true */
    if (!(c > c)) {
        result |= 2;
    }
    
    /* With NaN: !(nan > b) should be true */
    if (!(nan > b)) {
        result |= 4;
    }
    
    /* Complex: (a <= b) || isunordered(a, b) */
    if ((a <= b) || isunordered(a, nan)) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* UNLT (unordered or less than) */
__attribute__((noinline))
int test_unlt(void) {
    volatile double a = 2.0;
    volatile double b = 3.0;
    volatile double c = 3.0;
    volatile double nan = make_nan();
    int result = 0;
    
    /* Direct UNLT: !(a >= b) */
    if (!(a >= b)) {
        result |= 1;
    }
    
    /* Equal case: !(c >= c) should be false for ordered */
    if (!(c >= c)) {
        result |= 2;
    }
    
    /* With NaN: !(nan >= b) should be true */
    if (!(nan >= b)) {
        result |= 4;
    }
    
    /* Complex: (a < b) || isunordered(a, b) */
    if ((a < b) || isunordered(a, nan)) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* LTGT (less than or greater than - not equal and ordered) */
__attribute__((noinline))
int test_ltgt(void) {
    volatile double a = 2.0;
    volatile double b = 3.0;
    volatile double c = 3.0;
    volatile double nan = make_nan();
    int result = 0;
    
    /* Direct LTGT: (a < b) || (a > b) */
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    /* Equal case: (c < c) || (c > c) should be false */
    if ((c < c) || (c > c)) {
        result |= 2;
    }
    
    /* With NaN: (nan < b) || (nan > b) should be false */
    if ((nan < b) || (nan > b)) {
        result |= 4;
    }
    
    /* Alternative: a != b && !isunordered(a, b) */
    if ((a != b) && !isunordered(a, b)) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* Test with different floating-point types */
__attribute__((noinline))
int test_mixed_types(void) {
    volatile float f1 = 1.5f;
    volatile float f2 = 2.5f;
    volatile double d1 = 3.14;
    volatile double d2 = 2.71;
    volatile long double ld1 = 1.23456789L;
    volatile long double ld2 = 9.87654321L;
    int result = 0;
    
    /* Float comparisons */
    if (!(f1 < f2)) {  /* UNGE */
        result |= 1;
    }
    
    /* Double comparisons */
    if (!(d1 > d2)) {  /* UNLE */
        result |= 2;
    }
    
    /* Long double comparisons */
    if ((ld1 < ld2) || (ld1 > ld2)) {  /* LTGT */
        result |= 4;
    }
    
    /* Mixed in complex expression */
    result = (f1 != f1) ? (result | 8) : 
             ((d1 < d2) ? (result | 16) : (result | 32));
    
    sink(result);
    return result;
}

/* Test with memory operands */
__attribute__((noinline))
int test_memory_operands(void) {
    volatile double arr[4] = {1.0, 2.0, 3.0, make_nan()};
    volatile struct {
        double x;
        double y;
    } point = {1.5, 2.5};
    
    int result = 0;
    
    /* Array element comparisons */
    if (!(arr[0] < arr[1])) {  /* UNGE */
        result |= 1;
    }
    
    if (isunordered(arr[0], arr[3])) {  /* UNORDERED */
        result |= 2;
    }
    
    /* Struct member comparisons */
    if (!(point.x > point.y)) {  /* UNLE */
        result |= 4;
    }
    
    /* Complex addressing */
    volatile double* ptr = &arr[2];
    if ((*ptr < arr[1]) || (*ptr > arr[1])) {  /* LTGT */
        result |= 8;
    }
    
    sink(result);
    sink_ptr(&arr);
    sink_ptr(&point);
    return result;
}

/* Main function with checksum */
int main(void) {
    int checksum = 0;
    
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
