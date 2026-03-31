/* test_i386_condition_codes.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent optimization */
extern void sink(int);
extern void sink_ptr(void*);

/* Helper to generate NaN */
static inline double make_nan(void) {
    volatile double zero = 0.0;
    return zero / zero;
}

/* Helper to generate infinity */
static inline double make_inf(void) {
    volatile double one = 1.0;
    volatile double zero = 0.0;
    return one / zero;
}

/* UNORDERED condition */
__attribute__((noinline))
int test_unordered(double a, double b) {
    volatile double x = a;
    volatile double y = b;
    int result = 0;
    
    /* Using isunordered() */
    if (isunordered(x, y)) {
        result |= 1;
    }
    
    /* Direct NaN comparison */
    if (x != x || y != y) {
        result |= 2;
    }
    
    /* Inline asm to force condition code */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l[unordered_label]\n\t"
        : : : "cc" : unordered_label);
    
    return result;
    
unordered_label:
    result |= 4;
    return result;
}

/* ORDERED condition */
__attribute__((noinline))
int test_ordered(double a, double b) {
    volatile double x = a;
    volatile double y = b;
    int result = 0;
    
    /* Using isordered() */
    if (isordered(x, y)) {
        result |= 1;
    }
    
    /* Direct comparison */
    if (x == x && y == y) {
        result |= 2;
    }
    
    /* Inline asm with ordered condition */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l[ordered_label]\n\t"
        : : : "cc" : ordered_label);
    
    return result;
    
ordered_label:
    result |= 4;
    return result;
}

/* UNEQ (unordered or equal) */
__attribute__((noinline))
int test_uneq(double a, double b) {
    volatile double x = a;
    volatile double y = b;
    int result = 0;
    
    /* Generate UNEQ: !(a < b) && !(a > b) */
    if (!(x < y) && !(x > y)) {
        result |= 1;
    }
    
    /* Alternative: isunordered(x, y) || x == y */
    if (isunordered(x, y) || x == y) {
        result |= 2;
    }
    
    /* Inline asm */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l[uneq_label]\n\t"
        : : : "cc" : uneq_label);
    
    return result;
    
uneq_label:
    result |= 4;
    return result;
}

/* UNGE (not less than) */
__attribute__((noinline))
int test_unge(double a, double b) {
    volatile double x = a;
    volatile double y = b;
    int result = 0;
    
    /* Generate UNGE: !(a < b) */
    if (!(x < y)) {
        result |= 1;
    }
    
    /* Alternative: isunordered(x, y) || x >= y */
    if (isunordered(x, y) || x >= y) {
        result |= 2;
    }
    
    /* Inline asm */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l[unge_label]\n\t"
        : : : "cc" : unge_label);
    
    return result;
    
unge_label:
    result |= 4;
    return result;
}

/* UNGT (not less than or equal) */
__attribute__((noinline))
int test_ungt(double a, double b) {
    volatile double x = a;
    volatile double y = b;
    int result = 0;
    
    /* Generate UNGT: !(a <= b) */
    if (!(x <= y)) {
        result |= 1;
    }
    
    /* Alternative: isunordered(x, y) || x > y */
    if (isunordered(x, y) || x > y) {
        result |= 2;
    }
    
    /* Inline asm */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l[ungt_label]\n\t"
        : : : "cc" : ungt_label);
    
    return result;
    
ungt_label:
    result |= 4;
    return result;
}

/* UNLE (unordered or less than or equal) */
__attribute__((noinline))
int test_unle(double a, double b) {
    volatile double x = a;
    volatile double y = b;
    int result = 0;
    
    /* Generate UNLE: !(a > b) */
    if (!(x > y)) {
        result |= 1;
    }
    
    /* Alternative: isunordered(x, y) || x <= y */
    if (isunordered(x, y) || x <= y) {
        result |= 2;
    }
    
    /* Inline asm */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l[unle_label]\n\t"
        : : : "cc" : unle_label);
    
    return result;
    
unle_label:
    result |= 4;
    return result;
}

/* UNLT (unordered or less than) */
__attribute__((noinline))
int test_unlt(double a, double b) {
    volatile double x = a;
    volatile double y = b;
    int result = 0;
    
    /* Generate UNLT: !(a >= b) */
    if (!(x >= y)) {
        result |= 1;
    }
    
    /* Alternative: isunordered(x, y) || x < y */
    if (isunordered(x, y) || x < y) {
        result |= 2;
    }
    
    /* Inline asm */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l[unlt_label]\n\t"
        : : : "cc" : unlt_label);
    
    return result;
    
