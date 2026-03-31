/* test_x86_conditions.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>

/* Prevent optimizations from removing our test code */
#define NOINLINE __attribute__((noinline, noipa))

/* Global volatile variables to prevent constant folding */
volatile double g_nan = NAN;
volatile double g_inf = INFINITY;
volatile double g_neg_inf = -INFINITY;
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile double g_two = 2.0;

/* Test 1: Direct unordered comparisons that should generate UNORDERED/ORDERED */
NOINLINE int test_unordered_ordered(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_two;
    
    int result = 0;
    
    /* These should generate UNORDERED condition code */
    if (a != a) result |= 1;      /* NaN != NaN -> true (unordered) */
    if (a != b) result |= 2;      /* NaN != 1.0 -> true (unordered) */
    if (b != a) result |= 4;      /* 1.0 != NaN -> true (unordered) */
    
    /* These should generate ORDERED condition code */
    if (b == b) result |= 8;      /* 1.0 == 1.0 -> true (ordered) */
    if (c == c) result |= 16;     /* 2.0 == 2.0 -> true (ordered) */
    
    /* Mixed ordered/unordered comparisons */
    if (!(a == a)) result |= 32;  /* !(NaN == NaN) -> true */
    if (!(b != b)) result |= 64;  /* !(1.0 != 1.0) -> true */
    
    return result;
}

/* Test 2: Using math.h comparison macros */
NOINLINE int test_math_macros(void) {
    volatile double nan = g_nan;
    volatile double one = g_one;
    volatile double two = g_two;
    volatile double inf = g_inf;
    
    int result = 0;
    
    /* UNEQ: unordered or equal */
    if (isunordered(nan, one) || nan == one) result |= 1;
    
    /* UNGE: unordered or greater-or-equal */
    if (isunordered(nan, one) || nan >= one) result |= 2;
    
    /* UNGT: unordered or greater */
    if (isunordered(nan, one) || nan > one) result |= 4;
    
    /* UNLE: unordered or less-or-equal */
    if (isunordered(nan, one) || nan <= one) result |= 8;
    
    /* UNLT: unordered or less */
    if (isunordered(nan, one) || nan < one) result |= 16;
    
    /* LTGT: less or greater (but not equal, not unordered) */
    if (islessgreater(one, two)) result |= 32;
    if (islessgreater(two, one)) result |= 64;
    
    /* Test with infinity */
    if (isgreater(inf, one)) result |= 128;
    if (isless(one, inf)) result |= 256;
    
    return result;
}

/* Test 3: Inline assembly with %C modifier */
NOINLINE int test_inline_asm(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    int result = 0;
    
    /* Test various condition codes via inline assembly */
    #ifdef __x86_64__
    /* Using x87 floating point compare */
    __asm__ volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "set%C0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b)
        : "cc", "st"
    );
    #endif
    
    return result;
}

/* Test 4: SSE2 comparisons using builtins */
NOINLINE int test_sse2_comparisons(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_two;
    int result = 0;
    
    #ifdef __SSE2__
    /* Direct SSE2 unordered compare */
    if (__builtin_ia32_ucomisd(a, b)) result |= 1;
    if (__builtin_ia32_ucomisd(b, a)) result |= 2;
    if (__builtin_ia32_ucomisd(b, c)) result |= 4;
    if (__builtin_ia32_ucomisd(c, b)) result |= 8;
    
    /* Ordered compare */
    if (__builtin_ia32_comisd(b, c)) result |= 16;
    if (__builtin_ia32_comisd(c, b)) result |= 32;
    #endif
    
    return result;
}

/* Test 5: Long double (x87) comparisons */
NOINLINE int test_long_double(void) {
    volatile long double ld_nan = g_nan;
    volatile long double ld_one = g_one;
    volatile long double ld_two = g_two;
    int result = 0;
    
    /* Long double comparisons often use x87 instructions */
    if (ld_nan != ld_nan) result |= 1;
    if (ld_one == ld_one) result |= 2;
    if (ld_one != ld_two) result |= 4;
    if (ld_one < ld_two) result |= 8;
    if (ld_two > ld_one) result |= 16;
    
    /* Unordered comparisons with long double */
    if (isunordered(ld_nan, ld_one)) result |= 32;
    if (!isunordered(ld_one, ld_two)) result |= 64;
    
    return result;
}

