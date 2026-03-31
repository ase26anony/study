/* Test program to trigger x86 floating-point condition code output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

/* Prevent optimizations from removing critical code */
#define NOINLINE __attribute__((noinline, noipa))

/* Global volatile variables to prevent constant folding */
volatile double g_nan = NAN;
volatile double g_inf = INFINITY;
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile double g_neg_one = -1.0;

/* Arrays with mixed values including NaN */
volatile double arr1[16];
volatile double arr2[16];

/* Initialize arrays with special values */
void init_arrays(void) {
    for (int i = 0; i < 16; i++) {
        arr1[i] = (i % 2 == 0) ? (double)i : NAN;
        arr2[i] = (i % 3 == 0) ? NAN : (double)(i * 2);
    }
}

/* Test 1: Direct unordered comparisons using != and == operators */
NOINLINE int test_unordered_ordered(void) {
    int result = 0;
    
    /* UNORDERED: Compare NaN with normal number using != */
    if (g_nan != g_one) {
        result |= 1;
    }
    
    /* ORDERED: Compare normal numbers using == */
    if (g_one == g_one) {
        result |= 2;
    }
    
    /* More unordered comparisons */
    volatile double local_nan = NAN;
    volatile double local_num = 3.14159;
    
    if (local_nan != local_num) {
        result |= 4;
    }
    
    if (!(local_nan == local_num)) {
        result |= 8;
    }
    
    return result;
}

/* Test 2: Using math.h comparison macros */
NOINLINE int test_math_macros(void) {
    int result = 0;
    
    /* UNEQ: unordered or equal */
    if (isunordered(g_nan, g_one) || g_one == g_one) {
        result |= 1;
    }
    
    /* UNGE: unordered or greater-or-equal */
    if (isunordered(g_nan, g_one) || g_inf >= g_one) {
        result |= 2;
    }
    
    /* UNGT: unordered or greater */
    if (isunordered(g_nan, g_one) || g_inf > g_one) {
        result |= 4;
    }
    
    /* UNLE: unordered or less-or-equal */
    if (isunordered(g_nan, g_zero) || g_neg_one <= g_zero) {
        result |= 8;
    }
    
    /* UNLT: unordered or less */
    if (isunordered(g_nan, g_zero) || g_neg_one < g_zero) {
        result |= 16;
    }
    
    /* LTGT: less or greater (unordered equal returns false) */
    if (g_one < g_zero || g_one > g_zero) {
        result |= 32;
    }
    
    return result;
}

/* Test 3: Inline assembly with %C modifier for condition codes */
NOINLINE int test_inline_asm(void) {
    int result = 0;
    unsigned char byte_result;
    
    /* Test UNORDERED condition */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(byte_result)
        : "x"(g_nan), "x"(g_one)
        : "cc"
    );
    result |= (byte_result << 0);
    
    /* Test ORDERED condition */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(byte_result)
        : "x"(g_one), "x"(g_zero)
        : "cc"
    );
    result |= (byte_result << 8);
    
    /* Test UNEQ condition */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(byte_result)
        : "x"(g_one), "x"(g_one)
        : "cc"
    );
    result |= (byte_result << 16);
    
    return result;
}

/* Test 4: Loop with array comparisons - forces multiple condition code uses */
NOINLINE int test_array_comparisons(void) {
    int unordered_count = 0;
    int ordered_count = 0;
    int greater_count = 0;
    int less_count = 0;
    
    for (int i = 0; i < 16; i++) {
        /* UNORDERED check */
        if (isunordered(arr1[i], arr2[i])) {
            unordered_count++;
        }
        
        /* ORDERED check */
        if (!isunordered(arr1[i], arr2[i])) {
            ordered_count++;
        }
        
        /* UNGE: unordered or greater-or-equal */
        if (isunordered(arr1[i], arr2[i]) || arr1[i] >= arr2[i]) {
            /* Count something to use the result */
            greater_count++;
        }
        
        /* UNLE: unordered or less-or-equal */
        if (isunordered(arr1[i], arr2[i]) || arr1[i] <= arr2[i]) {
            less_count++;
        }
    }
    
    return unordered_count + (ordered_count << 8) + 
           (greater_count << 16) + (less_count << 24);
}