unlt_label:
    result |= 4;
    return result;
}

/* LTGT (less than or greater than - ordered and not equal) */
__attribute__((noinline))
int test_ltgt(double a, double b) {
    volatile double x = a;
    volatile double y = b;
    int result = 0;
    
    /* Generate LTGT: (a < b) || (a > b) */
    if ((x < y) || (x > y)) {
        result |= 1;
    }
    
    /* Alternative: isordered(x, y) && x != y */
    if (isordered(x, y) && x != y) {
        result |= 2;
    }
    
    /* Inline asm */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l[ltgt_label]\n\t"
        : : : "cc" : ltgt_label);
    
    return result;
    
ltgt_label:
    result |= 4;
    return result;
}

/* Complex expression combining multiple condition codes */
__attribute__((noinline))
int test_complex_expr(double a, double b, double c) {
    volatile double x = a;
    volatile double y = b;
    volatile double z = c;
    int result = 0;
    
    /* Nested ternary with different condition codes */
    result = (x != x) ? 1 : 
             ((y > z) ? 2 : 
             ((!(x < y)) ? 3 : 
             ((isunordered(y, z)) ? 4 : 5)));
    
    /* Complex logical expression */
    if ((!(x <= y) && isordered(x, z)) || 
        (isunordered(y, z) && !(x >= y))) {
        result += 10;
    }
    
    return result;
}

/* Test with different floating-point types */
__attribute__((noinline))
int test_mixed_types(float a, double b, long double c) {
    volatile float f = a;
    volatile double d = b;
    volatile long double ld = c;
    int result = 0;
    
    /* Mix types in comparisons */
    if (isunordered(f, d)) result |= 1;
    if (!(ld < (long double)d)) result |= 2;
    if ((double)f != d) result |= 4;
    if (isordered((double)ld, d)) result |= 8;
    
    return result;
}

/* Test with memory operands */
__attribute__((noinline))
int test_memory_operands(void) {
    volatile double arr[4] = {1.0, make_nan(), 3.0, make_inf()};
    volatile struct {
        double a;
        double b;
    } s = {make_nan(), 2.0};
    
    int result = 0;
    
    /* Array accesses */
    if (isunordered(arr[0], arr[1])) result |= 1;
    if (!(arr[2] < arr[3])) result |= 2;
    
    /* Struct member accesses */
    if (isunordered(s.a, s.b)) result |= 4;
    if (!(s.b >= s.a)) result |= 8;
    
    sink_ptr((void*)arr);
    sink_ptr((void*)&s);
    
    return result;
}

/* Main test driver */
int main(void) {
    volatile int checksum = 0;
    
    /* Generate various floating-point values */
    volatile double nan_val = make_nan();
    volatile double inf_val = make_inf();
    volatile double neg_inf = -inf_val;
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    
    /* Test each condition code pattern */
    checksum += test_unordered(nan_val, normal1);
    checksum += test_unordered(normal1, nan_val);
    checksum += test_unordered(nan_val, nan_val);
    
    checksum += test_ordered(normal1, normal2);
    checksum += test_ordered(normal1, inf_val);
    checksum += test_ordered(zero, zero);
    
    checksum += test_uneq(nan_val, normal1);
    checksum += test_uneq(normal1, normal1);
    checksum += test_uneq(normal1, normal2);
    
    checksum += test_unge(normal1, normal2);
    checksum += test_unge(nan_val, normal1);
    checksum += test_unge(inf_val, normal1);
    
    checksum += test_ungt(normal2, normal1);
    checksum += test_ungt(nan_val, normal1);
    checksum += test_ungt(inf_val, normal1);
    
    checksum += test_unle(normal1, normal2);
    checksum += test_unle(nan_val, normal1);
    checksum += test_unle(neg_inf, normal1);
    
    checksum += test_unlt(normal2, normal1);
    checksum += test_unlt(nan_val, normal1);
    checksum += test_unlt(neg_inf, normal1);
    
    checksum += test_ltgt(normal1, normal2);
    checksum += test_ltgt(normal2, normal1);
    checksum += test_ltgt(nan_val, normal1);
    
    /* Test complex expressions */
    checksum += test_complex_expr(nan_val, normal1, normal2);
    checksum += test_complex_expr(normal1, inf_val, neg_inf);
    checksum += test_complex_expr(zero, zero, nan_val);
    
    /* Test mixed types */
    checksum += test_mixed_types(1.0f, nan_val, 3.14159L);
    checksum += test_mixed_types(nan_val, inf_val, neg_inf);
    
    /* Test memory operands */
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
