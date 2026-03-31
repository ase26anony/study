/* gcc -O3 -march=x86-64 -mfpmath=sse -ffast-math -S -fverbose-asm test.c -o test.s */
/* Also try: gcc -O3 -m32 -mfpmath=387 -march=i686 -S test.c -o test-32.s */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Test 1: Direct unordered comparisons with NaN */
NOINLINE int test_unordered_comparisons(void) {
    volatile double nan_val = NAN;
    volatile double normal_val = 3.14159;
    volatile double inf_val = INFINITY;
    
    int results[8] = {0};
    
    /* These should generate UNORDERED/ORDERED condition codes */
    results[0] = (nan_val != normal_val) ? 1 : 0;      /* UNORDERED */
    results[1] = (nan_val == nan_val) ? 1 : 0;         /* ORDERED (false for NaN) */
    results[2] = (normal_val == normal_val) ? 1 : 0;   /* ORDERED */
    
    /* Mixed comparisons */
    results[3] = (normal_val != inf_val) ? 1 : 0;
    results[4] = (inf_val == inf_val) ? 1 : 0;
    
    /* Force multiple branches */
    if (isunordered(nan_val, normal_val)) results[5] = 1;  /* UNORDERED */
    if (isgreater(normal_val, 2.0)) results[6] = 1;       /* UNGT? */
    if (isless(2.0, normal_val)) results[7] = 1;          /* UNLT? */
    
    int sum = 0;
    for (int i = 0; i < 8; i++) sum += results[i];
    return sum;
}

/* Test 2: Inline assembly with %C modifier for condition codes */
NOINLINE int test_asm_condition_codes(void) {
    volatile double a = 1.5;
    volatile double b = 2.5;
    volatile double c = NAN;
    int result = 0;
    
    /* Test various condition codes through inline assembly */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    int result2 = 0;
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result2)
        : "x"(c), "x"(a)  /* Compare NaN with normal */
        : "cc"
    );
    
    return result + result2;
}

/* Test 3: Array operations with various comparison macros */
NOINLINE int test_array_comparisons(void) {
    volatile double arr1[8];
    volatile double arr2[8];
    
    /* Initialize with mix of values */
    for (int i = 0; i < 8; i++) {
        arr1[i] = (i % 2 == 0) ? (double)i : NAN;
        arr2[i] = (i % 3 == 0) ? (double)(i * 2) : NAN;
    }
    
    int counts[6] = {0};
    
    for (int i = 0; i < 8; i++) {
        /* Each of these may generate different condition codes */
        if (isunordered(arr1[i], arr2[i])) counts[0]++;  /* UNORDERED */
        if (isgreater(arr1[i], arr2[i])) counts[1]++;    /* UNGT */
        if (isless(arr1[i], arr2[i])) counts[2]++;       /* UNLT */
        if (isgreaterequal(arr1[i], arr2[i])) counts[3]++; /* UNGE */
        if (islessequal(arr1[i], arr2[i])) counts[4]++;  /* UNLE */
        if (!isunordered(arr1[i], arr2[i]) && 
            arr1[i] != arr2[i]) counts[5]++;             /* LTGT */
    }
    
    int sum = 0;
    for (int i = 0; i < 6; i++) sum += counts[i];
    return sum;
}

/* Test 4: Long double (x87) operations */
NOINLINE int test_long_double_ops(void) {
    volatile long double ld_nan = NAN;
    volatile long double ld_val = 3.14159265358979323846L;
    volatile long double ld_inf = INFINITY;
    
    int results = 0;
    
    /* x87 comparisons - may use different condition codes */
    if (ld_nan != ld_val) results |= 1;      /* UNORDERED */
    if (ld_val == ld_val) results |= 2;      /* ORDERED */
    if (ld_val > 2.0L) results |= 4;         /* UNGT? */
    if (2.0L < ld_val) results |= 8;         /* UNLT? */
    if (ld_val >= 3.0L) results |= 16;       /* UNGE? */
    if (ld_val <= 4.0L) results |= 32;       /* UNLE? */
    
    /* Complex expression to force multiple compares */
    long double temp = ld_val;
    for (int i = 0; i < 3; i++) {
        temp *= 1.1L;
        if (temp > ld_val) results += 64;
        if (isunordered(temp, ld_nan)) results += 128;
    }
    
    return results;
}

/* Test 5: Switch based on comparison results */
NOINLINE int test_switch_comparisons(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double c = NAN;
    
    int result = 0;
    
    /* Force compiler to generate multiple condition code checks */
    if (isunordered(a, c)) {
        result = 1;  /* UNORDERED */
    } else if (isgreater(a, b)) {
        result = 2;  /* UNGT */
    } else if (isless(a, b)) {
        result = 3;  /* UNLT */
    } else if (isgreaterequal(a, b)) {
        result = 4;  /* UNGE */
    } else if (islessequal(a, b)) {
        result = 5;  /* UNLE */
    } else if (!isunordered(a, b) && a != b) {
        result = 6;  /* LTGT */
    } else if (a == b) {
        result = 7;  /* UNEQ? */
    }
    
    /* More complex switch */
    switch (fpclassify(c)) {
        case FP_NAN: result += 10; break;
        case FP_INFINITE: result += 20; break;
        case FP_ZERO: result += 30; break;
        case FP_SUBNORMAL: result += 40; break;
        case FP_NORMAL: result += 50; break;
    }
    
    return result;
}

/* Test 6: Direct builtin usage */
NOINLINE int test_builtin_comparisons(void) {
    double a = 1.5;
    double b = 2.5;
    double c = NAN;
    int res = 0;
    
    /* GCC builtins for direct comparison */
    res = __builtin_ia32_ucomisd(a, b);   /* Unordered compare */
    res += __builtin_ia32_ucomisd(c, a);  /* Compare with NaN */
    res += __builtin_ia32_ucomisd(a, c);
    
    /* Use result in conditional */
    if (res & 0x40) {  /* Parity flag for unordered */
        res += 100;
    }
    
    return res;
}

/* Test 7: Mixed SSE/x87 operations */
NOINLINE int test_mixed_operations(void) {
    volatile double d1 = 1.0;
    volatile double d2 = NAN;
    volatile long double ld1 = 2.0L;
    volatile long double ld2 = NAN;
    
    int result = 0;
    
    /* SSE2 double comparison */
    if (d1 != d2) result += 1;
    
    /* x87 long double comparison */
    if (ld1 == ld1) result += 2;
    if (ld1 != ld2) result += 4;
    
    /* Mixed precision */
    double d_from_ld = (double)ld1;
    if (d_from_ld > d1) result += 8;
    
    /* Complex expression mixing types */
    for (int i = 0; i < 2; i++) {
        d1 += 0.5;
        ld1 *= 1.1L;
        
        if (isunordered(d1, d2)) result += 16;
        if (ld1 > 2.5L) result += 32;
        if (!isunordered(ld1, ld2) && ld1 != ld2) result += 64;
    }
    
    return result;
}

int main(void) {
    int checksum = 0;
    
    printf("Running condition code tests...\n");
    
    checksum += test_unordered_comparisons();
    checksum += test_asm_condition_codes();
    checksum += test_array_comparisons();
    checksum += test_long_double_ops();
    checksum += test_switch_comparisons();
    checksum += test_builtin_comparisons();
    checksum += test_mixed_operations();
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
