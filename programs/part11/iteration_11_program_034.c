/* test_i386_condition_codes.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent optimization */
extern void sink(int);
extern void sink_ptr(void*);

/* Force non-inlined functions */
#define NOINLINE __attribute__((noinline))

/* Global volatile variables to prevent constant folding */
volatile double g_nan = 0.0/0.0;
volatile double g_inf = 1.0/0.0;
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile double g_two = 2.0;

/* Array of volatile values */
volatile double varr[8] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};

/* Struct with volatile members */
struct volatile_doubles {
    volatile double a;
    volatile double b;
    volatile double c;
};

volatile struct volatile_doubles g_struct = {0.0, 0.0, 0.0};

/* Function 1: UNORDERED and ORDERED patterns */
NOINLINE int test_unordered_ordered(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_two;
    volatile float f1 = g_nan;
    volatile float f2 = g_one;
    volatile long double ld1 = g_nan;
    volatile long double ld2 = g_one;
    
    int result = 0;
    
    /* UNORDERED: Using isunordered() */
    if (isunordered(a, b)) {
        result |= 1;
    }
    
    /* UNORDERED: Direct NaN comparison */
    if (a != a) {  /* NaN != NaN is true */
        result |= 2;
    }
    
    /* ORDERED: Using isordered() */
    if (isordered(b, c)) {
        result |= 4;
    }
    
    /* ORDERED: Direct comparison */
    if (b == b) {  /* Non-NaN == itself */
        result |= 8;
    }
    
    /* Mixed types for unordered */
    if (isunordered(f1, f2)) {
        result |= 16;
    }
    
    /* Long double unordered */
    if (isunordered(ld1, ld2)) {
        result |= 32;
    }
    
    /* Inline assembly to force condition code use */
    volatile double x = g_nan;
    volatile double y = g_one;
    
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l1"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : unordered_label
    );
    
    result |= 64;
    goto skip_unordered;
    
unordered_label:
    result |= 128;
    
skip_unordered:
    
    sink(result);
    return result;
}

/* Function 2: UNEQ (unordered or equal) */
NOINLINE int test_uneq(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_one;
    
    int result = 0;
    
    /* UNEQ: !(a != b) which is a == b OR unordered */
    if (!(a != b)) {
        result |= 1;
    }
    
    /* Using memory operands from array */
    if (!(varr[0] != varr[1])) {
        result |= 2;
    }
    
    /* Using struct members */
    g_struct.a = g_nan;
    g_struct.b = g_one;
    if (!(g_struct.a != g_struct.b)) {
        result |= 4;
    }
    
    /* Complex expression with UNEQ */
    result += ((!(a != b)) ? 8 : 0);
    
    sink(result);
    return result;
}

/* Function 3: UNGE (not less than) = !(a < b) */
NOINLINE int test_unge(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    
    int result = 0;
    
    /* UNGE: !(a < b) */
    if (!(a < b)) {
        result |= 1;
    }
    
    /* UNGE with NaN (should be true for unordered) */
    if (!(c < b)) {
        result |= 2;
    }
    
    /* Using array elements */
    if (!(varr[2] < varr[3])) {
        result |= 4;
    }
    
    /* Nested in larger expression */
    int temp = (!(a < b)) + (!(b < a)) * 2;
    result |= (temp << 3);
    
    /* Inline assembly to force condition code */
    volatile double x = varr[4];
    volatile double y = varr[5];
    
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l1"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : unge_label
    );
    
    result |= 32;
    goto skip_unge;
    
unge_label:
    result |= 64;
    
skip_unge:
    
    sink(result);
    return result;
}

