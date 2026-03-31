/* Test program to trigger x86 condition code printing for floating-point comparisons */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Prevent optimizations from removing critical code */
#define NOINLINE __attribute__((noinline, noipa))

/* Global volatile variables to prevent constant folding */
volatile double g_nan = NAN;
volatile double g_inf = INFINITY;
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile double g_neg_one = -1.0;

/* Test 1: Direct unordered comparisons using != and == operators */
NOINLINE int test_unordered_ordered(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    
    int result = 0;
    
    /* These should generate UNORDERED condition codes */
    if (a != b) result |= 1;      /* UNORDERED for NaN != 1.0 */
    if (a != c) result |= 2;      /* UNORDERED for NaN != 0.0 */
    
    /* These should generate ORDERED condition codes */
    if (b == c) result |= 4;      /* ORDERED for 1.0 == 0.0 (false) */
    if (b != c) result |= 8;      /* ORDERED for 1.0 != 0.0 (true) */
    
    return result;
}

/* Test 2: Using math.h comparison macros */
NOINLINE int test_math_macros(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    volatile double d = g_inf;
    
    int result = 0;
    
    /* UNEQ: unordered or equal */
    if (isunordered(a, b) || (a == b)) result |= 1;
    
    /* UNGE: unordered or greater-or-equal */
    if (isunordered(b, c) || (b >= c)) result |= 2;
    
    /* UNGT: unordered or greater-than */
    if (isunordered(d, b) || (d > b)) result |= 4;  /* INF > 1.0 */
    
    /* UNLE: unordered or less-or-equal */
    if (isunordered(c, d) || (c <= d)) result |= 8;  /* 0.0 <= INF */
    
    /* UNLT: unordered or less-than */
    if (isunordered(c, b) || (c < b)) result |= 16; /* 0.0 < 1.0 */
    
    /* LTGT: less-than or greater-than (but not equal, not unordered) */
    if ((c < b) || (c > b)) result |= 32;  /* 0.0 < 1.0 */
    
    return result;
}

/* Test 3: Inline assembly with %C modifier for condition codes */
NOINLINE int test_inline_asm(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    int result1 = 0, result2 = 0, result3 = 0;
    
    /* Test UNORDERED condition code */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C0 %0"
        : "=r"(result1)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* Test ORDERED condition code */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C0 %0"
        : "=r"(result2)
        : "x"(c), "x"(b)
        : "cc"
    );
    
    /* Test UNEQ condition code */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C0 %0"
        : "=r"(result3)
        : "x"(c), "x"(c)  /* equal values */
        : "cc"
    );
    
    return (result1 << 0) | (result2 << 8) | (result3 << 16);
}

/* Test 4: Array operations with mixed comparisons */
NOINLINE int test_array_comparisons(void) {
    volatile double arr1[8];
    volatile double arr2[8];
    int results[8];
    
    /* Initialize with mixed values including NaN */
    for (int i = 0; i < 8; i++) {
        arr1[i] = (i % 2 == 0) ? (double)i : g_nan;
        arr2[i] = (i % 3 == 0) ? (double)(i * 2) : g_one;
    }
    
    int total = 0;
    
    /* Perform various comparisons that should generate different condition codes */
    for (int i = 0; i < 8; i++) {
        int r = 0;
        
        /* Use different comparison types to trigger different condition codes */
        if (isunordered(arr1[i], arr2[i])) r |= 1;      /* UNORDERED */
        if (!isunordered(arr1[i], arr2[i])) r |= 2;     /* ORDERED */
        if (isgreater(arr1[i], arr2[i])) r |= 4;        /* UNLE with reversed operands? */
        if (isless(arr1[i], arr2[i])) r |= 8;           /* UNGE with reversed operands? */
        if (arr1[i] == arr2[i]) r |= 16;                /* UNEQ for equal, UNORDERED for NaN */
        if (arr1[i] != arr2[i]) r |= 32;                /* UNEQ for not equal */
        
        results[i] = r;
        total += r;
    }
    
    return total;
}

/* Test 5: Long double (x87) operations */
NOINLINE int test_long_double(void) {
    volatile long double a = g_nan;
    volatile long double b = 1.0L;
    volatile long double c = 0.0L;
    
    int result = 0;
    
    /* x87 comparisons may generate different condition codes */
    if (a != b) result |= 1;      /* Should be UNORDERED */
    if (b == c) result |= 2;      /* Should be ORDERED (false) */
    if (b != c) result |= 4;      /* Should be ORDERED (true) */
    
    /* Explicit x87 inline assembly */
    int asm_result = 0;
    __asm__ volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "set%C0 %0"
        : "=r"(asm_result)
        : "m"(b), "m"(c)
        : "cc", "st"
    );
    
    result |= (asm_result << 8);
    
    return result;
}

/* Test 6: Switch statement based on comparison results */
NOINLINE int test_switch_comparisons(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    volatile double d = g_inf;
    
    int result = 0;
    
    /* Force compiler to generate multiple comparison branches */
    for (int i = 0; i < 4; i++) {
        volatile double x, y;
        
        switch (i) {
            case 0: x = a; y = b; break;  /* NaN vs 1.0 */
            case 1: x = b; y = c; break;  /* 1.0 vs 0.0 */
            case 2: x = c; y = d; break;  /* 0.0 vs INF */
            case 3: x = d; y = a; break;  /* INF vs NaN */
        }
        
        /* Each comparison type may generate different condition codes */
        if (isunordered(x, y)) {
            result += 1;  /* UNORDERED */
        } else if (x == y) {
            result += 2;  /* UNEQ */
        } else if (x > y) {
            result += 4;  /* UNLE with reversed? */
        } else if (x < y) {
            result += 8;  /* UNGE with reversed? */
        } else {
            result += 16; /* Should not happen with valid FP */
        }
    }
    
    return result;
}

/* Test 7: GCC builtins for direct SSE2 unordered compares */
NOINLINE int test_gcc_builtins(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    
    int result = 0;
    
    /* Use GCC's x86-specific builtins */
    int cmp1 = __builtin_ia32_ucomisd(a, b);  /* Compare NaN with 1.0 */
    int cmp2 = __builtin_ia32_ucomisd(b, c);  /* Compare 1.0 with 0.0 */
    int cmp3 = __builtin_ia32_ucomisd(c, c);  /* Compare 0.0 with 0.0 */
    
    /* Extract condition code results */
    if (cmp1 & 1) result |= 1;   /* PF set = UNORDERED */
    if (cmp1 & 4) result |= 2;   /* ZF set = EQUAL */
    if (cmp1 & 0x40) result |= 4; /* CF set = LESS_THAN */
    
    if (cmp2 & 1) result |= 8;
    if (cmp2 & 4) result |= 16;
    if (cmp2 & 0x40) result |= 32;
    
    if (cmp3 & 1) result |= 64;
    if (cmp3 & 4) result |= 128;
    if (cmp3 & 0x40) result |= 256;
    
    return result;
}

/* Main function that runs all tests */
int main(void) {
    int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Run all tests and accumulate results */
    checksum += test_unordered_ordered();
    checksum += test_math_macros();
    checksum += test_inline_asm();
    checksum += test_array_comparisons();
    checksum += test_long_double();
    checksum += test_switch_comparisons();
    checksum += test_gcc_builtins();
    
    printf("Final checksum: %d\n", checksum);
    
    /* Use results to prevent dead code elimination */
    if (checksum == 0) {
        printf("Warning: All tests returned zero\n");
    }
    
    return 0;
}
