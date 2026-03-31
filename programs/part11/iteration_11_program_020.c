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
    
    /* UNORDERED: using isunordered() */
    if (isunordered(d1, d2)) {
        result |= 1;
    }
    
    /* UNORDERED: direct NaN comparison */
    if (d1 != d1) {  /* NaN != NaN is true */
        result |= 2;
    }
    
    /* ORDERED: using isordered() */
    if (isordered(g_one, g_two)) {
        result |= 4;
    }
    
    /* ORDERED: direct comparison */
    if (g_one == g_one) {  /* Non-NaN == itself */
        result |= 8;
    }
    
    /* Mixed types */
    if (isunordered(farr[1], darr[2])) {
        result |= 16;
    }
    
    /* Memory operand from struct */
    if (isordered(vf.d, g_two)) {
        result |= 32;
    }
    
    sink(result);
    return result;
}

/* ========== UNEQ (unordered or equal) ========== */

__attribute__((noinline))
int test_uneq(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_one;
    
    int result = 0;
    
    /* UNEQ: !(a != b) which is a == b OR unordered */
    if (!(a != b)) {  /* NaN != 1.0 is true, so !(true) is false */
        result |= 1;
    }
    
    if (!(c != b)) {  /* 1.0 != 1.0 is false, so !(false) is true */
        result |= 2;
    }
    
    /* Using inline assembly to force condition code */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l1"
        : /* no outputs */
        : /* no inputs */
        : "st", "cc"
        : unordered_label
    );
    
    result |= 4;
    goto after_label;
    
unordered_label:
    result |= 8;
    
after_label:
    
    sink(result);
    return result;
}

/* ========== UNGE (not less than) ========== */

__attribute__((noinline))
int test_unge(void) {
    volatile double x = g_one;
    volatile double y = g_two;
    volatile double z = g_nan;
    
    int result = 0;
    
    /* UNGE: !(x < y) which is x >= y OR unordered */
    if (!(x < y)) {  /* 1.0 < 2.0 is true, so !(true) is false */
        result |= 1;
    }
    
    if (!(y < x)) {  /* 2.0 < 1.0 is false, so !(false) is true */
        result |= 2;
    }
    
    if (!(z < x)) {  /* NaN < 1.0 is unordered, so !(unordered) is true */
        result |= 4;
    }
    
    /* Complex expression with memory operand */
    if (!(darr[0] < darr[1]) && (farr[2] > farr[1])) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* ========== UNGT (not less than or equal) ========== */

__attribute__((noinline))
int test_ungt(void) {
    volatile double a = g_two;
    volatile double b = g_one;
    volatile double c = g_nan;
    
    int result = 0;
    
    /* UNGT: !(a <= b) which is a > b OR unordered */
    if (!(a <= b)) {  /* 2.0 <= 1.0 is false, so !(false) is true */
        result |= 1;
    }
    
    if (!(b <= a)) {  /* 1.0 <= 2.0 is true, so !(true) is false */
        result |= 2;
    }
    
    if (!(c <= a)) {  /* NaN <= 2.0 is unordered, so !(unordered) is true */
        result |= 4;
    }
    
    /* Using inline assembly with condition code */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l1"
        : /* no outputs */
        : /* no inputs */
        : "st", "cc"
        : nle_label
    );
    
    result |= 8;
    goto after_nle;
    
nle_label:
    result |= 16;
    
after_nle:
    
    sink(result);
    return result;
}

/* ========== UNLE (unordered or less than or equal) ========== */

__attribute__((noinline))
int test_unle(void) {
    volatile double p = g_one;
    volatile double q = g_two;
    volatile double r = g_nan;
    
    int result = 0;
    
    /* UNLE: !(p > q) which is p <= q OR unordered */
    if (!(p > q)) {  /* 1.0 > 2.0 is false, so !(false) is true */
        result |= 1;
    }
    
    if (!(q > p)) {  /* 2.0 > 1.0 is true, so !(true) is false */
        result |= 2;
    }
    
    if (!(r > p)) {  /* NaN > 1.0 is unordered, so !(unordered) is true */
        result |= 4;
    }
    
    /* Nested ternary expression */
    result |= (!(darr[2] > darr[1])) ? 8 : 16;
    
    sink(result);
    return result;
}

