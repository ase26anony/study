/* test_i386_condition_codes.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent optimization */
extern void sink(int);
extern void sink_ptr(void*);
extern void sink_double(double);

/* Force separate code generation */
#define NOINLINE __attribute__((noinline))

/* Global volatile variables to prevent constant folding */
volatile double g_nan = 0.0/0.0;
volatile double g_inf = 1.0/0.0;
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile double g_two = 2.0;

/* Arrays for memory operand variations */
volatile float farr[4] = {0.0f, 1.0f, 2.0f, 3.0f};
volatile double darr[4] = {0.0, 1.0, 2.0, 3.0};
volatile long double ldarr[4] = {0.0L, 1.0L, 2.0L, 3.0L};

/* Struct for complex memory access */
struct fp_pair {
    volatile double a;
    volatile double b;
};
volatile struct fp_pair fp_struct = {0.0, 0.0};

/* Pattern 1: UNORDERED condition */
NOINLINE int test_unordered(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    int result = 0;
    
    /* Direct NaN comparison */
    if (a != a) {
        result |= 1;
    }
    
    /* Using isunordered */
    if (isunordered(a, b)) {
        result |= 2;
    }
    
    /* Inline asm to force condition code use */
    volatile double x = g_nan;
    volatile double y = g_one;
    
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : unordered_label
    );
    
    result |= 4;
    goto end;
    
unordered_label:
    result |= 8;
    
end:
    sink(result);
    return result;
}

/* Pattern 2: ORDERED condition */
NOINLINE int test_ordered(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    int result = 0;
    
    /* Direct ordered comparison */
    if (a == a) {
        result |= 1;
    }
    
    /* Using isordered */
    if (isordered(a, b)) {
        result |= 2;
    }
    
    /* Complex expression with ordered check */
    volatile double x = g_one;
    volatile double y = g_two;
    
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : ordered_label
    );
    
    result |= 4;
    goto end2;
    
ordered_label:
    result |= 8;
    
end2:
    sink(result);
    return result;
}

/* Pattern 3: UNEQ (unordered or equal) */
NOINLINE int test_uneq(void) {
    volatile double a = g_nan;
    volatile double b = g_nan;
    volatile double c = g_one;
    volatile double d = g_one;
    int result = 0;
    
    /* Unordered case */
    if (!(a > b) && !(a < b)) {
        result |= 1;
    }
    
    /* Equal case */
    if (!(c > d) && !(c < d)) {
        result |= 2;
    }
    
    /* Memory operand variation */
    volatile float* pf = &farr[0];
    if (!(*pf > farr[1]) && !(*pf < farr[1])) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Pattern 4: UNGE (not less than) */
NOINLINE int test_unge(void) {
    volatile double a = g_two;
    volatile double b = g_one;
    volatile double c = g_nan;
    int result = 0;
    
    /* Normal case: a >= b */
    if (!(a < b)) {
        result |= 1;
    }
    
    /* NaN case: !(NaN < b) should be true */
    if (!(c < b)) {
        result |= 2;
    }
    
    /* Complex expression */
    result = (!(a < b) && !(b > a)) ? result | 4 : result;
    
    /* With inline asm */
    volatile double x = g_two;
    volatile double y = g_one;
    
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : unge_label
    );
    
    result |= 8;
    goto end4;
    
unge_label:
    result |= 16;
    
end4:
    sink(result);
    return result;
}

