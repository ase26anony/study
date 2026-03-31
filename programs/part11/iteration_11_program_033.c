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

/* UNORDERED/ORDERED patterns */
__attribute__((noinline))
static int test_unordered_ordered(void) {
    volatile double nan1 = make_nan();
    volatile double nan2 = make_nan();
    volatile double inf = make_inf();
    volatile double normal = 3.14;
    int result = 0;
    
    /* UNORDERED: using isunordered() */
    if (isunordered(nan1, normal)) {
        result |= 1;
    }
    
    /* UNORDERED: direct NaN comparison */
    if (nan1 != nan1) {
        result |= 2;
    }
    
    /* ORDERED: using isordered() */
    if (isordered(normal, inf)) {
        result |= 4;
    }
    
    /* ORDERED: direct comparison */
    if (normal == normal) {
        result |= 8;
    }
    
    /* Complex unordered expression */
    volatile double x = nan1;
    volatile double y = normal;
    if (x != x || y != y) {
        result |= 16;
    }
    
    checksum += result;
    return result;
}

/* UNEQ (unordered or equal) */
__attribute__((noinline))
static int test_uneq(void) {
    volatile double a = make_nan();
    volatile double b = 5.0;
    volatile double c = 5.0;
    volatile double d = 6.0;
    int result = 0;
    
    /* Should trigger UNEQ: !(a > b) && !(a < b) for NaN */
    if (!(a > b) && !(a < b)) {
        result |= 1;
    }
    
    /* Equal values */
    if (!(c > d) && !(c < d)) {
        result |= 2;
    }
    
    /* Using inline assembly to force condition code */
    volatile double e = make_nan();
    volatile double f = 7.0;
    
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l1\n\t"
        : /* no outputs */
        : /* no inputs */
        : "cc", "st", "st(1)"
        : equal_label
    );
    
    result |= 4;
    goto after_label;
    
equal_label:
    result |= 8;
    
after_label:
    checksum += result;
    return result;
}

/* UNGE (not less than: !(a < b)) */
__attribute__((noinline))
static int test_unge(void) {
    volatile double arr[4] = {1.0, 2.0, make_nan(), 4.0};
    volatile double x = arr[0];
    volatile double y = arr[1];
    volatile double nan = arr[2];
    int result = 0;
    
    /* Direct UNGE: !(x < y) when x >= y */
    if (!(x < y)) {  /* 1.0 < 2.0 is true, so !(true) is false */
        result |= 1;
    }
    
    /* With NaN operand */
    if (!(nan < y)) {  /* NaN < 2.0 is unordered, !(unordered) */
        result |= 2;
    }
    
    /* Complex expression */
    volatile double a = 5.0;
    volatile double b = 5.0;
    if (!(a < b) || (x != x)) {
        result |= 4;
    }
    
    checksum += result;
    return result;
}

/* UNGT (not less than or equal: !(a <= b)) */
__attribute__((noinline))
static int test_ungt(void) {
    volatile struct { double a; double b; } s = {3.0, 2.0};
    volatile double *ptr = &s.a;
    int result = 0;
    
    /* UNGT: !(b <= a) when b > a */
    if (!(s.b <= s.a)) {  /* 2.0 <= 3.0 is true, !(true) is false */
        result |= 1;
    }
    
    /* With memory operand */
    if (!(*(ptr+1) <= *ptr)) {  /* 2.0 <= 3.0 */
        result |= 2;
    }
    
    /* Using inline assembly */
    volatile double x = 10.0;
    volatile double y = 5.0;
    
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l1\n\t"
        : /* no outputs */
        : /* no inputs */
        : "cc", "st", "st(1)"
        : nle_label
    );
    
    result |= 4;
    goto after_label2;
    
nle_label:
    result |= 8;
    
after_label2:
    checksum += result;
    return result;
}

/* UNLE (unordered or less than or equal) */
__attribute__((noinline))
static int test_unle(void) {
    volatile double a = 2.0;
    volatile double b = 3.0;
    volatile double nan = make_nan();
    int result = 0;
    
    /* UNLE: !(a > b) */
    if (!(a > b)) {  /* 2.0 > 3.0 is false, !(false) is true */
        result |= 1;
    }
    
    /* With NaN */
    if (!(nan > b)) {  /* NaN > 3.0 is unordered, !(unordered) */
        result |= 2;
    }
    
    /* Complex ternary expression */
    volatile double x = 1.0;
    volatile double y = 2.0;
    int r = (!(x > y)) ? 16 : 32;
    result |= r;
    
    checksum += result;
    return result;
}

/* UNLT (unordered or less than) */
__attribute__((noinline))
static int test_unlt(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double nan = make_nan();
    int result = 0;
    
    /* UNLT: !(a >= b) */
    if (!(a >= b)) {  /* 1.0 >= 2.0 is false, !(false) is true */
        result |= 1;
    }
    
    /* With NaN */
    if (!(nan >= b)) {  /* NaN >= 2.0 is unordered, !(unordered) */
        result |= 2;
    }
    
    /* Nested comparisons */
    volatile double x = 5.0;
    volatile double y = 10.0;
    if ((!(x >= y)) && (x != nan)) {
        result |= 4;
    }
    
    checksum += result;
    return result;
}

/* LTGT (less than or greater than: not equal and ordered) */
__attribute__((noinline))
static int test_ltgt(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double c = 2.0;
    volatile double nan = make_nan();
    int result = 0;
    
    /* LTGT: (a < b) || (a > b) */
    if ((a < b) || (a > b)) {  /* 1.0 < 2.0 is true */
        result |= 1;
    }
    
    /* Equal values should not trigger */
    if ((c < b) || (c > b)) {  /* 2.0 < 2.0 and 2.0 > 2.0 are both false */
        result |= 2;
    }
    
    /* With NaN (should be false) */
    if ((nan < b) || (nan > b)) {  /* Both unordered */
        result |= 4;
    }
    
    /* Using different floating types */
    volatile float f1 = 3.0f;
    volatile float f2 = 4.0f;
    if ((f1 < f2) || (f1 > f2)) {
        result |= 8;
    }
    
    /* long double */
    volatile long double ld1 = 5.0L;
    volatile long double ld2 = 6.0L;
    if ((ld1 < ld2) || (ld1 > ld2)) {
        result |= 16;
    }
    
    checksum += result;
    return result;
}

/* Combined complex expression */
__attribute__((noinline))
static int test_combined(void) {
    volatile double vals[3] = {1.0, make_nan(), 3.0};
    int result = 0;
    
    /* Mix of different condition codes in one expression */
    result = (vals[0] != vals[0]) ? 1 : 
             ((!(vals[0] >= vals[2])) ? 2 : 
             ((vals[0] < vals[2]) || (vals[0] > vals[2]) ? 3 : 4));
    
    /* Another complex expression */
    volatile double x = vals[1];  /* NaN */
    volatile double y = vals[2];  /* 3.0 */
    if ((!(x < y)) && (!(x > y)) && (x == x || y != y)) {
        result |= 8;
    }
    
    checksum += result;
    return result;
}

int main(void) {
    printf("Testing i386 condition codes...\n");
    
    int total = 0;
    
    total += test_unordered_ordered();
    total += test_uneq();
    total += test_unge();
    total += test_ungt();
    total += test_unle();
    total += test_unlt();
    total += test_ltgt();
    total += test_combined();
    
    /* Use sink to prevent optimization */
    sink(total);
    sink(checksum);
    
    printf("Total result: %d\n", total);
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
