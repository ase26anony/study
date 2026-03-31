/* Test program to trigger x86 floating-point condition code mnemonics */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent optimizations from removing critical code */
#define NOINLINE __attribute__((noinline, noipa))
#define VOLATILE_DOUBLE volatile double
#define VOLATILE_LONG_DOUBLE volatile long double

/* Global volatile variables to prevent constant folding */
VOLATILE_DOUBLE g_nan = NAN;
VOLATILE_DOUBLE g_inf = INFINITY;
VOLATILE_DOUBLE g_zero = 0.0;
VOLATILE_DOUBLE g_one = 1.0;
VOLATILE_DOUBLE g_two = 2.0;

/* Test 1: Direct unordered comparisons with NaN */
NOINLINE int test_unordered_comparisons(void) {
    int results[8] = {0};
    
    /* UNORDERED: Compare NaN with anything using != */
    results[0] = (g_nan != g_one) ? 1 : 0;
    
    /* ORDERED: Compare non-NaN values using == */
    results[1] = (g_one == g_one) ? 1 : 0;
    
    /* UNEQ: unordered or equal - use isunordered() and == */
    results[2] = (isunordered(g_nan, g_one) || (g_nan == g_one)) ? 1 : 0;
    
    /* UNGE: not less than - use !isless() */
    results[3] = !isless(g_nan, g_one) ? 1 : 0;
    
    /* UNGT: not less or equal - use !islessequal() */
    results[4] = !islessequal(g_nan, g_one) ? 1 : 0;
    
    /* UNLE: unordered or less or equal - use isunordered() || islessequal() */
    results[5] = (isunordered(g_one, g_nan) || islessequal(g_one, g_nan)) ? 1 : 0;
    
    /* UNLT: unordered or less than - use isunordered() || isless() */
    results[6] = (isunordered(g_one, g_nan) || isless(g_one, g_nan)) ? 1 : 0;
    
    /* LTGT: less than or greater than (unordered) - use islessgreater() */
    results[7] = islessgreater(g_nan, g_one) ? 1 : 0;
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 2: Inline assembly with %C modifier for condition codes */
NOINLINE int test_asm_condition_codes(void) {
    int results = 0;
    double a = g_nan;
    double b = g_one;
    int r;
    
    /* Test UNORDERED */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C0 %0"
        : "=r"(r)
        : "x"(a), "x"(b), "C"(4)  /* 4 = UNORDERED condition */
        : "cc"
    );
    results += r;
    
    /* Test ORDERED */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C0 %0"
        : "=r"(r)
        : "x"(a), "x"(b), "C"(7)  /* 7 = ORDERED condition */
        : "cc"
    );
    results += r;
    
    /* Test UNEQ */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C0 %0"
        : "=r"(r)
        : "x"(g_one), "x"(g_one), "C"(8)  /* 8 = UNEQ condition */
        : "cc"
    );
    results += r;
    
    return results;
}

/* Test 3: Array operations with mixed comparisons */
NOINLINE int test_array_comparisons(void) {
    VOLATILE_DOUBLE arr1[16];
    VOLATILE_DOUBLE arr2[16];
    
    /* Initialize arrays with mix of values */
    for (int i = 0; i < 16; i++) {
        if (i % 4 == 0) {
            arr1[i] = g_nan;
            arr2[i] = i * 0.5;
        } else if (i % 4 == 1) {
            arr1[i] = i * 0.5;
            arr2[i] = g_nan;
        } else if (i % 4 == 2) {
            arr1[i] = i * 0.5;
            arr2[i] = (i + 1) * 0.5;
        } else {
            arr1[i] = i * 0.5;
            arr2[i] = i * 0.5;
        }
    }
    
    int counts[7] = {0};
    
    for (int i = 0; i < 16; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        /* Count various comparison results */
        counts[0] += isunordered(a, b) ? 1 : 0;      /* UNORDERED */
        counts[1] += !isunordered(a, b) ? 1 : 0;     /* ORDERED */
        counts[2] += (isunordered(a, b) || (a == b)) ? 1 : 0; /* UNEQ */
        counts[3] += !isless(a, b) ? 1 : 0;          /* UNGE */
        counts[4] += !islessequal(a, b) ? 1 : 0;     /* UNGT */
        counts[5] += (isunordered(a, b) || islessequal(a, b)) ? 1 : 0; /* UNLE */
        counts[6] += islessgreater(a, b) ? 1 : 0;    /* LTGT */
    }
    
    int sum = 0;
    for (int i = 0; i < 7; i++) {
        sum += counts[i];
    }
    return sum;
}

