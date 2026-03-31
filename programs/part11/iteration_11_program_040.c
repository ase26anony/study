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
    volatile float f;
    volatile double d;
    volatile long double ld;
};

volatile struct volatile_floats vf = {0.0f, 0.0, 0.0L};

/* ========== UNORDERED/ORDERED patterns ========== */

__attribute__((noinline))
int test_unordered_ordered(void) {
    volatile double d1 = g_nan;
    volatile double d2 = g_one;
    volatile float f1 = farr[0];
    volatile long double ld1 = ldarr[0];
    
    int result = 0;
    
    /* UNORDERED: Using isunordered() */
    if (isunordered(d1, d2)) {
        result |= 1;
    }
    
    /* UNORDERED: Direct NaN comparison */
    if (d1 != d1) {  /* NaN != NaN is true */
        result |= 2;
    }
    
    /* ORDERED: Using isordered() */
    if (isordered(g_one, g_two)) {
        result |= 4;
    }
    
    /* ORDERED: Direct comparison */
    if (g_one == g_one) {  /* Non-NaN == itself */
        result |= 8;
    }
    
    /* Mixed types unordered check */
    if (isunordered(farr[1], darr[2])) {
        result |= 16;
    }
    
    sink(result);
    return result;
}

/* ========== UNEQ (unordered or equal) ========== */

