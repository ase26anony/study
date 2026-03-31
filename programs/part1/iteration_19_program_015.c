/* Test program to trigger x86 floating-point condition code printing */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent optimizations from removing critical code */
#define NOINLINE __attribute__((noinline, noipa))

/* Global volatile variables to prevent constant folding */
volatile double g_nan = NAN;
volatile double g_inf = INFINITY;
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile double g_neg_one = -1.0;

/* Test 1: Direct unordered comparisons with NaN */
NOINLINE int test_unordered_comparisons(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    volatile double d = g_nan;
    
    int results[8] = {0};
    
    /* These should generate UNORDERED/ORDERED condition codes */
    results[0] = (a != b);  /* UNORDERED when a is NaN */
    results[1] = (a == a);  /* UNORDERED when a is NaN */
    results[2] = (b == b);  /* ORDERED when b is normal */
    results[3] = (c != d);  /* UNORDERED when d is NaN */
    
    /* Force use of different condition codes */
    results[4] = isunordered(a, b);
    results[5] = isordered(b, c);
    results[6] = !isunordered(c, d);
    results[7] = !isordered(a, d);
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 2: Inline assembly with %C modifier for condition codes */
NOINLINE int test_asm_condition_codes(void) {
    volatile double x = g_nan;
    volatile double y = g_one;
    volatile double z = g_zero;
    volatile double w = g_inf;
    
    int results[12] = {0};
    uint8_t byte_result;
    
    /* Test various condition codes through inline assembly */
    
    /* UNORDERED/ORDERED tests */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(byte_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    results[0] = byte_result;
    
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(byte_result)
        : "x"(y), "x"(z)
        : "cc"
    );
    results[1] = byte_result;
    
    /* UNEQ test (unordered or equal) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(byte_result)
        : "x"(z), "x"(z)  /* equal values */
        : "cc"
    );
    results[2] = byte_result;
    
    /* UNGE test (not less than) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(byte_result)
        : "x"(w), "x"(y)  /* INF > 1.0 */
        : "cc"
    );
    results[3] = byte_result;
    
    /* UNGT test (not less than or equal) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(byte_result)
        : "x"(w), "x"(g_neg_one)  /* INF > -1.0 */
        : "cc"
    );
    results[4] = byte_result;
    
    /* UNLE test (unordered or less than or equal) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(byte_result)
        : "x"(g_neg_one), "x"(w)  /* -1.0 < INF */
        : "cc"
    );
    results[5] = byte_result;
    
    /* UNLT test (unordered or less than) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(byte_result)
        : "x"(g_neg_one), "x"(g_one)  /* -1.0 < 1.0 */
        : "cc"
    );
    results[6] = byte_result;
    
    /* LTGT test (less than or greater than) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(byte_result)
        : "x"(g_one), "x"(g_neg_one)  /* 1.0 > -1.0 */
        : "cc"
    );
    results[7] = byte_result;
    
    /* Test with x87 instructions for long double */
    volatile long double ld1 = g_nan;
    volatile long double ld2 = g_one;
    
    __asm__ volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "set%C0 %0"
        : "=r"(byte_result)
        : "m"(ld1), "m"(ld2)
        : "cc", "st"
    );
    results[8] = byte_result;
    
    /* More x87 tests with different condition codes */
    __asm__ volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "set%C0 %0"
        : "=r"(byte_result)
        : "m"(ld2), "m"(ld1)
        : "cc", "st"
    );
    results[9] = byte_result;
    
    /* Test with ordered comparison */
    volatile long double ld3 = g_zero;
    __asm__ volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "set%C0 %0"
        : "=r"(byte_result)
        : "m"(ld3), "m"(ld3)
        : "cc", "st"
    );
    results[10] = byte_result;
    
    /* Test with UNEQ on x87 */
    __asm__ volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "set%C0 %0"
        : "=r"(byte_result)
        : "m"(ld3), "m"(ld3)
        : "cc", "st"
    );
    results[11] = byte_result;
    
    int sum = 0;
    for (int i = 0; i < 12; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 3: Array operations with various comparison macros */
NOINLINE int test_array_comparisons(void) {
    volatile double arr1[16];
    volatile double arr2[16];
    
    /* Initialize arrays with mix of values */
    for (int i = 0; i < 16; i++) {
        if (i % 5 == 0) {
            arr1[i] = g_nan;
            arr2[i] = i * 0.5;
        } else if (i % 3 == 0) {
            arr1[i] = g_inf;
            arr2[i] = -g_inf;
        } else if (i % 2 == 0) {
            arr1[i] = i * 1.5;
            arr2[i] = g_nan;
        } else {
            arr1[i] = i * 0.7;
            arr2[i] = i * 0.7;
        }
    }
    
    int counts[8] = {0};
    
    /* Count various comparison results */
    for (int i = 0; i < 16; i++) {
        counts[0] += isunordered(arr1[i], arr2[i]);   /* UNORDERED */
        counts[1] += isordered(arr1[i], arr2[i]);     /* ORDERED */
        counts[2] += !isgreater(arr1[i], arr2[i]) && !isless(arr1[i], arr2[i]) 
                     && isordered(arr1[i], arr2[i]);  /* UNEQ approximation */
        counts[3] += !isless(arr1[i], arr2[i]);       /* UNGE */
        counts[4] += !isless(arr1[i], arr2[i]) && !(arr1[i] == arr2[i]); /* UNGT approximation */
        counts[5] += isless(arr1[i], arr2[i]) || isunordered(arr1[i], arr2[i]); /* UNLE */
        counts[6] += isless(arr1[i], arr2[i]) || isunordered(arr1[i], arr2[i]); /* UNLT (similar) */
        counts[7] += isless(arr1[i], arr2[i]) || isgreater(arr1[i], arr2[i]);   /* LTGT */
    }
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += counts[i];
    }
    return sum;
}

