/* test_i386_condition_codes.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent optimization */
extern void sink(int);
extern void sink_ptr(void*);

/* Volatile variables to prevent constant folding */
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
struct fp_struct {
    volatile float f;
    volatile double d;
    volatile long double ld;
};

volatile struct fp_struct fp = {0.0f, 0.0, 0.0L};

/* Pattern 1: UNORDERED comparisons with NaN */
__attribute__((noinline))
int pattern_unordered(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile float c = farr[0];
    volatile long double d = ldarr[1];
    
    int result = 0;
    
    /* Direct NaN comparison - should generate UNORDERED */
    if (a != a) {
        result |= 1;
    }
    
    /* Using isunordered() */
    if (isunordered(a, b)) {
        result |= 2;
    }
    
    /* Complex expression with unordered check */
    if (isunordered(c, d) || (c < d)) {
        result |= 4;
    }
    
    /* Inline assembly to force condition code use */
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : unordered_label
    );
    
    result |= 8;
    
unordered_label:
    sink(result);
    return result;
}

/* Pattern 2: ORDERED comparisons */
__attribute__((noinline))
int pattern_ordered(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    
    int result = 0;
    
    /* Direct ordered comparison */
    if (a == a) {
        result |= 1;
    }
    
    /* Using isordered() */
    if (isordered(a, b)) {
        result |= 2;
    }
    
    /* Complex expression */
    if (isordered(a, b) && !isunordered(c, a)) {
        result |= 4;
    }
    
    /* Inline assembly with ordered condition */
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : ordered_label
    );
    
    result |= 8;
    
ordered_label:
    sink(result);
    return result;
}

/* Pattern 3: UNEQ (unordered or equal) */
__attribute__((noinline))
int pattern_uneq(void) {
    volatile double a = farr[1];  /* Load from memory */
    volatile double b = g_one;
    volatile double c = g_nan;
    
    int result = 0;
    
    /* Generate UNEQ: !(a < b) && !(a > b) */
    if (!(a < b) && !(a > b)) {
        result |= 1;
    }
    
    /* Alternative: a == b (including NaN case) */
    if (!(a != b)) {
        result |= 2;
    }
    
    /* Complex with memory operand */
    if (!(fp.d < darr[2]) && !(fp.d > darr[2])) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Pattern 4: UNGE (not less than) */
__attribute__((noinline))
int pattern_unge(void) {
    volatile double a = g_two;
    volatile double b = g_one;
    volatile float c = farr[2];
    
    int result = 0;
    
    /* Direct: !(a < b) */
    if (!(a < b)) {
        result |= 1;
    }
    
    /* With NaN possibility */
    if (!(c < b) || isunordered(c, b)) {
        result |= 2;
    }
    
    /* Complex expression */
    int temp = (!(a < b)) ? 4 : 0;
    result |= temp;
    
    sink(result);
    return result;
}

/* Pattern 5: UNGT (not less than or equal) */
__attribute__((noinline))
int pattern_ungt(void) {
    volatile double a = g_two;
    volatile double b = g_one;
    volatile long double c = ldarr[3];
    
    int result = 0;
    
    /* Direct: !(a <= b) */
    if (!(a <= b)) {
        result |= 1;
    }
    
    /* With memory operand */
    if (!(c <= b)) {
        result |= 2;
    }
    
    /* Inline assembly */
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : ungt_label
    );
    
    result |= 4;
    
ungt_label:
    sink(result);
    return result;
}

/* Pattern 6: UNLE (unordered or less than or equal) */
__attribute__((noinline))
int pattern_unle(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    
    int result = 0;
    
    /* Direct: !(a > b) */
    if (!(a > b)) {
        result |= 1;
    }
    
    /* Complex: (a <= b) || (a != a) */
    if ((a <= b) || (a != a)) {
        result |= 2;
    }
    
    /* Using different types */
    if (!(farr[0] > darr[1])) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Pattern 7: UNLT (unordered or less than) */
__attribute__((noinline))
int pattern_unlt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    
    int result = 0;
    
    /* Direct: !(a >= b) */
    if (!(a >= b)) {
        result |= 1;
    }
    
    /* Alternative formulation */
    if ((a < b) || (a != a)) {
        result |= 2;
    }
    
    /* With struct member */
    if (!(fp.d >= darr[3])) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Pattern 8: LTGT (less than or greater than - ordered and not equal) */
__attribute__((noinline))
int pattern_ltgt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    
    int result = 0;
    
    /* Direct: (a < b) || (a > b) */
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    /* With NaN check to ensure ordered */
    if ((!isunordered(a, b)) && (a != b)) {
        result |= 2;
    }
    
    /* Complex nested ternary */
    int temp = ((a < b) || (a > b)) ? 
               ((b < a) || (b > a) ? 4 : 0) : 0;
    result |= temp;
    
    /* Inline assembly for LTGT */
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : ltgt_label
    );
    
    result |= 8;
    
ltgt_label:
    sink(result);
    return result;
}

/* Pattern 9: Mixed condition codes in complex expression */
__attribute__((noinline))
int pattern_mixed(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_two;
    volatile float d = farr[1];
    
    int result = 0;
    
    /* Complex expression combining multiple conditions */
    result = (a != a) ? 1 : 
             (!(b < c) ? 2 : 
             (!(c > b) ? 3 : 
             ((d < c) || (d > c) ? 4 : 5)));
    
    /* Nested with logical operators */
    if ((isunordered(a, b) && !(c == c)) || 
        (!(d >= c) && (b <= c))) {
        result |= 8;
    }
    
    /* Memory-intensive version */
    volatile double* ptr = &darr[0];
    if (!(*ptr < *(ptr+1)) && !(*(ptr+1) > *(ptr+2))) {
        result |= 16;
    }
    
    sink(result);
    return result;
}

/* Pattern 10: Long double operations */
__attribute__((noinline))
int pattern_long_double(void) {
    volatile long double a = ldarr[0];
    volatile long double b = ldarr[1];
    volatile long double c = 0.0L/0.0L;  /* NaN */
    
    int result = 0;
    
    /* Long double comparisons */
    if (a != a) {
        result |= 1;
    }
    
    if (!(a > b)) {
        result |= 2;
    }
    
    if ((a < b) || (a > b)) {
        result |= 4;
    }
    
    if (!(a <= b)) {
        result |= 8;
    }
    
    sink(result);
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
    
    farr[0] = 0.0f; farr[1] = 1.0f; farr[2] = 2.0f; farr[3] = 3.0f;
    darr[0] = 0.0; darr[1] = 1.0; darr[2] = 2.0; darr[3] = 3.0;
    ldarr[0] = 0.0L; ldarr[1] = 1.0L; ldarr[2] = 2.0L; ldarr[3] = 3.0L;
    
    fp.f = 1.5f;
    fp.d = 2.5;
    fp.ld = 3.5L;
    
    /* Call all pattern functions */
    checksum ^= pattern_unordered();
    checksum ^= pattern_ordered();
    checksum ^= pattern_uneq();
    checksum ^= pattern_unge();
    checksum ^= pattern_ungt();
    checksum ^= pattern_unle();
    checksum ^= pattern_unlt();
    checksum ^= pattern_ltgt();
    checksum ^= pattern_mixed();
    checksum ^= pattern_long_double();
    
    /* Print checksum to ensure all code is live */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

/* Dummy sink functions to prevent optimization */
void sink(int x) {
    volatile static int buffer;
    buffer = x;
}

void sink_ptr(void* p) {
    volatile static void* buffer;
    buffer = p;
}