/* Test 4: Long double (x87) operations */
NOINLINE int test_long_double_ops(void) {
    VOLATILE_LONG_DOUBLE ld_nan = NAN;
    VOLATILE_LONG_DOUBLE ld_one = 1.0L;
    VOLATILE_LONG_DOUBLE ld_two = 2.0L;
    
    int results = 0;
    
    /* Force x87 unordered compare */
    if (ld_nan != ld_one) results++;      /* Should generate UNORDERED */
    if (ld_one == ld_one) results++;      /* Should generate ORDERED */
    
    /* Complex expression to force condition code usage */
    long double a = ld_nan;
    long double b = ld_one;
    long double c = ld_two;
    
    /* This should generate multiple condition checks */
    if (isunordered(a, b) || (a == b)) results++;  /* UNEQ */
    if (!isless(a, b)) results++;                  /* UNGE */
    if (!islessequal(a, b)) results++;             /* UNGT */
    if (isunordered(b, a) || islessequal(b, a)) results++; /* UNLE */
    if (isunordered(a, b) || isless(a, b)) results++;      /* UNLT */
    if (islessgreater(a, b)) results++;            /* LTGT */
    
    /* Mix with regular doubles to force different FPU modes */
    double d = g_nan;
    if (isunordered(d, (double)b)) results++;
    
    return results;
}

/* Test 5: Switch statement based on comparison results */
NOINLINE int test_switch_comparisons(void) {
    VOLATILE_DOUBLE vals[8] = {NAN, INFINITY, -INFINITY, 0.0, 1.0, 2.0, -1.0, -2.0};
    int result = 0;
    
    for (int i = 0; i < 8; i++) {
        double a = vals[i];
        double b = vals[(i + 1) % 8];
        
        /* Use switch on comparison results to force multiple condition codes */
        int cmp_result;
        if (isunordered(a, b)) {
            cmp_result = 0;  /* UNORDERED */
        } else if (a == b) {
            cmp_result = 1;  /* ORDERED + equal */
        } else if (a < b) {
            cmp_result = 2;  /* Less than */
        } else {
            cmp_result = 3;  /* Greater than */
        }
        
        switch (cmp_result) {
            case 0: result += 1; break;  /* UNORDERED */
            case 1: result += 2; break;  /* ORDERED equal */
            case 2: result += 3; break;  /* Less */
            case 3: result += 4; break;  /* Greater */
        }
        
        /* Additional unordered checks */
        if (!isless(a, b)) result++;     /* UNGE */
        if (islessgreater(a, b)) result++; /* LTGT */
    }
    
    return result;
}

/* Test 6: Direct GCC builtins for unordered compares */
NOINLINE int test_builtin_comparisons(void) {
    double a = g_nan;
    double b = g_one;
    int result = 0;
    
    /* Use GCC x86 specific builtins */
    result += __builtin_isunordered(a, b) ? 1 : 0;      /* UNORDERED */
    result += !__builtin_isunordered(a, b) ? 1 : 0;     /* ORDERED */
    
    /* Complex expression that might generate UNEQ */
    if (__builtin_isunordered(a, b) || (a == b)) {
        result++;
    }
    
    /* Force generation of various condition codes through branching */
    volatile int* ptr = &result;
    if (!(a < b)) (*ptr)++;      /* UNGE */
    if (!(a <= b)) (*ptr)++;     /* UNGT */
    if (__builtin_isunordered(a, b) || (a <= b)) (*ptr)++; /* UNLE */
    if (__builtin_isunordered(a, b) || (a < b)) (*ptr)++;  /* UNLT */
    if ((a < b) || (a > b)) (*ptr)++;  /* LTGT */
    
    return result;
}

int main(void) {
    int total = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Run all tests to trigger different condition code paths */
    total += test_unordered_comparisons();
    total += test_asm_condition_codes();
    total += test_array_comparisons();
    total += test_long_double_ops();
    total += test_switch_comparisons();
    total += test_builtin_comparisons();
    
    printf("Total checksum: %d\n", total);
    
    /* Use results to prevent dead code elimination */
    volatile int sink = total;
    return sink > 0 ? 0 : 1;
}
