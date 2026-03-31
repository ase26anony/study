/* test_i386_condition_codes.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent optimization */
extern void sink(int);
extern void sink_ptr(void*);

/* Volatile variables to prevent constant folding */
volatile double g_dnan = 0.0/0.0;
volatile double g_dinf = 1.0/0.0;
volatile double g_dneg = -1.0;
volatile double g_dpos = 1.0;
volatile float g_fnan = 0.0f/0.0f;
volatile float g_finf = 1.0f/0.0f;
volatile long double g_ldnan = 0.0L/0.0L;

/* Arrays for memory operand variations */
volatile double darr[4] = {0.0, 1.0, 2.0, 3.0};
volatile float farr[4] = {0.0f, 1.0f, 2.0f, 3.0f};

/* Struct for complex memory access */
struct fp_pair {
    volatile double a;
    volatile double b;
};
volatile struct fp_pair fp_struct = {0.0, 1.0};

/* ========== UNORDERED/ORDERED patterns ========== */

__attribute__((noinline))
int test_unordered_ordered(void) {
    volatile double d1 = g_dnan;
    volatile double d2 = g_dpos;
    volatile float f1 = g_fnan;
    volatile long double ld1 = g_ldnan;
    
    int result = 0;
    
    /* Direct NaN comparisons for UNORDERED */
    if (d1 != d1) {  /* Should generate UNORDERED */
        result |= 1;
    }
    
    if (isunordered(f1, f1)) {  /* Should generate UNORDERED */
        result |= 2;
    }
    
    /* ORDERED comparisons */
    if (d2 == d2) {  /* Should generate ORDERED */
        result |= 4;
    }
    
    if (isordered(ld1, d2)) {  /* Should generate ORDERED */
        result |= 8;
    }
    
    /* Complex expression with ternary */
    result += (d1 != d1) ? 16 : ((d2 == d2) ? 32 : 0);
    
    sink(result);
    return result;
}

/* ========== UNEQ (unordered or equal) ========== */

