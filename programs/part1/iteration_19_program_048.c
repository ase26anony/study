/* Test program to trigger x86 condition code printing for floating-point comparisons */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent optimizations from removing critical code */
#define NOINLINE __attribute__((noinline, noipa))
#define VOLATILE_DOUBLE volatile double
#define VOLATILE_LONG_DOUBLE volatile long double

/* Test 1: Direct unordered comparisons with NaN values */
NOINLINE int test_unordered_comparisons(void) {
    VOLATILE_DOUBLE nan_val = NAN;
    VOLATILE_DOUBLE normal_val = 3.14159;
    VOLATILE_DOUBLE inf_val = INFINITY;
    
    int results[8] = {0};
    
    /* These should generate UNORDERED/ORDERED condition codes */
    results[0] = (nan_val != normal_val) ? 1 : 0;      /* UNORDERED */
    results[1] = (nan_val == nan_val) ? 1 : 0;         /* ORDERED (false for NaN) */
    results[2] = (normal_val == normal_val) ? 1 : 0;   /* ORDERED (true) */
    
    /* Mixed comparisons */
    results[3] = (normal_val != inf_val) ? 1 : 0;
    results[4] = (inf_val == inf_val) ? 1 : 0;
    
    /* Complex expression to prevent optimization */
    VOLATILE_DOUBLE a = 1.0;
    VOLATILE_DOUBLE b = 2.0;
    VOLATILE_DOUBLE c = NAN;
    
    results[5] = ((a < b) && !(c == c)) ? 1 : 0;
    results[6] = ((a > b) || (c != c)) ? 1 : 0;
    
    int sum = 0;
    for (int i = 0; i < 7; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 2: Using math.h comparison macros */
NOINLINE int test_math_macros(void) {
    VOLATILE_DOUBLE nan1 = NAN;
    VOLATILE_DOUBLE nan2 = NAN;
    VOLATILE_DOUBLE val1 = 1.5;
    VOLATILE_DOUBLE val2 = 2.5;
    
    int results[12] = {0};
    
    /* These map directly to various condition codes */
    results[0] = isunordered(nan1, val1);   /* UNORDERED */
    results[1] = !isunordered(val1, val2);  /* ORDERED */
    results[2] = isgreater(val1, val2);     /* UNLE? */
    results[3] = isgreaterequal(val1, val2); /* UNLT? */
    results[4] = isless(val1, val2);        /* UNGE? */
    results[5] = islessequal(val1, val2);   /* UNGT? */
    results[6] = islessgreater(val1, val2); /* LTGT */
    
    /* More complex cases */
    results[7] = isunordered(nan1, nan2);
    results[8] = isgreater(val2, val1);
    results[9] = islessequal(val2, val1);
    
    /* Chain comparisons to force multiple condition codes */
    results[10] = (isunordered(nan1, val1) || isgreater(val1, val2)) ? 1 : 0;
    results[11] = (!isunordered(val1, val2) && isless(val1, val2)) ? 1 : 0;
    
    int sum = 0;
    for (int i = 0; i < 12; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 3: Inline assembly with %C modifier for condition codes */
NOINLINE int test_inline_asm(void) {
    VOLATILE_DOUBLE x = 1.0;
    VOLATILE_DOUBLE y = 2.0;
    VOLATILE_DOUBLE z = NAN;
    
    unsigned char result1, result2, result3, result4;
    
    /* Test various condition codes through inline assembly */
    /* Using x87 floating-point compare */
    __asm__ volatile (
        "fldl %2\n\t"
        "fldl %3\n\t"
        "fucomip %%st(1), %%st\n\t"
        "set%C0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result1)
        : "0"(0), "m"(x), "m"(y)
        : "cc", "st"
    );
    
    /* Unordered comparison */
    __asm__ volatile (
        "fldl %2\n\t"
        "fldl %3\n\t"
        "fucomip %%st(1), %%st\n\t"
        "set%C0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result2)
        : "0"(0), "m"(z), "m"(x)
        : "cc", "st"
    );
    
    /* SSE2 compare for different condition codes */
    __asm__ volatile (
        "movsd %2, %%xmm0\n\t"
        "movsd %3, %%xmm1\n\t"
        "ucomisd %%xmm0, %%xmm1\n\t"
        "set%C0 %0"
        : "=r"(result3)
        : "0"(0), "m"(x), "m"(y)
        : "xmm0", "xmm1", "cc"
    );
    
    /* Another SSE compare with NaN */
    __asm__ volatile (
        "movsd %2, %%xmm0\n\t"
        "movsd %3, %%xmm1\n\t"
        "ucomisd %%xmm0, %%xmm1\n\t"
        "set%C0 %0"
        : "=r"(result4)
        : "0"(0), "m"(z), "m"(z)
        : "xmm0", "xmm1", "cc"
    );
    
    return result1 + result2 + result3 + result4;
}

/* Test 4: Long double (x87) operations */
NOINLINE int test_long_double(void) {
    VOLATILE_LONG_DOUBLE ld_nan = NAN;
    VOLATILE_LONG_DOUBLE ld1 = 3.14159265358979323846L;
    VOLATILE_LONG_DOUBLE ld2 = 2.71828182845904523536L;
    
    int results = 0;
    
    /* Long double comparisons often use x87 instructions */
    if (ld1 > ld2) results += 1;      /* UNLE */
    if (ld1 < ld2) results += 2;      /* UNGE */
    if (ld1 == ld1) results += 4;     /* ORDERED */
    if (ld_nan != ld_nan) results += 8; /* UNORDERED */
    if (!(ld1 == ld2)) results += 16;  /* LTGT? */
    
    /* More complex long double expression */
    VOLATILE_LONG_DOUBLE a = 1.0L;
    VOLATILE_LONG_DOUBLE b = 2.0L;
    VOLATILE_LONG_DOUBLE c = ld_nan;
    
    if ((a < b) && (c != c)) results += 32;
    if ((a > b) || (c == c)) results += 64;
    
    return results;
}

/* Test 5: Array operations with mixed comparisons */
NOINLINE int test_array_comparisons(void) {
    #define ARRAY_SIZE 16
    VOLATILE_DOUBLE arr1[ARRAY_SIZE];
    VOLATILE_DOUBLE arr2[ARRAY_SIZE];
    
    /* Initialize with mix of values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (i % 4 == 0) {
            arr1[i] = NAN;
            arr2[i] = (double)i;
        } else if (i % 4 == 1) {
            arr1[i] = (double)i;
            arr2[i] = NAN;
        } else if (i % 4 == 2) {
            arr1[i] = (double)i;
            arr2[i] = (double)(i * 2);
        } else {
            arr1[i] = (double)i;
            arr2[i] = (double)i;
        }
    }
    
    int unordered_count = 0;
    int ordered_count = 0;
    int greater_count = 0;
    int less_count = 0;
    int equal_count = 0;
    
    /* Loop with various comparisons */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (isunordered(arr1[i], arr2[i])) {
            unordered_count++;
        } else {
            ordered_count++;
        }
        
        if (isgreater(arr1[i], arr2[i])) {
            greater_count++;
        }
        
        if (isless(arr1[i], arr2[i])) {
            less_count++;
        }
        
        if (!isunordered(arr1[i], arr2[i]) && 
            !isgreater(arr1[i], arr2[i]) && 
            !isless(arr1[i], arr2[i])) {
            equal_count++;
        }
    }
    
    /* Complex return value to prevent dead code elimination */
    return unordered_count + (ordered_count << 4) + 
           (greater_count << 8) + (less_count << 12) + 
           (equal_count << 16);
}

/* Test 6: Switch statement based on comparison results */
NOINLINE int test_switch_comparisons(void) {
    VOLATILE_DOUBLE a = 1.5;
    VOLATILE_DOUBLE b = 2.5;
    VOLATILE_DOUBLE c = NAN;
    
    int result = 0;
    
    /* Force generation of multiple condition code checks */
    if (isunordered(a, c)) {
        result = 1;  /* UNORDERED */
    } else if (isgreater(a, b)) {
        result = 2;  /* UNLE */
    } else if (isless(a, b)) {
        result = 3;  /* UNGE */
    } else if (!isunordered(a, b) && !isgreater(a, b) && !isless(a, b)) {
        result = 4;  /* UNEQ */
    }
    
    /* Another switch-like structure */
    int cmp_result = 0;
    if (a != a) cmp_result = 1;      /* UNORDERED */
    else if (a > b) cmp_result = 2;   /* UNLE */
    else if (a < b) cmp_result = 3;   /* UNGE */
    else if (a == b) cmp_result = 4;  /* UNEQ */
    else cmp_result = 5;              /* LTGT */
    
    return result + (cmp_result << 4);
}

/* Test 7: Direct use of builtin functions */
NOINLINE int test_builtins(void) {
    double a = 1.0;
    double b = 2.0;
    double c = NAN;
    
    int results = 0;
    
    /* GCC builtin for unordered compare */
    int cmp1 = __builtin_isgreater(a, b);
    int cmp2 = __builtin_isless(a, b);
    int cmp3 = __builtin_isunordered(a, c);
    int cmp4 = __builtin_isunordered(a, b);
    
    results = cmp1 + (cmp2 << 2) + (cmp3 << 4) + (cmp4 << 6);
    
    /* SSE2 specific builtins */
    __m128d x = _mm_set_sd(a);
    __m128d y = _mm_set_sd(b);
    __m128d z = _mm_set_sd(c);
    
    int sse_cmp1 = _mm_ucomigt_sd(x, y);
    int sse_cmp2 = _mm_ucomilt_sd(x, y);
    int sse_cmp3 = _mm_ucomieq_sd(x, y);
    int sse_cmp4 = _mm_ucomineq_sd(x, y);
    
    results += (sse_cmp1 << 8) + (sse_cmp2 << 10) + 
               (sse_cmp3 << 12) + (sse_cmp4 << 14);
    
    return results;
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
    total += test_switch_comparisons();
    
    /* Note: test_builtins requires <xmmintrin.h> and may not compile
       on all systems without -msse2, so we conditionally include it */
    #ifdef __SSE2__
    #include <xmmintrin.h>
    total += test_builtins();
    #endif
    
    printf("Total checksum: %d\n", total);
    printf("Test completed.\n");
    
    return 0;
}
