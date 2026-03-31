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

/* Pattern 1: UNORDERED comparisons with NaN */
__attribute__((noinline))
int test_unordered(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile float c = farr[0];
    volatile long double d = ldarr[0];
    
    int result = 0;
    
    /* Direct unordered check */
    if (a != a) {  /* Should generate UNORDERED */
        result |= 1;
    }
    
    /* Using isunordered() */
    if (isunordered(a, b)) {
        result |= 2;
    }
    
    /* Complex expression with unordered */
    result |= (isunordered(a, c) ? 4 : 0);
    
    /* Inline assembly to force condition code use */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l1"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : unordered_label
    );
    
    return result;
    
unordered_label:
    result |= 8;
    return result;
}

/* Pattern 2: ORDERED comparisons */
__attribute__((noinline))
int test_ordered(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    
    int result = 0;
    
    /* Direct ordered check */
    if (a == a) {  /* Should generate ORDERED */
        result |= 1;
    }
    
    /* Using isordered() */
    if (isordered(a, b)) {
        result |= 2;
    }
    
    /* Complex expression */
    result |= (isordered(a, c) ? 0 : 4);
    
    /* Memory operand from array */
    volatile double* ptr = &darr[1];
    if (isordered(*ptr, darr[2])) {
        result |= 8;
    }
    
    return result;
}

/* Pattern 3: UNEQ (unordered or equal) */
__attribute__((noinline))
int test_uneq(void) {
    volatile double a = g_one;
    volatile double b = g_one;
    volatile double c = g_nan;
    
    int result = 0;
    
    /* Generate UNEQ: !(a != b) which is a == b OR unordered */
    if (!(a != b)) {
        result |= 1;
    }
    
    /* With NaN */
    if (!(c != c)) {  /* NaN != NaN is true, so !(true) is false */
        result |= 2;
    }
    
    /* Complex expression */
    result |= (!(a != b) ? 4 : 0);
    
    /* Using inline assembly */
    volatile double x = farr[1];
    volatile double y = farr[1];
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l1"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : uneq_label
    );
    
    return result;
    
uneq_label:
    result |= 8;
    return result;
}

/* Pattern 4: UNGE (not less than) = !(a < b) */
__attribute__((noinline))
int test_unge(void) {
    volatile double a = g_two;
    volatile double b = g_one;
    volatile double c = g_nan;
    
    int result = 0;
    
    /* Direct UNGE: !(a < b) */
    if (!(a < b)) {
        result |= 1;
    }
    
    /* With equal values */
    if (!(g_one < g_one)) {
        result |= 2;
    }
    
    /* Complex expression with memory operand */
    volatile double* ptr = &darr[0];
    result |= (!(*ptr < darr[3]) ? 4 : 0);
    
    /* Using struct member */
    vf.d = 3.0;
    if (!(vf.d < 2.0)) {
        result |= 8;
    }
    
    return result;
}

/* Pattern 5: UNGT (not less than or equal) = !(a <= b) */
__attribute__((noinline))
int test_ungt(void) {
    volatile double a = g_two;
    volatile double b = g_one;
    
    int result = 0;
    
    /* Direct UNGT: !(a <= b) */
    if (!(a <= b)) {
        result |= 1;
    }
    
    /* With equal values */
    if (!(g_one <= g_zero)) {
        result |= 2;
    }
    
    /* Nested in ternary */
    result = (!(a <= b)) ? (result | 4) : result;
    
    /* Memory operand from array */
    if (!(darr[1] <= darr[0])) {
        result |= 8;
    }
    
    return result;
}

/* Pattern 6: UNLE (unordered or less than or equal) */
__attribute__((noinline))
int test_unle(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    
    int result = 0;
    
    /* Direct UNLE: !(a > b) */
    if (!(a > b)) {
        result |= 1;
    }
    
    /* With NaN */
    if (!(c > g_one)) {
        result |= 2;
    }
    
    /* Complex logical expression */
    if (!(a > b) || isunordered(a, b)) {
        result |= 4;
    }
    
    /* Using float type */
    volatile float f1 = farr[0];
    volatile float f2 = farr[3];
    result |= (!(f1 > f2) ? 8 : 0);
    
    return result;
}