__attribute__((noinline))
int test_uneq(void) {
    volatile double a = darr[0];
    volatile double b = darr[1];
    volatile double nan = g_nan;
    
    int result = 0;
    
    /* UNEQ: !(a != b) which is a == b OR unordered */
    if (!(a != b)) {
        result |= 1;
    }
    
    /* UNEQ with NaN */
    if (!(nan != nan)) {  /* NaN != NaN is true, so !true is false */
        result |= 2;
    }
    
    /* Complex expression that might generate UNEQ */
    result += (a == b) ? 4 : 0;
    
    /* Using struct member */
    if (!(vf.d != vf.d)) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* ========== UNGE (not less than) ========== */

__attribute__((noinline))
int test_unge(void) {
    volatile double x = darr[1];
    volatile double y = darr[2];
    volatile double nan = g_nan;
    
    int result = 0;
    
    /* UNGE: !(x < y) which is x >= y OR unordered */
    if (!(x < y)) {
        result |= 1;
    }
    
    /* UNGE with NaN */
    if (!(nan < y)) {  /* NaN < y is false, so !false is true */
        result |= 2;
    }
    
    /* Using float */
    volatile float f1 = farr[1];
    volatile float f2 = farr[2];
    if (!(f1 < f2)) {
        result |= 4;
    }
    
    /* Complex expression */
    result += !(x < y) ? 8 : 0;
    
    sink(result);
    return result;
}

/* ========== UNGT (not less than or equal) ========== */

__attribute__((noinline))
int test_ungt(void) {
    volatile double p = darr[2];
    volatile double q = darr[1];
    volatile double nan = g_nan;
    
    int result = 0;
    
    /* UNGT: !(p <= q) which is p > q OR unordered */
    if (!(p <= q)) {
        result |= 1;
    }
    
    /* UNGT with NaN */
    if (!(nan <= q)) {  /* NaN <= q is false, so !false is true */
        result |= 2;
    }
    
    /* Using long double */
    volatile long double ld1 = ldarr[0];
    volatile long double ld2 = ldarr[1];
    if (!(ld1 <= ld2)) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* ========== UNLE (unordered or less than or equal) ========== */

__attribute__((noinline))
int test_unle(void) {
    volatile double u = darr[0];
    volatile double v = darr[3];
    volatile double nan = g_nan;
    
    int result = 0;
    
    /* UNLE: Using || operator */
    if (isunordered(u, v) || (u <= v)) {
        result |= 1;
    }
    
    /* Alternative: !(u > v) */
    if (!(u > v)) {
        result |= 2;
    }
    
    /* With NaN */
    if (!(nan > v)) {  /* NaN > v is false, so !false is true */
        result |= 4;
    }
    
    /* Complex ternary */
    result += (u <= v) ? 8 : 0;
    if (isunordered(u, v)) {
        result |= 16;
    }
    
    sink(result);
    return result;
}

/* ========== UNLT (unordered or less than) ========== */

__attribute__((noinline))
int test_unlt(void) {
    volatile double m = darr[1];
    volatile double n = darr[3];
    volatile double nan = g_nan;
    
    int result = 0;
    
    /* UNLT: Using || operator */
    if (isunordered(m, n) || (m < n)) {
        result |= 1;
    }
    
    /* Alternative: !(m >= n) */
    if (!(m >= n)) {
        result |= 2;
    }
    
    /* With NaN */
    if (!(nan >= n)) {  /* NaN >= n is false, so !false is true */
        result |= 4;
    }
    
    /* Using array elements */
    if (!(darr[0] >= darr[2])) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* ========== LTGT (less than or greater than, ordered) ========== */

__attribute__((noinline))
int test_ltgt(void) {
    volatile double r = darr[1];
    volatile double s = darr[2];
    volatile double nan = g_nan;
    
    int result = 0;
    
    /* LTGT: (r < s) || (r > s) - ordered and not equal */
    if ((r < s) || (r > s)) {
        result |= 1;
    }
    
    /* Alternative: !(r == s) && !isunordered(r, s) */
    if (!(r == s) && !isunordered(r, s)) {
        result |= 2;
    }
    
    /* With NaN (should be false) */
    if ((nan < s) || (nan > s)) {
        result |= 4;
    }
    
    /* Complex expression */
    result += ((r < s) || (r > s)) ? 8 : 0;
    
    sink(result);
    return result;
}

/* ========== Inline assembly to force condition codes ========== */

__attribute__((noinline))
int test_asm_condition_codes(void) {
    volatile double a = darr[0];
    volatile double b = darr[1];
    volatile double nan = g_nan;
    
    int result = 0;
    
    /* Force UNORDERED condition code */
    if (isunordered(a, nan)) {
        __asm__ goto ("j%c0 %l0" : : "i" (0) : : label1);
        result |= 1;
        label1:;
    }
    
    /* Force ORDERED condition code */
    if (isordered(a, b)) {
        __asm__ goto ("j%c0 %l0" : : "i" (0) : : label2);
        result |= 2;
        label2:;
    }
    
    /* Force UNEQ condition code */
    if (!(a != b)) {
        __asm__ goto ("j%c0 %l0" : : "i" (0) : : label3);
        result |= 4;
        label3:;
    }
    
    /* Force UNGE condition code */
    if (!(a < b)) {
        __asm__ goto ("j%c0 %l0" : : "i" (0) : : label4);
        result |= 8;
        label4:;
    }
    
    sink(result);
    return result;
}

/* ========== Complex nested expressions ========== */

__attribute__((noinline))
int test_complex_expressions(void) {
    volatile double x = darr[0];
    volatile double y = darr[1];
    volatile double z = darr[2];
    volatile double nan = g_nan;
    
    int result = 0;
    
    /* Nested ternary with multiple comparisons */
    result = (x != x) ? 1 : 
             ((y > z) ? 2 : 
             ((!(x < y)) ? 3 : 
             ((isunordered(y, z)) ? 4 : 5)));
    
    /* Logical AND/OR combination */
    if ((!(x >= y)) && (isordered(z, nan) || (z != z))) {
        result |= 16;
    }
    
    /* Multiple comparisons in single expression */
    int temp = ((x < y) || (x > y)) + 
               (!(y <= z)) + 
               (isunordered(x, nan) ? 1 : 0);
    result += temp;
    
    sink(result);
    return result;
}

/* ========== Main function ========== */

int main(void) {
    int checksum = 0;
    
    /* Initialize with potentially NaN values */
    volatile double seed = 0.0;
    for (int i = 0; i < 4; i++) {
        darr[i] = seed + i;
        farr[i] = seed + i;
        if (i < 2) ldarr[i] = seed + i;
    }
    
    /* Force NaN into some variables */
    g_nan = 0.0/0.0;
    g_inf = 1.0/0.0;
    
    /* Call all test functions */
    checksum += test_unordered_ordered();
    checksum += test_uneq();
    checksum += test_unge();
    checksum += test_ungt();
    checksum += test_unle();
    checksum += test_unlt();
    checksum += test_ltgt();
    checksum += test_asm_condition_codes();
    checksum += test_complex_expressions();
    
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
