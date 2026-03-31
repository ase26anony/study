/* Test program to trigger x86 floating-point condition code output */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Prevent optimizations from removing critical code */
#define NOINLINE __attribute__((noinline, noipa))

/* Volatile variables to prevent constant folding */
static volatile double dnan = NAN;
static volatile double dinf = INFINITY;
static volatile double dneg = -1.0;
static volatile double dpos = 1.0;
static volatile double dzero = 0.0;

static volatile long double ldnan = NAN;
static volatile long double ldinf = INFINITY;
static volatile long double ldneg = -1.0L;
static volatile long double ldpos = 1.0L;
static volatile long double ldzero = 0.0L;

/* Test 1: Direct unordered comparisons with NaN */
NOINLINE int test_unordered_comparisons(void) {
    int result = 0;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    if (dnan != dnan) result |= 1;      /* UNORDERED */
    if (dnan == dnan) result |= 2;      /* ORDERED (false for NaN) */
    if (!(dnan == dnan)) result |= 4;   /* UNORDERED */
    
    /* Mixed comparisons */
    if (dnan < dpos) result |= 8;       /* UNORDERED */
    if (dpos > dnan) result |= 16;      /* UNORDERED */
    if (dnan <= dpos) result |= 32;     /* UNORDERED */
    if (dpos >= dnan) result |= 64;     /* UNORDERED */
    
    return result;
}

/* Test 2: Using math.h comparison macros */
NOINLINE int test_math_macros(void) {
    int result = 0;
    
    /* These macros explicitly handle NaN */
    if (isunordered(dnan, dpos)) result |= 1;      /* UNORDERED */
    if (isgreater(dpos, dneg)) result |= 2;        /* GT */
    if (isless(dneg, dpos)) result |= 4;           /* LT */
    if (islessequal(dzero, dzero)) result |= 8;    /* LE */
    if (isgreaterequal(dpos, dzero)) result |= 16; /* GE */
    
    /* UNEQ: unordered or equal */
    if (!islessgreater(dnan, dnan)) result |= 32;  /* UNEQ */
    if (!islessgreater(dpos, dpos)) result |= 64;  /* EQ (but could be UNEQ) */
    
    return result;
}

/* Test 3: Inline assembly with %C modifier */
NOINLINE int test_inline_asm(void) {
    int result = 0;
    unsigned char byte_result;
    
    /* x87 floating-point compare with condition code output */
    __asm__ volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "set%C0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(byte_result)
        : "m"(dnan), "m"(dpos)
        : "cc", "st"
    );
    result |= byte_result;
    
    /* SSE2 compare with unordered handling */
    __asm__ volatile (
        "movsd %1, %%xmm0\n\t"
        "movsd %2, %%xmm1\n\t"
        "ucomisd %%xmm1, %%xmm0\n\t"
        "set%C0 %0"
        : "=r"(byte_result)
        : "m"(dnan), "m"(dnan)
        : "xmm0", "xmm1", "cc"
    );
    result |= (byte_result << 8);
    
    return result;
}

/* Test 4: Long double (x87) comparisons */
NOINLINE int test_long_double(void) {
    int result = 0;
    
    /* Long double comparisons often use x87 instructions */
    if (ldnan != ldnan) result |= 1;
    if (ldpos > ldneg) result |= 2;
    if (ldneg < ldpos) result |= 4;
    if (ldzero <= ldzero) result |= 8;
    if (ldpos >= ldzero) result |= 16;
    
    /* Explicit unordered check */
    if (isunordered(ldnan, ldpos)) result |= 32;
    
    return result;
}

/* Test 5: Array processing with various comparisons */
NOINLINE int test_array_comparisons(void) {
    volatile double arr1[8] = {NAN, 1.0, NAN, 3.0, 5.0, NAN, 7.0, 9.0};
    volatile double arr2[8] = {NAN, 2.0, 4.0, NAN, 6.0, 8.0, NAN, 10.0};
    int counts[8] = {0};  /* Count different comparison results */
    
    for (int i = 0; i < 8; i++) {
        if (isunordered(arr1[i], arr2[i])) counts[0]++;      /* UNORDERED */
        if (isgreater(arr1[i], arr2[i])) counts[1]++;        /* GT */
        if (isless(arr1[i], arr2[i])) counts[2]++;           /* LT */
        if (!islessgreater(arr1[i], arr2[i])) counts[3]++;   /* UNEQ */
        if (!isless(arr1[i], arr2[i])) counts[4]++;          /* UNGE (nlt) */
        if (!islessequal(arr1[i], arr2[i])) counts[5]++;     /* UNGT (nle) */
        if (islessequal(arr1[i], arr2[i])) counts[6]++;      /* LE */
        if (isless(arr1[i], arr2[i])) counts[7]++;           /* LT (duplicate for pattern) */
    }
    
    /* Combine counts into a single checksum */
    int checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum = (checksum * 31 + counts[i]) & 0xFF;
    }
    return checksum;
}

/* Test 6: Switch based on comparison results */
NOINLINE int test_switch_comparisons(double a, double b) {
    int result = 0;
    
    /* Complex switch to force multiple condition codes */
    if (isunordered(a, b)) {
        result = 1;  /* UNORDERED */
    } else if (a == b) {
        result = 2;  /* EQ */
    } else if (a > b) {
        result = 3;  /* GT */
    } else if (a < b) {
        result = 4;  /* LT */
    } else if (!(a >= b)) {
        result = 5;  /* UNGE */
    } else if (!(a <= b)) {
        result = 6;  /* UNGT */
    } else {
        result = 7;
    }
    
    return result;
}

/* Test 7: Direct builtin usage */
NOINLINE int test_builtins(void) {
    int result = 0;
    
    /* GCC x86 builtins for floating-point comparisons */
    if (__builtin_isgreater(dpos, dneg)) result |= 1;
    if (__builtin_isless(dneg, dpos)) result |= 2;
    if (__builtin_isunordered(dnan, dpos)) result |= 4;
    if (__builtin_isnan(dnan)) result |= 8;
    
    /* SSE2 specific builtin */
    if (__builtin_ia32_ucomisd(dnan, dnan)) {
        /* This sets ZF,PF,CF flags for unordered */
        result |= 16;
    }
    
    return result;
}

/* Main function that runs all tests */
int main(void) {
    int total = 0;
    
    printf("Running floating-point condition code tests...\n");
    
    /* Run all tests and accumulate results */
    total += test_unordered_comparisons();
    total += test_math_macros();
    total += test_inline_asm();
    total += test_long_double();
    total += test_array_comparisons();
    
    /* Test switch with different inputs */
    total += test_switch_comparisons(dnan, dpos);
    total += test_switch_comparisons(dpos, dneg);
    total += test_switch_comparisons(dzero, dzero);
    
    total += test_builtins();
    
    printf("Total checksum: %d\n", total);
    
    /* Additional volatile operations to ensure code isn't optimized away */
    volatile double check = dnan + dpos + dneg + dzero;
    volatile long double lcheck = ldnan + ldpos + ldneg + ldzero;
    
    return (total > 0) ? 0 : 1;
}
