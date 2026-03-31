/* test_i386_condition_codes.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent optimization of intermediate values */
extern void sink(int value);

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

/* Pattern 1: UNORDERED comparisons */
__attribute__((noinline))
int test_unordered(void) {
    volatile double nan1 = make_nan();
    volatile double nan2 = make_nan();
    volatile double normal = 3.14;
    volatile double inf = make_inf();
    
    int result = 0;
    
    /* Direct unordered checks */
    if (isunordered(nan1, normal)) {
        result |= 1;
    }
    
    if (isunordered(nan1, nan2)) {
        result |= 2;
    }
    
    /* Using inline assembly to force condition code */
    volatile double a = nan1;
    volatile double b = normal;
    
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l[unordered_label]"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : unordered_label
    );
    
    /* Not taken path */
    result |= 4;
    goto skip_unordered;
    
unordered_label:
    /* Taken path - unordered */
    result |= 8;
    
skip_unordered:
    sink(result);
    return result;
}

/* Pattern 2: ORDERED comparisons */
__attribute__((noinline))
int test_ordered(void) {
    volatile double normal1 = 1.0;
    volatile double normal2 = 2.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    if (isordered(normal1, normal2)) {
        result |= 1;
    }
    
    /* Complex expression with ordered check */
    volatile double x = normal1;
    volatile double y = normal2;
    
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l[ordered_label]"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : ordered_label
    );
    
    result |= 2;
    goto skip_ordered;
    
ordered_label:
    result |= 4;
    
skip_ordered:
    sink(result);
    return result;
}

/* Pattern 3: UNEQ (unordered or equal) */
__attribute__((noinline))
int test_uneq(void) {
    volatile double a = 5.0;
    volatile double b = 5.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    /* Generate UNEQ: !(a != b) which is a == b or unordered */
    if (!(a != b)) {
        result |= 1;
    }
    
    /* With NaN */
    if (!(nan != nan)) {
        result |= 2;
    }
    
    /* Complex expression */
    volatile double x = a;
    volatile double y = b;
    int r = (x == x) ? ((!(x != y)) ? 4 : 8) : 16;
    result |= r;
    
    sink(result);
    return result;
}

/* Pattern 4: UNGE (not less than) */
__attribute__((noinline))
int test_unge(void) {
    volatile double a = 10.0;
    volatile double b = 5.0;
    volatile double c = 10.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    /* UNGE: !(a < b) which is a >= b or unordered */
    if (!(a < b)) {
        result |= 1;
    }
    
    if (!(c < b)) {
        result |= 2;
    }
    
    /* With NaN */
    if (!(nan < a)) {
        result |= 4;
    }
    
    /* Using inline assembly */
    volatile double x = a;
    volatile double y = b;
    
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l[unge_label]"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : unge_label
    );
    
    result |= 8;
    goto skip_unge;
    
unge_label:
    result |= 16;
    
skip_unge:
    sink(result);
    return result;
}

/* Pattern 5: UNGT (not less than or equal) */
__attribute__((noinline))
int test_ungt(void) {
    volatile double a = 10.0;
    volatile double b = 5.0;
    volatile double c = 10.0;
    
    int result = 0;
    
    /* UNGT: !(a <= b) which is a > b or unordered */
    if (!(a <= b)) {
        result |= 1;
    }
    
    if (!(c <= b)) {
        result |= 2;
    }
    
    /* Complex nested expression */
    volatile double x = a;
    volatile double y = b;
    volatile double z = c;
    
    int r = (!(x <= y)) ? ((!(y <= z)) ? 4 : 8) : 16;
    result |= r;
    
    sink(result);
    return result;
}