/* Test 5: Long double (x87) operations */
NOINLINE int test_long_double(void) {
    int result = 0;
    volatile long double ld_nan = NAN;
    volatile long double ld_one = 1.0L;
    volatile long double ld_zero = 0.0L;
    
    /* x87 style comparisons */
    if (ld_nan != ld_one) {
        result |= 1;
    }
    
    if (ld_one == ld_one) {
        result |= 2;
    }
    
    if (isgreater(ld_one, ld_zero)) {
        result |= 4;
    }
    
    if (isless(ld_zero, ld_one)) {
        result |= 8;
    }
    
    /* Complex expression that might generate LTGT */
    if ((ld_one < ld_zero) || (ld_one > ld_zero)) {
        result |= 16;
    }
    
    return result;
}

/* Test 6: Switch based on comparison results */
NOINLINE int test_switch_comparisons(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    int result = 0;
    
    /* This switch should generate multiple condition code checks */
    int cmp_result = 0;
    if (isunordered(a, b)) cmp_result = 1;
    else if (a == b) cmp_result = 2;
    else if (a > b) cmp_result = 3;
    else if (a < b) cmp_result = 4;
    else cmp_result = 5;  /* LTGT case? */
    
    switch (cmp_result) {
        case 1: result = 0x01; break;  /* UNORDERED */
        case 2: result = 0x02; break;  /* ORDERED/EQ */
        case 3: result = 0x04; break;  /* GT */
        case 4: result = 0x08; break;  /* LT */
        case 5: result = 0x10; break;  /* Other */
    }
    
    return result;
}

/* Test 7: Direct use of builtins for SSE comparisons */
NOINLINE int test_sse_builtins(void) {
    int result = 0;
    volatile __m128d v1, v2;
    volatile int cmp_result;
    
    /* Initialize vectors */
    v1 = _mm_set_pd(g_nan, g_one);
    v2 = _mm_set_pd(g_one, g_zero);
    
    /* Use builtin for unordered compare */
    cmp_result = _mm_ucomilt_sd(_mm_set_sd(g_nan), _mm_set_sd(g_one));
    result |= (cmp_result & 1);
    
    cmp_result = _mm_ucomile_sd(_mm_set_sd(g_zero), _mm_set_sd(g_one));
    result |= ((cmp_result & 1) << 1);
    
    cmp_result = _mm_ucomigt_sd(_mm_set_sd(g_one), _mm_set_sd(g_zero));
    result |= ((cmp_result & 1) << 2);
    
    cmp_result = _mm_ucomige_sd(_mm_set_sd(g_one), _mm_set_sd(g_zero));
    result |= ((cmp_result & 1) << 3);
    
    cmp_result = _mm_ucomieq_sd(_mm_set_sd(g_one), _mm_set_sd(g_one));
    result |= ((cmp_result & 1) << 4);
    
    cmp_result = _mm_ucomineq_sd(_mm_set_sd(g_one), _mm_set_sd(g_zero));
    result |= ((cmp_result & 1) << 5);
    
    return result;
}

/* Main function that runs all tests */
int main(void) {
    int checksum = 0;
    
    /* Initialize test data */
    init_arrays();
    
    /* Run all tests */
    checksum ^= test_unordered_ordered();
    checksum ^= test_math_macros();
    checksum ^= test_inline_asm();
    checksum ^= test_array_comparisons();
    checksum ^= test_long_double();
    checksum ^= test_switch_comparisons();
    
    /* Only use SSE builtins if compiling with SSE support */
    #ifdef __SSE2__
    checksum ^= test_sse_builtins();
    #endif
    
    /* Print result to prevent dead code elimination */
    printf("Result checksum: %d\n", checksum);
    
    return checksum == 0 ? 0 : 1;
}