/* Pattern 5: UNGT (not less than or equal) */
NOINLINE int test_ungt(void) {
    volatile double a = g_two;
    volatile double b = g_one;
    volatile double c = g_nan;
    int result = 0;
    
    /* Normal case: a > b */
    if (!(a <= b)) {
        result |= 1;
    }
    
    /* NaN case: !(NaN <= b) should be true */
    if (!(c <= b)) {
        result |= 2;
    }
    
    /* Using struct member */
    fp_struct.a = g_two;
    fp_struct.b = g_one;
    if (!(fp_struct.a <= fp_struct.b)) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Pattern 6: UNLE (unordered or less than or equal) */
NOINLINE int test_unle(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    int result = 0;
    
    /* Normal case: a <= b */
    if (!(a > b)) {
        result |= 1;
    }
    
    /* NaN case: !(NaN > b) should be true */
    if (!(c > b)) {
        result |= 2;
    }
    
    /* Long double variation */
    volatile long double ld1 = ldarr[0];
    volatile long double ld2 = ldarr[1];
    if (!(ld1 > ld2)) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Pattern 7: UNLT (unordered or less than) */
NOINLINE int test_unlt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    int result = 0;
    
    /* Normal case: a < b */
    if (!(a >= b)) {
        result |= 1;
    }
    
    /* NaN case: !(NaN >= b) should be true */
    if (!(c >= b)) {
        result |= 2;
    }
    
    /* Complex nested expression */
    result = (!(a >= b) || !(b <= a)) ? result | 4 : result;
    
    sink(result);
    return result;
}

/* Pattern 8: LTGT (less than or greater than - ordered and not equal) */
NOINLINE int test_ltgt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    volatile double d = g_nan;
    int result = 0;
    
    /* Standard LTGT: (a < b) || (a > b) */
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    /* Should be false for NaN */
    if (!((c < d) || (c > d))) {
        result |= 2;
    }
    
    /* With inline asm to force condition code */
    volatile double x = g_one;
    volatile double y = g_two;
    
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : ltgt_label
    );
    
    result |= 4;
    goto end8;
    
ltgt_label:
    result |= 8;
    
end8:
    /* Array memory operand variation */
    if ((darr[0] < darr[1]) || (darr[0] > darr[1])) {
        result |= 16;
    }
    
    sink(result);
    return result;
}

/* Pattern 9: Mixed conditions in complex expression */
NOINLINE int test_mixed(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_two;
    int result = 0;
    
    /* Complex ternary with multiple conditions */
    result = (a != a) ? 1 : 
             ((b < c) ? 2 : 
             ((!(b > c)) ? 3 : 
             (((b < c) || (b > c)) ? 4 : 5)));
    
    /* Nested logical expressions */
    if ((isunordered(a, b) || !(b >= c)) && (b != c)) {
        result |= 8;
    }
    
    /* Multiple memory operands */
    volatile double* pa = &darr[0];
    volatile double* pb = &darr[1];
    volatile double* pc = &darr[2];
    
    if ((*pa < *pb) || !(*pb <= *pc)) {
        result |= 16;
    }
    
    sink(result);
    return result;
}

/* Pattern 10: SSE-based comparisons */
NOINLINE int test_sse_comparisons(void) {
    volatile float fa = farr[0];
    volatile float fb = farr[1];
    volatile float fc = farr[2];
    int result = 0;
    
    /* SSE-style comparisons that may generate different condition codes */
    if (!(fa < fb) && !(fa > fb)) {  /* UNEQ */
        result |= 1;
    }
    
    if (!(fa >= fb)) {  /* UNLT */
        result |= 2;
    }
    
    if (!(fa <= fb)) {  /* UNGT */
        result |= 4;
    }
    
    if ((fa < fb) || (fa > fb)) {  /* LTGT */
        result |= 8;
    }
    
    /* Mixed float/double */
    volatile double da = darr[0];
    if (!(fa < da) || !(da > fb)) {
        result |= 16;
    }
    
    sink(result);
    return result;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize with potentially NaN values */
    g_nan = 0.0/0.0;
    g_inf = 1.0/0.0;
    
    /* Call all test patterns */
    checksum += test_unordered();
    checksum += test_ordered();
    checksum += test_uneq();
    checksum += test_unge();
    checksum += test_ungt();
    checksum += test_unle();
    checksum += test_unlt();
    checksum += test_ltgt();
    checksum += test_mixed();
    checksum += test_sse_comparisons();
    
    /* Print checksum to ensure all code is live */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
