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
volatile double arr_d[4] = {0.0, 1.0, 2.0, 0.0/0.0};
volatile float arr_f[4] = {0.0f, 1.0f, 2.0f, 0.0f/0.0f};
volatile long double arr_ld[4] = {0.0L, 1.0L, 2.0L, 0.0L/0.0L};

/* Struct for complex memory access */
struct fp_pair {
    volatile double a;
    volatile double b;
};
volatile struct fp_pair fp_struct = {0.0, 0.0/0.0};

/* Pattern 1: UNORDERED comparisons with NaN */
__attribute__((noinline))
int test_unordered(void) {
    volatile double d = g_nan;
    volatile float f = arr_f[3];
    volatile long double ld = arr_ld[3];
    int result = 0;
    
    /* Using isunordered() */
    if (isunordered(d, d)) {
        result |= 1;
    }
    
    /* Direct NaN comparison */
    if (d != d) {
        result |= 2;
    }
    
    /* Mixed types */
    if (isunordered(f, d)) {
        result |= 4;
    }
    
    /* Memory operand */
    if (isunordered(arr_d[3], arr_d[0])) {
        result |= 8;
    }
    
    /* Struct member */
    if (isunordered(fp_struct.a, fp_struct.b)) {
        result |= 16;
    }
    
    /* Inline assembly to force condition code */
    volatile double x = g_nan;
    volatile double y = g_one;
    
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l1\n\t"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : unordered_label
    );
    
    result |= 32;
    goto end;
    
unordered_label:
    result |= 64;
    
end:
    sink(result);
    return result;
}

/* Pattern 2: ORDERED comparisons */
__attribute__((noinline))
int test_ordered(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double nan = g_nan;
    int result = 0;
    
    /* Using isordered() */
    if (isordered(a, b)) {
        result |= 1;
    }
    
    /* Direct comparison */
    if (a == a && b == b) {
        result |= 2;
    }
    
    /* With NaN */
    if (isordered(nan, a)) {
        result |= 4;
    } else {
        result |= 8;
    }
    
    /* Complex expression */
    result |= (isordered(arr_d[0], arr_d[1]) ? 16 : 0);
    
    /* Inline assembly */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l1\n\t"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : ordered_label
    );
    
    result |= 32;
    goto end2;
    
ordered_label:
    result |= 64;
    
end2:
    sink(result);
    return result;
}

/* Pattern 3: UNEQ (unordered or equal) */
__attribute__((noinline))
int test_uneq(void) {
    volatile double a = g_one;
    volatile double b = g_one;
    volatile double nan = g_nan;
    int result = 0;
    
    /* Direct unordered or equal */
    if (!(a > b) && !(a < b)) {  /* a == b or unordered */
        result |= 1;
    }
    
    /* With NaN */
    if (isunordered(nan, a) || nan == a) {
        result |= 2;
    }
    
    /* Complex expression */
    result |= ((a != a) || (a == b)) ? 4 : 0;
    
    /* Memory operands */
    if (!(arr_d[0] > arr_d[1]) && !(arr_d[0] < arr_d[1])) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* Pattern 4: UNGE (not less than) = !(a < b) */
__attribute__((noinline))
int test_unge(void) {
    volatile double a = g_two;
    volatile double b = g_one;
    volatile double nan = g_nan;
    int result = 0;
    
    /* Direct !(a < b) */
    if (!(a < b)) {
        result |= 1;
    }
    
    /* With NaN */
    if (!(nan < b)) {
        result |= 2;
    }
    
    /* Complex: !(a < b) && ordered */
    if (!(a < b) && isordered(a, b)) {
        result |= 4;
    }
    
    /* Ternary operator */
    result |= (!(arr_f[0] < arr_f[1])) ? 8 : 0;
    
    sink(result);
    return result;
}

/* Pattern 5: UNGT (not less than or equal) = !(a <= b) */
__attribute__((noinline))
int test_ungt(void) {
    volatile double a = g_two;
    volatile double b = g_one;
    int result = 0;
    
    /* Direct !(a <= b) */
    if (!(a <= b)) {
        result |= 1;
    }
    
    /* Complex: !(a <= b) || unordered */
    if (!(a <= b) || isunordered(a, b)) {
        result |= 2;
    }
    
    /* Memory operand with struct */
    if (!(fp_struct.a <= fp_struct.b)) {
        result |= 4;
    }
    
    /* Nested in expression */
    result = (result > 0) ? (result | (!(arr_d[0] <= arr_d[1]) ? 8 : 0)) : 0;
    
    sink(result);
    return result;
}

/* Pattern 6: UNLE (unordered or less than or equal) */
__attribute__((noinline))
int test_unle(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double nan = g_nan;
    int result = 0;
    
    /* Direct unordered or <= */
    if (isunordered(a, b) || a <= b) {
        result |= 1;
    }
    
    /* Alternative: !(a > b) */
    if (!(a > b)) {
        result |= 2;
    }
    
    /* With NaN */
    if (!(nan > b)) {
        result |= 4;
    }
    
    /* Complex memory access */
    volatile double* ptr = &arr_d[0];
    if (!(ptr[0] > ptr[1])) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* Pattern 7: UNLT (unordered or less than) */
__attribute__((noinline))
int test_unlt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double nan = g_nan;
    int result = 0;
    
    /* Direct unordered or < */
    if (isunordered(a, b) || a < b) {
        result |= 1;
    }
    
    /* Alternative: !(a >= b) */
    if (!(a >= b)) {
        result |= 2;
    }
    
    /* With NaN */
    if (!(nan >= b)) {
        result |= 4;
    }
    
    /* Nested ternary */
    result |= (!(arr_f[0] >= arr_f[1])) ? 8 : 0;
    
    sink(result);
    return result;
}

/* Pattern 8: LTGT (not equal and ordered) */
__attribute__((noinline))
int test_ltgt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double nan = g_nan;
    int result = 0;
    
    /* Direct (a < b) || (a > b) */
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    /* With ordered check */
    if (isordered(a, b) && a != b) {
        result |= 2;
    }
    
    /* Complex: !(a == b) && ordered */
    if (!(a == b) && isordered(a, b)) {
        result |= 4;
    }
    
    /* Memory operands */
    if ((arr_d[0] < arr_d[1]) || (arr_d[0] > arr_d[1])) {
        result |= 8;
    }
    
    /* Inline assembly for condition code */
    volatile double x = g_one;
    volatile double y = g_two;
    
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l1\n\t"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : ltgt_label
    );
    
    result |= 16;
    goto end8;
    