/* Pattern 6: UNLE (unordered or less than or equal) */
__attribute__((noinline))
int test_unle(void) {
    volatile double a = 5.0;
    volatile double b = 10.0;
    volatile double c = 10.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    /* UNLE: !(a > b) which is a <= b or unordered */
    if (!(a > b)) {
        result |= 1;
    }
    
    if (!(c > b)) {
        result |= 2;
    }
    
    /* With NaN */
    if (!(nan > a)) {
        result |= 4;
    }
    
    /* Mixed types */
    volatile float f1 = 3.0f;
    volatile float f2 = 7.0f;
    if (!(f1 > f2)) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* Pattern 7: UNLT (unordered or less than) */
__attribute__((noinline))
int test_unlt(void) {
    volatile double a = 5.0;
    volatile double b = 10.0;
    volatile double c = 10.0;
    
    int result = 0;
    
    /* UNLT: !(a >= b) which is a < b or unordered */
    if (!(a >= b)) {
        result |= 1;
    }
    
    if (!(c >= b)) {
        result |= 2;
    }
    
    /* Using memory operands */
    volatile double arr[3] = {1.0, 2.0, 3.0};
    if (!(arr[0] >= arr[2])) {
        result |= 4;
    }
    
    /* Long double */
    volatile long double ld1 = 5.0L;
    volatile long double ld2 = 10.0L;
    if (!(ld1 >= ld2)) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* Pattern 8: LTGT (less than or greater than - ordered and not equal) */
__attribute__((noinline))
int test_ltgt(void) {
    volatile double a = 5.0;
    volatile double b = 10.0;
    volatile double c = 10.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    /* LTGT: (a < b) || (a > b) */
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    /* This should be false for equal values */
    if ((c < b) || (c > b)) {
        result |= 2;
    }
    
    /* With NaN - should be false */
    if ((nan < a) || (nan > a)) {
        result |= 4;
    }
    
    /* Complex expression */
    volatile double x = a;
    volatile double y = b;
    int r = ((x < y) || (x > y)) ? 8 : 16;
    result |= r;
    
    /* Using struct members */
    struct Point {
        volatile double x;
        volatile double y;
    } p1 = {1.0, 2.0}, p2 = {3.0, 4.0};
    
    if ((p1.x < p2.x) || (p1.x > p2.x)) {
        result |= 32;
    }
    
    sink(result);
    return result;
}

/* Pattern 9: Mixed condition codes in complex expressions */
__attribute__((noinline))
int test_mixed(void) {
    volatile double nan = make_nan();
    volatile double inf = make_inf();
    volatile double normal = 42.0;
    volatile double zero = 0.0;
    
    int result = 0;
    
    /* Complex nested ternary with various comparisons */
    result = (nan != nan) ? 
                ((normal > zero) ? 1 : 2) :
                ((!(normal < zero)) ? 4 : 8);
    
    /* Multiple condition codes in logical expression */
    if ((isunordered(nan, normal) || !(normal <= zero)) && 
        ((normal > zero) || (normal < zero))) {
        result |= 16;
    }
    
    /* Array-based comparisons */
    volatile double arr[4] = {nan, inf, normal, zero};
    for (int i = 0; i < 3; i++) {
        if (!(arr[i] >= arr[i+1])) {
            result |= (1 << (i + 5));
        }
    }
    
    sink(result);
    return result;
}

/* Pattern 10: Force specific x87 condition codes */
__attribute__((noinline))
int test_x87_condition_codes(void) {
    volatile double d1, d2;
    int result = 0;
    
    /* Test various x87 comparisons that should generate specific condition codes */
    d1 = make_nan();
    d2 = 1.0;
    
    /* This should generate UNORDERED */
    __asm__ volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "fstp %%st(0)\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "m" (d1), "m" (d2)
        : "eax", "cc", "st", "st(1)"
    );
    
    sink(result);
    return result;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize volatile seed */
    volatile int seed = 42;
    srand(seed);
    
    /* Call all pattern functions */
    checksum += test_unordered();
    checksum += test_ordered();
    checksum += test_uneq();
    checksum += test_unge();
    checksum += test_ungt();
    checksum += test_unle();
    checksum += test_unlt();
    checksum += test_ltgt();
    checksum += test_mixed();
    checksum += test_x87_condition_codes();
    
    /* Print checksum to ensure all code is live */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
