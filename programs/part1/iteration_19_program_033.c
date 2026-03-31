/* test_condition_codes.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent optimizations from removing crucial operations */
#define NOINLINE __attribute__((noinline, noipa))

/* Test 1: Unordered comparisons with NaN values */
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
    if (isunordered(nan_val, normal_val)) results[5] = 1;
    if (isunordered(normal_val, normal_val)) results[6] = 0;
    else results[6] = 1;
    
    /* Complex expression to prevent optimization */
    results[7] = (isunordered(nan_val, normal_val) && 
                  !isunordered(normal_val, normal_val)) ? 1 : 0;
    
    int sum = 0;
    for (int i = 0; i < 8; i++) sum += results[i];
    return sum;
}

/* Test 2: Inline assembly with %C modifier */
NOINLINE int test_asm_condition_codes(void) {
    volatile double a = NAN;
    volatile double b = 2.71828;
    volatile double c = -INFINITY;
    volatile double d = 0.0;
    
    int result1 = 0, result2 = 0, result3 = 0, result4 = 0;
    
    /* Test UNORDERED condition code */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result1)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* Test ORDERED condition code */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result2)
        : "x"(b), "x"(d)
        : "cc"
    );
    
    /* Test UNEQ condition code */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result3)
        : "x"(d), "x"(d)
        : "cc"
    );
    
    /* Test UNLT condition code */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result4)
        : "x"(c), "x"(b)
        : "cc"
    );
    
    return result1 + result2 + result3 + result4;
}

/* Test 3: Array comparisons with various condition codes */
NOINLINE int test_array_comparisons(void) {
    volatile double arr1[16];
    volatile double arr2[16];
    
    /* Initialize with mix of values */
    for (int i = 0; i < 16; i++) {
        if (i % 5 == 0) {
            arr1[i] = NAN;
            arr2[i] = (double)i;
        } else if (i % 3 == 0) {
            arr1[i] = INFINITY;
            arr2[i] = -INFINITY;
        } else {
            arr1[i] = (double)i * 1.5;
            arr2[i] = (double)i * 0.75;
        }
    }
    
    int counts[7] = {0};  /* For different comparison types */
    
    for (int i = 0; i < 16; i++) {
        /* Use various comparison macros to trigger different condition codes */
        counts[0] += isunordered(arr1[i], arr2[i]) ? 1 : 0;    /* UNORDERED */
        counts[1] += isgreater(arr1[i], arr2[i]) ? 1 : 0;      /* UNLE? Actually generates GT */
        counts[2] += isless(arr1[i], arr2[i]) ? 1 : 0;         /* UNGE? Actually generates LT */
        counts[3] += islessequal(arr1[i], arr2[i]) ? 1 : 0;    /* UNGT */
        counts[4] += isgreaterequal(arr1[i], arr2[i]) ? 1 : 0; /* UNLT */
        
        /* Direct comparisons that might generate UNEQ/LTGT */
        counts[5] += (arr1[i] == arr2[i]) ? 1 : 0;             /* UNEQ when NaN */
        counts[6] += (arr1[i] != arr2[i]) ? 1 : 0;             /* LTGT when NaN */
    }
    
    int sum = 0;
    for (int i = 0; i < 7; i++) sum += counts[i];
    return sum;
}

/* Test 4: Long double (x87) comparisons */
NOINLINE int test_long_double_comparisons(void) {
    volatile long double ld_nan = NAN;
    volatile long double ld_inf = INFINITY;
    volatile long double ld_normal = 3.14159265358979323846L;
    volatile long double ld_zero = 0.0L;
    
    int results = 0;
    
    /* x87 style comparisons - may generate different condition codes */
    if (ld_nan != ld_normal) results += 1;      /* UNORDERED */
    if (ld_normal == ld_normal) results += 2;   /* ORDERED */
    if (isless(ld_zero, ld_normal)) results += 4; /* UNGE */
    if (isgreater(ld_inf, ld_normal)) results += 8; /* UNLE */
    
    /* Complex switch based on comparison results */
    switch (fpclassify(ld_nan)) {
        case FP_NAN: results += 16; break;
        case FP_INFINITE: results += 32; break;
        default: break;
    }
    
    switch (fpclassify(ld_normal)) {
        case FP_NORMAL: results += 64; break;
        case FP_ZERO: results += 128; break;
        default: break;
    }
    
    /* Force x87 instructions with inline asm */
    int asm_result = 0;
    __asm__ volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "set%C0 %0"
        : "=r"(asm_result)
        : "m"(ld_normal), "m"(ld_zero)
        : "cc", "st"
    );
    
    return results + asm_result;
}

/* Test 5: Mixed SSE/x87 with GCC builtins */
NOINLINE int test_builtin_comparisons(void) {
    volatile double d1 = NAN;
    volatile double d2 = 42.0;
    volatile double d3 = -INFINITY;
    volatile double d4 = INFINITY;
    
    int results = 0;
    
    /* Use GCC x86 builtins for direct unordered compares */
    results += __builtin_ia32_ucomisd(d1, d2) ? 1 : 0;
    results += __builtin_ia32_ucomisd(d2, d3) ? 2 : 0;
    results += __builtin_ia32_ucomisd(d3, d4) ? 4 : 0;
    results += __builtin_ia32_ucomisd(d4, d4) ? 8 : 0;
    
    /* Conditional moves based on comparisons */
    double cmov_result;
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "mov $0x3FF0000000000000, %%rax\n\t"
        "movq %%rax, %0\n\t"
        "jp 1f\n\t"
        "mov $0, %%rax\n\t"
        "movq %%rax, %0\n\t"
        "1:"
        : "=m"(cmov_result)
        : "x"(d1), "x"(d2)
        : "rax", "cc"
    );
    
    /* Interpret the double as integer for checksum */
    uint64_t bits;
    memcpy(&bits, &cmov_result, sizeof(bits));
    results += (int)(bits & 0xFF);
    
    return results;
}

int main(void) {
    int total = 0;
    
    printf("Testing x86 condition code generation...\n");
    
    total += test_unordered_comparisons();
    total += test_asm_condition_codes();
    total += test_array_comparisons();
    total += test_long_double_comparisons();
    total += test_builtin_comparisons();
    
    printf("Checksum: %d\n", total);
    
    /* Use results to prevent dead code elimination */
    volatile int sink = total;
    (void)sink;
    
    return 0;
}