ltgt_label:
    result |= 32;
    
end8:
    sink(result);
    return result;
}

/* Pattern 9: Complex nested comparisons */
__attribute__((noinline))
int test_complex_nested(void) {
    volatile double a = arr_d[0];
    volatile double b = arr_d[1];
    volatile double c = arr_d[2];
    volatile double nan = arr_d[3];
    int result = 0;
    
    /* Multiple condition codes in one expression */
    result = (a != a) ? 1 : 
             ((b > c) ? 2 : 
             ((!(a < b)) ? 3 : 
             ((isunordered(c, nan)) ? 4 : 
             ((!(b >= c)) ? 5 : 6))));
    
    /* Logical AND/OR of comparisons */
    if ((!(a <= b)) && (isordered(c, nan) || (c > a))) {
        result |= 8;
    }
    
    /* Chained comparisons */
    int r1 = (!(a == b)) && isordered(a, b);
    int r2 = (!(b >= c)) || isunordered(b, nan);
    int r3 = (a < b) || (a > b);
    
    result |= (r1 ? 16 : 0) | (r2 ? 32 : 0) | (r3 ? 64 : 0);
    
    sink(result);
    return result;
}

/* Pattern 10: Mixed floating-point types */
__attribute__((noinline))
int test_mixed_types(void) {
    volatile float f1 = arr_f[0];
    volatile float f2 = arr_f[1];
    volatile double d1 = arr_d[0];
    volatile double d2 = arr_d[1];
    volatile long double ld1 = arr_ld[0];
    volatile long double ld2 = arr_ld[1];
    int result = 0;
    
    /* Float comparisons */
    if (!(f1 < f2)) result |= 1;
    if (isunordered(f1, f2) || f1 == f2) result |= 2;
    
    /* Double comparisons */
    if ((d1 < d2) || (d1 > d2)) result |= 4;
    if (!(d1 >= d2)) result |= 8;
    
    /* Long double comparisons */
    if (!(ld1 <= ld2)) result |= 16;
    if (isunordered(ld1, ld2)) result |= 32;
    
    /* Cross-type comparisons */
    if (!(f1 > d1)) result |= 64;
    if (isunordered(d1, ld1)) result |= 128;
    
    sink(result);
    return result;
}

/* Main function with checksum */
int main(void) {
    int checksum = 0;
    
    /* Initialize struct */
    fp_struct.a = g_one;
    fp_struct.b = g_nan;
    
    /* Initialize arrays with varying values */
    for (int i = 0; i < 4; i++) {
        arr_d[i] = (double)i;
        arr_f[i] = (float)i;
        arr_ld[i] = (long double)i;
    }
    arr_d[3] = g_nan;
    arr_f[3] = 0.0f/0.0f;
    arr_ld[3] = 0.0L/0.0L;
    
    /* Call all test functions */
    checksum ^= test_unordered();
    checksum ^= test_ordered();
    checksum ^= test_uneq();
    checksum ^= test_unge();
    checksum ^= test_ungt();
    checksum ^= test_unle();
    checksum ^= test_unlt();
    checksum ^= test_ltgt();
    checksum ^= test_complex_nested();
    checksum ^= test_mixed_types();
    
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
