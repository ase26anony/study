/* Test program to trigger x86 floating-point condition code printing */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent optimizations from removing critical operations */
#define NOINLINE __attribute__((noinline, noipa))

/* Global volatile variables to prevent constant folding */
volatile double g_nan = NAN;
volatile double g_inf = INFINITY;
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile long double g_ld_nan;
volatile long double g_ld_inf;
volatile long double g_ld_zero = 0.0L;
volatile long double g_ld_one = 1.0L;

/* Initialize long double NaN/Inf values */
static void init_ld_values(void) {
    g_ld_nan = strtold("NAN", NULL);
    g_ld_inf = strtold("INF", NULL);
}

/* Test 1: Various unordered comparisons using double */
NOINLINE int test_unordered_comparisons(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    volatile double d = g_inf;
    
    int results[16];
    int idx = 0;
    
    /* UNORDERED: a != a (NaN comparison) */
    results[idx++] = (a != a) ? 1 : 0;
    
    /* ORDERED: b == b (normal number comparison) */
    results[idx++] = (b == b) ? 1 : 0;
    
    /* UNEQ: !(a > b) && !(a < b) for NaN */
    results[idx++] = !(a > b) && !(a < b) ? 1 : 0;
    
    /* UNGE: !(a < b) for NaN */
    results[idx++] = !(a < b) ? 1 : 0;
    
    /* UNGT: !(a <= b) for NaN */
    results[idx++] = !(a <= b) ? 1 : 0;
    
    /* UNLE: !(a > b) for NaN */
    results[idx++] = !(a > b) ? 1 : 0;
    
    /* UNLT: !(a >= b) for NaN */
    results[idx++] = !(a >= b) ? 1 : 0;
    
    /* LTGT: (a < b) || (a > b) for NaN (should be false) */
    results[idx++] = (a < b) || (a > b) ? 1 : 0;
    
    /* More comparisons mixing values */
    results[idx++] = isunordered(a, b);
    results[idx++] = isordered(c, d);
    results[idx++] = isgreater(c, d);
    results[idx++] = isless(c, d);
    results[idx++] = isgreaterequal(c, d);
    results[idx++] = islessequal(c, d);
    results[idx++] = islessgreater(c, d);
    results[idx++] = !isunordered(c, d) && !isgreater(c, d) && !isless(c, d);
    
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 2: Inline assembly with %C modifier for condition codes */
NOINLINE int test_asm_condition_codes(void) {
    volatile double x = g_one;
    volatile double y = g_nan;
    volatile double z = g_zero;
    
    int result = 0;
    
    /* Test UNORDERED condition */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result)
        : "x"(x), "x"(y)
        : "cc"
    );
    
    int result2 = 0;
    /* Test ORDERED condition */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result2)
        : "x"(z), "x"(x)
        : "cc"
    );
    
    return result + result2;
}

/* Test 3: Loop over arrays with various comparisons */
NOINLINE int test_array_comparisons(void) {
    volatile double arr1[8];
    volatile double arr2[8];
    
    /* Initialize arrays with mix of values */
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
            arr1[i] = g_inf;
            arr2[i] = g_inf;
        }
    }
    
    int counts[8] = {0};
    
    for (int i = 0; i < 8; i++) {
        counts[0] += isunordered(arr1[i], arr2[i]);
        counts[1] += isordered(arr1[i], arr2[i]);
        counts[2] += isgreater(arr1[i], arr2[i]);
        counts[3] += isless(arr1[i], arr2[i]);
        counts[4] += isgreaterequal(arr1[i], arr2[i]);
        counts[5] += islessequal(arr1[i], arr2[i]);
        counts[6] += islessgreater(arr1[i], arr2[i]);
        counts[7] += !isunordered(arr1[i], arr2[i]) && 
                     !isgreater(arr1[i], arr2[i]) && 
                     !isless(arr1[i], arr2[i]);
    }
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += counts[i];
    }
    return sum;
}

/* Test 4: Long double (x87) comparisons */
NOINLINE int test_long_double_comparisons(void) {
    volatile long double a = g_ld_nan;
    volatile long double b = g_ld_one;
    volatile long double c = g_ld_zero;
    volatile long double d = g_ld_inf;
    
    int results = 0;
    
    /* Force x87 unordered compare */
    if (a != a) results |= 1;      /* UNORDERED */
    if (b == b) results |= 2;      /* ORDERED */
    
    /* Complex expression to force multiple condition codes */
    if (!(a > b) && !(a < b)) results |= 4;    /* UNEQ */
    if (!(a < b)) results |= 8;                /* UNGE */
    if (!(a <= b)) results |= 16;              /* UNGT */
    if (!(a > b)) results |= 32;               /* UNLE */
    if (!(a >= b)) results |= 64;              /* UNLT */
    if ((a < b) || (a > b)) results |= 128;    /* LTGT */
    
    /* More comparisons */
    if (c > d) results |= 256;
    if (c < d) results |= 512;
    if (c >= d) results |= 1024;
    if (c <= d) results |= 2048;
    
    return results;
}

/* Test 5: Switch based on floating-point classification */
NOINLINE int test_fpclassify_switch(void) {
    volatile double vals[6] = {
        g_nan, g_inf, -g_inf, g_zero, -g_zero, g_one
    };
    
    int results = 0;
    
    for (int i = 0; i < 6; i++) {
        switch (fpclassify(vals[i])) {
            case FP_NAN:
                results += 1;
                /* Force unordered comparison in NaN case */
                if (isunordered(vals[i], vals[(i+1)%6])) results += 2;
                break;
            case FP_INFINITE:
                results += 4;
                /* Ordered comparison for infinite values */
                if (isordered(vals[i], vals[(i+1)%6])) results += 8;
                break;
            case FP_ZERO:
                results += 16;
                /* Various comparisons for zero */
                if (isgreater(vals[i], vals[(i+1)%6])) results += 32;
                if (isless(vals[i], vals[(i+1)%6])) results += 64;
                break;
            case FP_SUBNORMAL:
                results += 128;
                break;
            case FP_NORMAL:
                results += 256;
                /* Complex expression to trigger multiple condition codes */
                if (!isunordered(vals[i], vals[(i+1)%6]) && 
                    !isgreater(vals[i], vals[(i+1)%6]) && 
                    !isless(vals[i], vals[(i+1)%6])) {
                    results += 512;
                }
                break;
        }
    }
    
    return results;
}

/* Test 6: Direct use of GCC builtins for SSE2 unordered compares */
NOINLINE int test_sse2_builtins(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    volatile double d = g_inf;
    
    int res = 0;
    
    /* Use GCC's IA32 builtins for direct unordered compare */
    res |= __builtin_ia32_ucomisd(a, b);
    res |= __builtin_ia32_ucomisd(b, a);
    res |= __builtin_ia32_ucomisd(c, d);
    res |= __builtin_ia32_ucomisd(d, c);
    res |= __builtin_ia32_ucomisd(c, c);
    res |= __builtin_ia32_ucomisd(a, a);
    
    /* Comisd for ordered compare */
    res |= __builtin_ia32_comisd(b, c);
    res |= __builtin_ia32_comisd(d, b);
    
    return res & 0xFF;
}

/* Main function that runs all tests */
int main(void) {
    init_ld_values();
    
    int checksum = 0;
    
    checksum += test_unordered_comparisons();
    checksum += test_asm_condition_codes();
    checksum += test_array_comparisons();
    checksum += test_long_double_comparisons();
    checksum += test_fpclassify_switch();
    checksum += test_sse2_builtins();
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