/* Function 4: UNGT (not less than or equal) = !(a <= b) */
NOINLINE int test_ungt(void) {
    volatile double a = g_two;
    volatile double b = g_one;
    volatile double c = g_nan;
    
    int result = 0;
    
    /* UNGT: !(a <= b) */
    if (!(a <= b)) {
        result |= 1;
    }
    
    /* With NaN */
    if (!(c <= b)) {
        result |= 2;
    }
    
    /* Complex logical expression */
    if (!(a <= b) && (a > b)) {
        result |= 4;
    }
    
    /* Using memory indirect */
    volatile double* ptr1 = &varr[6];
    volatile double* ptr2 = &varr[7];
    if (!(*ptr1 <= *ptr2)) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* Function 5: UNLE (unordered or less than or equal) */
NOINLINE int test_unle(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    
    int result = 0;
    
    /* UNLE: !(a > b) */
    if (!(a > b)) {
        result |= 1;
    }
    
    /* With NaN */
    if (!(c > b)) {
        result |= 2;
    }
    
    /* Combined with other conditions */
    if (!(a > b) || (a <= b)) {
        result |= 4;
    }
    
    /* Ternary operator */
    result += (!(varr[0] > varr[1])) ? 8 : 0;
    
    sink(result);
    return result;
}

/* Function 6: UNLT (unordered or less than) */
NOINLINE int test_unlt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    
    int result = 0;
    
    /* UNLT: !(a >= b) */
    if (!(a >= b)) {
        result |= 1;
    }
    
    /* With NaN */
    if (!(c >= b)) {
        result |= 2;
    }
    
    /* Nested in expression */
    int temp = (!(a >= b)) ? 4 : 8;
    result |= temp;
    
    /* Using struct members */
    g_struct.a = g_one;
    g_struct.b = g_two;
    if (!(g_struct.a >= g_struct.b)) {
        result |= 16;
    }
    
    sink(result);
    return result;
}

/* Function 7: LTGT (not equal and ordered) */
NOINLINE int test_ltgt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    volatile double d = g_one;
    
    int result = 0;
    
    /* LTGT: (a < b) || (a > b) */
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    /* With equal values (should be false) */
    if ((a < d) || (a > d)) {
        result |= 2;
    }
    
    /* With NaN (should be false) */
    if ((c < b) || (c > b)) {
        result |= 4;
    }
    
    /* Complex expression */
    if (((a < b) || (a > b)) && !(a == b)) {
        result |= 8;
    }
    
    /* Using array elements */
    if ((varr[0] < varr[1]) || (varr[0] > varr[1])) {
        result |= 16;
    }
    
    /* Inline assembly for LTGT */
    volatile double x = g_two;
    volatile double y = g_one;
    
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l1"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : ltgt_label
    );
    
    result |= 32;
    goto skip_ltgt;
    
ltgt_label:
    result |= 64;
    
skip_ltgt:
    
    sink(result);
    return result;
}

/* Function 8: Mixed condition codes in complex expressions */
NOINLINE int test_mixed_conditions(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_two;
    volatile double d = g_one;
    
    int result = 0;
    
    /* Combination of multiple condition codes */
    result = (a != a) ? 1 : 0;                     /* UNORDERED */
    result += isordered(b, c) ? 2 : 0;             /* ORDERED */
    result += (!(a != b)) ? 4 : 0;                 /* UNEQ */
    result += (!(b < c)) ? 8 : 0;                  /* UNGE */
    result += (!(c <= b)) ? 16 : 0;                /* UNGT */
    result += (!(b > c)) ? 32 : 0;                 /* UNLE */
    result += (!(b >= c)) ? 64 : 0;                /* UNLT */
    result += ((b < c) || (b > c)) ? 128 : 0;      /* LTGT */
    
    /* Nested ternary with different types */
    volatile float f1 = g_nan;
    volatile float f2 = g_one;
    result += (isunordered(f1, f2) ? 256 : 
              (isordered(f1, f2) ? 512 : 
              (!(f1 != f2) ? 1024 : 0)));
    
    /* Long double comparisons */
    volatile long double ld1 = g_nan;
    volatile long double ld2 = g_one;
    volatile long double ld3 = g_two;
    
    if (isunordered(ld1, ld2)) result += 2048;
    if (!(ld2 < ld3)) result += 4096;
    if ((ld2 < ld3) || (ld2 > ld3)) result += 8192;
    
    sink(result);
    return result;
}

/* Main function */
int main(void) {
    int checksum = 0;
    
    /* Initialize array with some NaN values */
    varr[2] = 0.0/0.0;  /* NaN */
    varr[5] = 1.0/0.0;  /* Inf */
    
    /* Initialize struct */
    g_struct.c = 0.0/0.0;  /* NaN */
    
    /* Call all test functions */
    checksum += test_unordered_ordered();
    checksum += test_uneq();
    checksum += test_unge();
    checksum += test_ungt();
    checksum += test_unle();
    checksum += test_unlt();
    checksum += test_ltgt();
    checksum += test_mixed_conditions();
    
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
