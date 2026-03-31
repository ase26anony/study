/* test_i386_condition_codes.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent optimization */
extern void sink(int);
extern void sink_ptr(void*);

/* Volatile to prevent constant folding */
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
    volatile float f1;
    volatile float f2;
    volatile double d1;
    volatile double d2;
};

/* Function 1: UNORDERED/ORDERED patterns */
__attribute__((noinline))
int test_unordered_ordered(void) {
    volatile double d = g_nan;
    volatile float f = farr[0];
    volatile long double ld = ldarr[0];
    int result = 0;
    
    /* UNORDERED patterns */
    if (d != d) {  /* Should generate UNORDERED */
        result |= 1;
    }
    
    if (isunordered(f, farr[1])) {
        result |= 2;
    }
    
    if (isunordered(ld, ldarr[1])) {
        result |= 4;
    }
    
    /* ORDERED patterns */
    if (d == d) {  /* Should generate ORDERED (false for NaN) */
        result |= 8;
    }
    
    if (isordered(g_one, g_two)) {
        result |= 16;
    }
    
    /* Complex expression with unordered */
    result += (d != d) ? 32 : ((g_one < g_two) ? 64 : 128);
    
    sink(result);
    return result;
}

/* Function 2: UNEQ (unordered or equal) */
__attribute__((noinline))
int test_uneq(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_one;
    int result = 0;
    
    /* UNEQ: !(a < b) && !(a > b)  or  a == b || unordered(a,b) */
    if (!(a < b) && !(a > b)) {
        result |= 1;
    }
    
    /* Using struct members */
    struct volatile_floats vf = {0.0f, 0.0f/0.0f, 0.0, 0.0/0.0};
    if (!(vf.d1 < vf.d2) && !(vf.d1 > vf.d2)) {
        result |= 2;
    }
    
    /* Inline assembly to force condition code */
    int r1 = 0;
    __asm__ goto (
        "fucomi %%st(1), %%st\n\t"
        "jne %l[not_equal]\n\t"
        "jp %l[unordered]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)"
        : not_equal, unordered
    );
    
    r1 = 1;
    goto end;
    
not_equal:
    r1 = 2;
    goto end;
    
unordered:
    r1 = 3;
    
end:
    result |= (r1 << 4);
    
    sink(result);
    return result;
}

/* Function 3: UNGE (!(a < b)) and UNGT (!(a <= b)) */
__attribute__((noinline))
int test_unge_ungt(void) {
    volatile float x = farr[0];
    volatile float y = farr[1];
    volatile double d1 = darr[0];
    volatile double d2 = darr[1];
    int result = 0;
    
    /* UNGE: !(x < y)  which is x >= y or unordered */
    if (!(x < y)) {
        result |= 1;
    }
    
    /* UNGT: !(x <= y)  which is x > y or unordered */
    if (!(d1 <= d2)) {
        result |= 2;
    }
    
    /* Complex expression combining both */
    result += (!(x < y) && !(d1 <= d2)) ? 4 : 8;
    
    /* Using memory operands */
    if (!(farr[2] < farr[3])) {
        result |= 16;
    }
    
    if (!(darr[2] <= darr[3])) {
        result |= 32;
    }
    
    sink(result);
    return result;
}

/* Function 4: UNLE (!(a > b)) and UNLT (!(a >= b)) */
__attribute__((noinline))
int test_unle_unlt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double nan = g_nan;
    int result = 0;
    
    /* UNLE: !(a > b)  which is a <= b or unordered */
    if (!(a > b)) {
        result |= 1;
    }
    
    /* UNLT: !(a >= b)  which is a < b or unordered */
    if (!(a >= b)) {
        result |= 2;
    }
    
    /* With NaN */
    if (!(nan > b)) {  /* Always true */
        result |= 4;
    }
    
    if (!(nan >= b)) {  /* Always true */
        result |= 8;
    }
    
    /* Complex ternary */
    result += (!(a > b) || !(b >= a)) ? 16 : 32;
    
    sink(result);
    return result;
}