/* Test 6: Array-based comparisons to force loop generation */
NOINLINE int test_array_comparisons(void) {
    volatile double arr1[8];
    volatile double arr2[8];
    int counts[8] = {0};
    
    /* Initialize with mix of values */
    for (int i = 0; i < 8; i++) {
        if (i % 4 == 0) {
            arr1[i] = g_nan;
            arr2[i] = i * 1.0;
        } else if (i % 4 == 1) {
            arr1[i] = i * 1.0;
            arr2[i] = g_nan;
        } else if (i % 4 == 2) {
            arr1[i] = i * 1.0;
            arr2[i] = (i + 1) * 1.0;
        } else {
            arr1[i] = i * 1.0;
            arr2[i] = i * 1.0;
        }
    }
    
    /* Perform various comparisons in a loop */
    for (int i = 0; i < 8; i++) {
        if (isunordered(arr1[i], arr2[i])) counts[0]++;
        if (isgreater(arr1[i], arr2[i])) counts[1]++;
        if (isless(arr1[i], arr2[i])) counts[2]++;
        if (isgreaterequal(arr1[i], arr2[i])) counts[3]++;
        if (islessequal(arr1[i], arr2[i])) counts[4]++;
        if (islessgreater(arr1[i], arr2[i])) counts[5]++;
    }
    
    /* Aggregate results */
    int result = 0;
    for (int i = 0; i < 6; i++) {
        result = (result << 4) | (counts[i] & 0xF);
    }
    
    return result;
}

/* Test 7: Switch statement based on comparison results */
NOINLINE int test_switch_comparisons(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    int result = 0;
    
    /* Force generation of multiple condition codes */
    if (isunordered(a, b)) {
        result = 1;  /* UNORDERED */
    } else if (a == b) {
        result = 2;  /* EQ */
    } else if (a > b) {
        result = 3;  /* GT */
    } else if (a < b) {
        result = 4;  /* LT */
    }
    
    /* Another switch-like structure */
    volatile double c = g_two;
    if (islessgreater(b, c)) {
        result |= 8;  /* LTGT */
    }
    if (!isunordered(b, c) && b != c) {
        result |= 16; /* NE (ordered) */
    }
    
    return result;
}

/* Test 8: Conditional moves based on FP comparisons */
NOINLINE int test_conditional_moves(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_two;
    int r1, r2, r3;
    
    /* These might generate conditional moves with condition codes */
    r1 = (a != a) ? 100 : 200;      /* UNORDERED */
    r2 = (b == c) ? 300 : 400;      /* EQ (false) */
    r3 = (b < c) ? 500 : 600;       /* LT (true) */
    
    /* Force use of results */
    volatile int dummy = r1 + r2 + r3;
    
    /* More complex conditional expressions */
    double d = isunordered(a, b) ? g_nan : g_one;
    double e = isgreater(b, c) ? g_two : g_one;
    
    /* Use results to prevent optimization */
    return (int)(d + e) + r1 + r2 + r3;
}

int main(void) {
    int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Run all tests and accumulate checksum */
    checksum ^= test_unordered_ordered();
    checksum ^= test_math_macros() << 1;
    checksum ^= test_inline_asm() << 2;
    checksum ^= test_sse2_comparisons() << 3;
    checksum ^= test_long_double() << 4;
    checksum ^= test_array_comparisons() << 5;
    checksum ^= test_switch_comparisons() << 6;
    checksum ^= test_conditional_moves() << 7;
    
    printf("Final checksum: %d\n", checksum);
    printf("(Non-zero checksum indicates code was executed)\n");
    
    return 0;
}