/* ========== UNLT (unordered or less than) ========== */

__attribute__((noinline))
int test_unlt(void) {
    volatile double u = g_one;
    volatile double v = g_two;
    volatile double w = g_nan;
    
    int result = 0;
    
    /* UNLT: !(u >= v) which is u < v OR unordered */
    if (!(u >= v)) {  /* 1.0 >= 2.0 is false, so !(false) is true */
        result |= 1;
    }
    
    if (!(v >= u)) {  /* 2.0 >= 1.0 is true, so !(true) is false */
        result |= 2;
    }
    
    if (!(w >= u)) {  /* NaN >= 1.0 is unordered, so !(unordered) is true */
        result |= 4;
    }
    
    /* Complex logical expression */
    if ((!(farr[0] >= farr[1])) || (darr[3] != darr[2])) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* ========== LTGT (not equal and ordered) ========== */

__attribute__((noinline))
int test_ltgt(void) {
    volatile double m = g_one;
    volatile double n = g_two;
    volatile double o = g_nan;
    
    int result = 0;
    
    /* LTGT: (m < n) || (m > n) - not equal and ordered */
    if ((m < n) || (m > n)) {  /* 1.0 < 2.0 is true, so true */
        result |= 1;
    }
    
    if ((n < m) || (n > m)) {  /* 2.0 < 1.0 is false, 2.0 > 1.0 is true, so true */
        result |= 2;
    }
    
    if ((o < m) || (o > m)) {  /* Both comparisons unordered, so false */
        result |= 4;
    }
    
    /* Using inline assembly */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l1"
        : /* no outputs */
        : /* no inputs */
        : "st", "cc"
        : une_label
    );
    
    result |= 8;
    goto after_une;
    
une_label:
    result |= 16;
    
after_une:
    
    sink(result);
    return result;
}

/* ========== Complex combined patterns ========== */

__attribute__((noinline))
int test_combined(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_two;
    volatile float f = farr[0];
    
    int result = 0;
    
    /* Complex expression mixing multiple condition codes */
    result = (a != a) ? 1 : ((b > c) ? 2 : 3);
    result += (!(b < c)) ? 4 : 8;
    result += ((f > farr[1]) || (f < farr[1])) ? 16 : 32;
    
    /* Nested with memory operands from struct */
    if (isunordered(vf.d, b) && !(vf.d >= c)) {
        result += 64;
    }
    
    /* Multiple comparisons in single expression */
    int temp = (darr[0] == darr[0]) + (!(darr[1] <= darr[2])) + 
               ((darr[3] < darr[0]) || (darr[3] > darr[0]));
    result += temp * 128;
    
    sink(result);
    return result;
}

/* ========== Main function ========== */

int main(void) {
    int checksum = 0;
    
    /* Initialize volatile globals */
    g_nan = 0.0/0.0;
    g_inf = 1.0/0.0;
    g_zero = 0.0;
    g_one = 1.0;
    g_two = 2.0;
    
    farr[0] = 0.0f; farr[1] = 1.0f; farr[2] = 2.0f; farr[3] = 3.0f;
    darr[0] = 0.0; darr[1] = 1.0; darr[2] = 2.0; darr[3] = 3.0;
    ldarr[0] = 0.0L; ldarr[1] = 1.0L;
    
    vf.f = 0.5f;
    vf.d = 1.5;
    vf.ld = 2.5L;
    
    /* Call all test functions */
    checksum += test_unordered_ordered();
    checksum += test_uneq();
    checksum += test_unge();
    checksum += test_ungt();
    checksum += test_unle();
    checksum += test_unlt();
    checksum += test_ltgt();
    checksum += test_combined();
    
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