/* Function 5: LTGT (not equal and ordered: (a < b) || (a > b)) */
__attribute__((noinline))
int test_ltgt(void) {
    volatile float f1 = farr[0];
    volatile float f2 = farr[1];
    volatile double d1 = darr[2];
    volatile double d2 = darr[3];
    int result = 0;
    
    /* LTGT: (a < b) || (a > b) */
    if ((f1 < f2) || (f1 > f2)) {
        result |= 1;
    }
    
    if ((d1 < d2) || (d1 > d2)) {
        result |= 2;
    }
    
    /* With NaN - should be false */
    volatile double nan = g_nan;
    if ((nan < d1) || (nan > d1)) {
        result |= 4;
    }
    
    /* Complex nested expression */
    result += ((f1 < f2) || (f1 > f2)) ? 
              (((d1 < d2) || (d1 > d2)) ? 8 : 16) : 32;
    
    /* Inline assembly to force LTGT condition code */
    int r = 0;
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "jne %l[not_equal]\n\t"
        "jp %l[unordered]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)"
        : not_equal, unordered
    );
    
    /* Equal and ordered */
    r = 1;
    goto done;
    
not_equal:
    /* Not equal - could be LTGT or unordered */
    r = 2;
    goto done;
    
unordered:
    r = 3;
    
done:
    result |= (r << 8);
    
    sink(result);
    return result;
}

/* Function 6: Mixed condition codes in complex expressions */
__attribute__((noinline))
int test_mixed_complex(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_two;
    volatile float f = farr[0];
    int result = 0;
    
    /* Complex expression with multiple condition codes */
    result = (a != a) ? 1 : 
             ((b < c) ? 2 : 
             ((!(b > c)) ? 3 : 
             (((b < c) || (b > c)) ? 4 : 5)));
    
    /* Logical AND/OR of comparisons */
    if ((!(a < b)) && ((b < c) || (b > c))) {
        result |= 8;
    }
    
    if ((!(farr[1] >= farr[2])) || (isunordered(darr[0], darr[1]))) {
        result |= 16;
    }
    
    /* Nested comparisons */
    int temp = 0;
    temp = (!(b > c)) ? 32 : 64;
    temp += ((b < c) || (b > c)) ? 128 : 256;
    result += temp;
    
    sink(result);
    return result;
}

/* Function 7: Long double comparisons */
__attribute__((noinline))
int test_long_double(void) {
    volatile long double ld1 = ldarr[0];
    volatile long double ld2 = ldarr[1];
    volatile long double ld_nan = 0.0L/0.0L;
    int result = 0;
    
    /* Various condition codes with long double */
    if (ld1 != ld1) {  /* UNORDERED */
        result |= 1;
    }
    
    if (!(ld1 < ld2)) {  /* UNGE */
        result |= 2;
    }
    
    if (!(ld1 > ld2)) {  /* UNLE */
        result |= 4;
    }
    
    if ((ld1 < ld2) || (ld1 > ld2)) {  /* LTGT */
        result |= 8;
    }
    
    if (!(ld1 <= ld2)) {  /* UNGT */
        result |= 16;
    }
    
    if (!(ld1 >= ld2)) {  /* UNLT */
        result |= 32;
    }
    
    /* With NaN */
    if (!(ld_nan < ld1)) {  /* UNGE (always true) */
        result |= 64;
    }
    
    sink(result);
    return result;
}

/* Main function that calls all test patterns */
int main(void) {
    int checksum = 0;
    
    /* Initialize some NaN values */
    volatile double local_nan = 0.0/0.0;
    volatile float local_nanf = 0.0f/0.0f;
    sink_ptr((void*)&local_nan);
    sink_ptr((void*)&local_nanf);
    
    /* Call each test function */
    checksum += test_unordered_ordered();
    checksum += test_uneq();
    checksum += test_unge_ungt();
    checksum += test_unle_unlt();
    checksum += test_ltgt();
    checksum += test_mixed_complex();
    checksum += test_long_double();
    
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
