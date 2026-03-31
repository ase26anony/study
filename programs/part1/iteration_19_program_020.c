/* test_x86_condcodes.c
 * Compile with: gcc -O2 -march=x86-64 -mfpmath=sse -ffast-math -S test_x86_condcodes.c
 * Also try: gcc -O3 -m32 -mfpmath=387 -march=i686 -S test_x86_condcodes.c
 */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Prevent constant folding */
volatile double vd1 = 1.0;
volatile double vd2 = 2.0;
volatile double vd_nan = NAN;
volatile double vd_inf = INFINITY;
volatile long double vld1 = 1.0L;
volatile long double vld2 = 2.0L;
volatile long double vld_nan = NAN;

/* Test 1: Direct unordered comparisons that should generate UNORDERED/ORDERED */
NOINLINE int test_unordered_ordered(void) {
    int result = 0;
    
    /* These should generate UNORDERED condition code */
    if (vd_nan != vd1) result |= 1;      /* unordered comparison */
    if (vd1 != vd_nan) result |= 2;
    if (!(vd_nan == vd1)) result |= 4;   /* negated ordered comparison */
    
    /* These should generate ORDERED condition code */
    if (vd1 == vd2) result |= 8;         /* ordered comparison */
    if (!(vd1 != vd2)) result |= 16;     /* negated unordered comparison */
    
    return result;
}

/* Test 2: Using math.h comparison macros */
NOINLINE int test_math_macros(void) {
    int result = 0;
    double a = vd1;
    double b = vd2;
    double nan = vd_nan;
    
    /* UNEQ: unordered or equal */
    if (isunordered(a, nan) || a == b) result |= 1;
    
    /* UNGE: unordered or greater-or-equal (not less than) */
    if (isunordered(a, nan) || a >= b) result |= 2;
    
    /* UNGT: unordered or greater-than (not less-or-equal) */
    if (isunordered(a, nan) || a > b) result |= 4;
    
    /* UNLE: unordered or less-or-equal */
    if (isunordered(a, nan) || a <= b) result |= 8;
    
    /* UNLT: unordered or less-than */
    if (isunordered(a, nan) || a < b) result |= 16;
    
    /* LTGT: less-than or greater-than (not equal and not unordered) */
    if (islessgreater(a, b)) result |= 32;
    
    return result;
}

/* Test 3: Inline assembly with %C modifier */
NOINLINE int test_asm_condcodes(void) {
    int result = 0;
    double x = vd1;
    double y = vd_nan;
    
    /* Test UNORDERED with inline assembly */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result)
        : "x"(x), "x"(y)
        : "cc"
    );
    
    int result2;
    double a = vd2;
    double b = vd1;
    
    /* Test ORDERED with inline assembly */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result2)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    return result | (result2 << 8);
}

/* Test 4: Array operations with mixed comparisons */
NOINLINE int test_array_comparisons(void) {
    volatile double arr1[8] = {1.0, 2.0, NAN, 4.0, 5.0, NAN, 7.0, 8.0};
    volatile double arr2[8] = {1.0, 3.0, 3.0, NAN, 5.0, 6.0, NAN, 8.0};
    int counts[7] = {0}; /* UNORDERED, ORDERED, UNEQ, UNGE, UNGT, UNLE, UNLT */
    
    for (int i = 0; i < 8; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        /* UNORDERED */
        if (isunordered(a, b)) counts[0]++;
        
        /* ORDERED */
        if (!isunordered(a, b)) counts[1]++;
        
        /* UNEQ: unordered or equal */
        if (isunordered(a, b) || a == b) counts[2]++;
        
        /* UNGE: unordered or greater-or-equal (not less than) */
        if (isunordered(a, b) || a >= b) counts[3]++;
        
        /* UNGT: unordered or greater-than (not less-or-equal) */
        if (isunordered(a, b) || a > b) counts[4]++;
        
        /* UNLE: unordered or less-or-equal */
        if (isunordered(a, b) || a <= b) counts[5]++;
        
        /* UNLT: unordered or less-than */
        if (isunordered(a, b) || a < b) counts[6]++;
    }
    
    int result = 0;
    for (int i = 0; i < 7; i++) {
        result = (result << 4) | (counts[i] & 0xF);
    }
    return result;
}

/* Test 5: Long double (x87) operations */
NOINLINE int test_long_double(void) {
    volatile long double a = vld1;
    volatile long double b = vld2;
    volatile long double nan = vld_nan;
    int result = 0;
    
    /* Force x87 unordered comparisons */
    if (nan != a) result |= 1;      /* UNORDERED */
    if (a == b) result |= 2;        /* ORDERED */
    if (isunordered(a, nan) || a == b) result |= 4;  /* UNEQ */
    if (isunordered(a, nan) || a >= b) result |= 8;  /* UNGE */
    if (isunordered(a, nan) || a > b) result |= 16;  /* UNGT */
    if (isunordered(a, nan) || a <= b) result |= 32; /* UNLE */
    if (isunordered(a, nan) || a < b) result |= 64;  /* UNLT */
    if (islessgreater(a, b)) result |= 128;          /* LTGT */
    
    return result;
}

/* Test 6: Switch statement based on comparison results */
NOINLINE int test_switch_comparisons(void) {
    volatile double x = vd1;
    volatile double y = vd_nan;
    int result = 0;
    
    /* This switch should generate multiple condition code checks */
    int cmp_result = 0;
    if (isunordered(x, y)) cmp_result = 1;
    else if (x == y) cmp_result = 2;
    else if (x > y) cmp_result = 3;
    else if (x < y) cmp_result = 4;
    else if (x != y) cmp_result = 5;  /* LTGT case */
    
    switch (cmp_result) {
        case 1: result = 0x1; break;  /* UNORDERED */
        case 2: result = 0x2; break;  /* ORDERED/UNEQ */
        case 3: result = 0x4; break;  /* UNGT */
        case 4: result = 0x8; break;  /* UNLT */
        case 5: result = 0x10; break; /* LTGT */
        default: result = 0x20; break;
    }
    
    return result;
}

/* Test 7: Mixed SSE and x87 operations */
NOINLINE int test_mixed_fpu(void) {
    int result = 0;
    
    /* SSE2 double comparison */
    double sse_a = vd1;
    double sse_b = vd_nan;
    if (sse_a != sse_b) result |= 1;
    
    /* x87 long double comparison */
    long double x87_a = vld1;
    long double x87_b = vld_nan;
    if (x87_a == x87_b) result |= 2;
    
    /* Mixed: convert and compare */
    double from_x87 = (double)x87_a;
    if (from_x87 != sse_b) result |= 4;
    
    return result;
}

/* Main function that calls all tests */
int main(void) {
    int checksum = 0;
    
    checksum ^= test_unordered_ordered();
    checksum ^= test_math_macros() << 4;
    checksum ^= test_asm_condcodes() << 8;
    checksum ^= test_array_comparisons() << 12;
    checksum ^= test_long_double() << 16;
    checksum ^= test_switch_comparisons() << 20;
    checksum ^= test_mixed_fpu() << 24;
    
    /* Print to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