__attribute__((noinline))
int test_uneq(void) {
    volatile double a = g_dnan;
    volatile double b = g_dpos;
    volatile double c = g_dpos;
    
    int result = 0;
    
    /* UNEQ: !(a < b) && !(a > b) */
    if (!(a < b) && !(a > b)) {  /* NaN comparison - should be UNEQ */
        result |= 1;
    }
    
    if (!(b < c) && !(b > c)) {  /* Equal values - should be UNEQ */
        result |= 2;
    }
    
    /* Using memory operands */
    if (!(darr[0] < darr[1]) && !(darr[0] > darr[1])) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* ========== UNGE (not less than) ========== */

__attribute__((noinline))
int test_unge(void) {
    volatile double a = g_dnan;
    volatile double b = g_dpos;
    volatile double c = g_dneg;
    
    int result = 0;
    
    /* UNGE: !(a < b) */
    if (!(a < b)) {  /* NaN comparison - should be UNGE */
        result |= 1;
    }
    
    if (!(c < b)) {  /* -1 < 1 is true, so !(c < b) is false */
        result |= 2;
    }
    
    if (!(b < c)) {  /* 1 < -1 is false, so !(b < c) is true */
        result |= 4;
    }
    
    /* Complex expression */
    result += (!(fp_struct.a < fp_struct.b)) ? 8 : 0;
    
    sink(result);
    return result;
}

/* ========== UNGT (not less than or equal) ========== */

__attribute__((noinline))
int test_ungt(void) {
    volatile double a = g_dnan;
    volatile double b = g_dpos;
    volatile double c = g_dpos;
    
    int result = 0;
    
    /* UNGT: !(a <= b) */
    if (!(a <= b)) {  /* NaN comparison - should be UNGT */
        result |= 1;
    }
    
    if (!(b <= c)) {  /* Equal values - should be false */
        result |= 2;
    }
    
    /* Using array elements */
    if (!(darr[0] <= darr[1])) {
        result |= 4;
    }
    
    if (!(darr[1] <= darr[0])) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* ========== UNLE (unordered or less than or equal) ========== */

__attribute__((noinline))
int test_unle(void) {
    volatile double a = g_dnan;
    volatile double b = g_dpos;
    volatile double c = g_dneg;
    
    int result = 0;
    
    /* UNLE: !(a > b) */
    if (!(a > b)) {  /* NaN comparison - should be UNLE */
        result |= 1;
    }
    
    if (!(c > b)) {  /* -1 > 1 is false, so !(c > b) is true */
        result |= 2;
    }
    
    if (!(b > c)) {  /* 1 > -1 is true, so !(b > c) is false */
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* ========== UNLT (unordered or less than) ========== */

__attribute__((noinline))
int test_unlt(void) {
    volatile double a = g_dnan;
    volatile double b = g_dpos;
    volatile double c = g_dneg;
    
    int result = 0;
    
    /* UNLT: !(a >= b) */
    if (!(a >= b)) {  /* NaN comparison - should be UNLT */
        result |= 1;
    }
    
    if (!(c >= b)) {  /* -1 >= 1 is false, so !(c >= b) is true */
        result |= 2;
    }
    
    if (!(b >= c)) {  /* 1 >= -1 is true, so !(b >= c) is false */
        result |= 4;
    }
    
    /* Mixed float/double */
    volatile float f1 = g_fnan;
    if (!(f1 >= b)) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* ========== LTGT (not equal and ordered) ========== */

__attribute__((noinline))
int test_ltgt(void) {
    volatile double a = g_dnan;
    volatile double b = g_dpos;
    volatile double c = g_dneg;
    volatile double d = g_dpos;
    
    int result = 0;
    
    /* LTGT: (a < b) || (a > b) */
    if ((a < b) || (a > b)) {  /* NaN comparison - should be false */
        result |= 1;
    }
    
    if ((c < b) || (c > b)) {  /* -1 vs 1 - should be true */
        result |= 2;
    }
    
    if ((b < d) || (b > d)) {  /* Equal values - should be false */
        result |= 4;
    }
    
    /* Complex nested expression */
    result += ((farr[0] < farr[1]) || (farr[0] > farr[1])) ? 8 : 0;
    
    sink(result);
    return result;
}

/* ========== Inline assembly to force condition codes ========== */

__attribute__((noinline))
int test_asm_condition_codes(void) {
    volatile double a = g_dnan;
    volatile double b = g_dpos;
    volatile double c = g_dneg;
    int result = 0;
    
    /* Force UNORDERED condition code with inline asm goto */
    if (a != a) {
        __asm__ goto ("j%c0 %l0" : : "i" (0) : : label_unordered);
        result |= 1;
        goto skip_unordered;
    label_unordered:
        result |= 2;
    skip_unordered:;
    }
    
    /* Force ORDERED condition code */
    if (b == b) {
        __asm__ goto ("j%c0 %l0" : : "i" (0) : : label_ordered);
        result |= 4;
        goto skip_ordered;
    label_ordered:
        result |= 8;
    skip_ordered:;
    }
    
    /* Force UNEQ condition code */
    if (!(a < b) && !(a > b)) {
        __asm__ goto ("j%c0 %l0" : : "i" (0) : : label_uneq);
        result |= 16;
        goto skip_uneq;
    label_uneq:
        result |= 32;
    skip_uneq:;
    }
    
    /* Force UNGE condition code */
    if (!(c < b)) {
        __asm__ goto ("j%c0 %l0" : : "i" (0) : : label_unge);
        result |= 64;
        goto skip_unge;
    label_unge:
        result |= 128;
    skip_unge:;
    }
    
    sink(result);
    return result;
}

/* ========== Complex mixed expressions ========== */

__attribute__((noinline))
int test_complex_mixed(void) {
    volatile double d1 = g_dnan;
    volatile double d2 = g_dpos;
    volatile float f1 = g_fnan;
    volatile long double ld1 = g_ldnan;
    
    int result = 0;
    
    /* Nested ternary with multiple condition codes */
    result = (d1 != d1) ? 
                ((d2 > 0.0) ? 1 : 2) : 
                ((f1 == f1) ? 3 : 4);
    
    /* Logical AND/OR combination */
    if ((d1 != d1) && (d2 == d2)) {
        result += 10;
    }
    
    if ((!(darr[0] < darr[1])) || (farr[0] != farr[0])) {
        result += 20;
    }
    
    /* Multiple comparisons in single expression */
    int temp = ((d1 < d2) ? 100 : 200) + 
               ((!(d2 <= d1)) ? 300 : 400) +
               (((d1 > d2) || (d1 < d2)) ? 500 : 600);
    
    result += temp % 256;
    
    sink(result);
    return result;
}

/* ========== Main function ========== */

int main(void) {
    int checksum = 0;
    
    /* Initialize volatile values */
    g_dnan = 0.0/0.0;
    g_dinf = 1.0/0.0;
    g_dneg = -1.0;
    g_dpos = 1.0;
    g_fnan = 0.0f/0.0f;
    g_finf = 1.0f/0.0f;
    g_ldnan = 0.0L/0.0L;
    
    darr[0] = 0.0; darr[1] = 1.0; darr[2] = 2.0; darr[3] = 3.0;
    farr[0] = 0.0f; farr[1] = 1.0f; farr[2] = 2.0f; farr[3] = 3.0f;
    fp_struct.a = 0.0; fp_struct.b = 1.0;
    
    /* Call all test functions */
    checksum += test_unordered_ordered();
    checksum += test_uneq();
    checksum += test_unge();
    checksum += test_ungt();
    checksum += test_unle();
    checksum += test_unlt();
    checksum += test_ltgt();
    checksum += test_asm_condition_codes();
    checksum += test_complex_mixed();
    
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