/* Test 4: Complex control flow based on floating-point comparisons */
NOINLINE int test_complex_control_flow(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    volatile double d = g_inf;
    
    int result = 0;
    
    /* Switch-like behavior using condition codes */
    if (isunordered(a, b)) {
        result += 1;  /* UNORDERED path */
    }
    
    if (isordered(b, c)) {
        result += 2;  /* ORDERED path */
    }
    
    if (!isless(a, b) && !isgreater(a, b) && isunordered(a, b)) {
        result += 4;  /* UNEQ-like path */
    }
    
    if (!isless(d, b)) {
        result += 8;  /* UNGE path */
    }
    
    if (!isless(d, b) && d != b) {
        result += 16; /* UNGT path */
    }
    
    if (isless(b, d) || isunordered(b, d)) {
        result += 32; /* UNLE path */
    }
    
    if (isless(b, d) || isunordered(b, d)) {
        result += 64; /* UNLT path */
    }
    
    if (isless(c, b) || isgreater(c, b)) {
        result += 128; /* LTGT path */
    }
    
    /* Additional tests with long double for x87 */
    volatile long double ld1 = g_nan;
    volatile long double ld2 = g_one;
    volatile long double ld3 = g_zero;
    
    if (ld1 != ld2) {  /* Should generate unordered compare */
        result += 256;
    }
    
    if (ld2 == ld2) {  /* Should generate ordered compare */
        result += 512;
    }
    
    if (ld3 <= ld2) {  /* Should generate UNLE condition */
        result += 1024;
    }
    
    if (ld2 >= ld3) {  /* Should generate UNGE condition */
        result += 2048;
    }
    
    return result;
}

/* Test 5: Mixed SSE and x87 operations */
NOINLINE int test_mixed_fpu_operations(void) {
    volatile double d1 = g_nan;
    volatile double d2 = g_one;
    volatile long double ld1 = g_nan;
    volatile long double ld2 = g_one;
    
    int results[6] = {0};
    
    /* SSE2 double comparison */
    results[0] = (d1 > d2) ? 1 : 0;
    results[1] = (d1 < d2) ? 1 : 0;
    results[2] = (d1 == d2) ? 1 : 0;
    results[3] = (d1 != d2) ? 1 : 0;
    
    /* x87 long double comparison */
    results[4] = (ld1 > ld2) ? 1 : 0;
    results[5] = (ld1 < ld2) ? 1 : 0;
    
    /* Force use of condition codes in conditional moves */
    volatile double d3 = g_zero;
    volatile double d4 = g_one;
    double cmov_result;
    
    __asm__ volatile (
        "ucomisd %2, %3\n\t"
        "movapd %1, %0\n\t"
        "cmov%C0pd %4, %0"
        : "=x"(cmov_result)
        : "x"(d3), "x"(d4), "x"(d3), "x"(d4)
        : "cc"
    );
    
    int sum = 0;
    for (int i = 0; i < 6; i++) {
        sum += results[i];
    }
    sum += (int)cmov_result;
    
    return sum;
}

int main(void) {
    int total = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Run all tests to trigger various condition code generation */
    total += test_unordered_comparisons();
    total += test_asm_condition_codes();
    total += test_array_comparisons();
    total += test_complex_control_flow();
    total += test_mixed_fpu_operations();
    
    printf("Total checksum: %d\n", total);
    
    /* Use results to prevent dead code elimination */
    volatile int sink = total;
    
    return 0;
}