/* Pattern 7: UNLT (unordered or less than) */
__attribute__((noinline))
int test_unlt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    
    int result = 0;
    
    /* Direct UNLT: !(a >= b) */
    if (!(a >= b)) {
        result |= 1;
    }
    
    /* With NaN */
    if (!(c >= g_one)) {
        result |= 2;
    }
    
    /* Complex expression */
    result |= (!(a >= b) ? 4 : 0);
    
    /* Using long double */
    volatile long double ld1 = ldarr[0];
    volatile long double ld2 = ldarr[1];
    if (!(ld1 >= ld2)) {
        result |= 8;
    }
    
    return result;
}

/* Pattern 8: LTGT (less than or greater than - not equal and ordered) */
__attribute__((noinline))
int test_ltgt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    
    int result = 0;
    
    /* Direct LTGT: (a < b) || (a > b) */
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    /* With equal values */
    if ((g_one < g_one) || (g_one > g_one)) {
        result |= 2;
    }
    
    /* Complex expression */
    result |= (((a < b) || (a > b)) ? 4 : 0);
    
    /* Using memory operands and mixing types */
    volatile float* fptr = (volatile float*)&farr[0];
    if ((*fptr < farr[1]) || (*fptr > farr[1])) {
        result |= 8;
    }
    
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
             ((!(b > c)) ? 3 : 
             ((b == b) ? 4 : 5)));
    
    /* Logical AND/OR of different comparisons */
    if ((isunordered(a, b) || (b < c)) && !(c <= b)) {
        result |= 8;
    }
    
    /* Multiple memory accesses */
    volatile struct volatile_floats* vfp = &vf;
    if ((vfp->d != vfp->d) || (darr[0] > darr[1])) {
        result |= 16;
    }
    
    return result;
}

/* Pattern 10: Force specific condition codes with inline assembly */
__attribute__((noinline))
int test_asm_conditions(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    
    int result = 0;
    
    /* Force UNORDERED */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l1"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : asm_unordered
    );
    
    /* Force ORDERED */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l1"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : asm_ordered
    );
    
    /* Force UNEQ */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l1"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : asm_uneq
    );
    
    return result;

asm_unordered:
    result |= 1;
    goto asm_ordered;
    
asm_ordered:
    result |= 2;
    goto asm_uneq;
    
asm_uneq:
    result |= 4;
    return result;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize volatile values */
    g_nan = 0.0/0.0;
    g_inf = 1.0/0.0;
    g_zero = 0.0;
    g_one = 1.0;
    g_two = 2.0;
    
    farr[0] = 0.0f;
    farr[1] = 1.0f;
    farr[2] = 2.0f;
    farr[3] = 3.0f;
    
    darr[0] = 0.0;
    darr[1] = 1.0;
    darr[2] = 2.0;
    darr[3] = 3.0;
    
    ldarr[0] = 0.0L;
    ldarr[1] = 1.0L;
    
    vf.f = 0.0f;
    vf.d = 0.0;
    vf.ld = 0.0L;
    
    /* Call all test functions */
    checksum += test_unordered();
    sink(checksum);
    
    checksum += test_ordered();
    sink(checksum);
    
    checksum += test_uneq();
    sink(checksum);
    
    checksum += test_unge();
    sink(checksum);
    
    checksum += test_ungt();
    sink(checksum);
    
    checksum += test_unle();
    sink(checksum);
    
    checksum += test_unlt();
    sink(checksum);
    
    checksum += test_ltgt();
    sink(checksum);
    
    checksum += test_mixed();
    sink(checksum);
    
    checksum += test_asm_conditions();
    sink(checksum);
    
    /* Print final checksum to ensure all code is live */
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
