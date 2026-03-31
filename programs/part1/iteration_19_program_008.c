/* test_condition_codes.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent optimizations from removing critical operations */
#define NO_OPT __attribute__((noinline, noipa))

/* Test 1: Direct unordered comparisons with NaN */
NO_OPT int test_unordered_comparisons(void) {
    volatile double nan_val = NAN;
    volatile double normal_val = 3.14159;
    volatile double inf_val = INFINITY;
    int results[8] = {0};
    
    /* These should generate UNORDERED/ORDERED condition codes */
    results[0] = (nan_val != normal_val) ? 1 : 0;  /* UNORDERED */
    results[1] = (nan_val == nan_val) ? 1 : 0;     /* UNORDERED */
    results[2] = (normal_val == normal_val) ? 1 : 0; /* ORDERED */
    results[3] = (normal_val != inf_val) ? 1 : 0;  /* ORDERED */
    
    /* Mixed comparisons */
    results[4] = (isunordered(nan_val, normal_val)) ? 1 : 0;
    results[5] = (isgreater(normal_val, nan_val)) ? 1 : 0;
    results[6] = (isless(nan_val, normal_val)) ? 1 : 0;
    results[7] = (isunordered(nan_val, nan_val)) ? 1 : 0;
    
    int sum = 0;
    for (int i = 0; i < 8; i++) sum += results[i];
    return sum;
}

/* Test 2: Inline assembly with %C modifier for condition codes */
NO_OPT int test_asm_condition_codes(void) {
    volatile double a = 1.0;
    volatile double b = NAN;
    volatile double c = 2.0;
    volatile double d = -INFINITY;
    
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    
    /* UNORDERED comparison */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C0 %0"
        : "=r"(r1)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* ORDERED comparison */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C1 %0"
        : "=r"(r2)
        : "x"(c), "x"(a)
        : "cc"
    );
    
    /* UNEQ comparison (unordered or equal) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C2 %0"
        : "=r"(r3)
        : "x"(a), "x"(a)
        : "cc"
    );
    
    /* UNLT comparison (unordered or less than) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C3 %0"
        : "=r"(r4)
        : "x"(d), "x"(c)
        : "cc"
    );
    
    return r1 + r2 + r3 + r4;
}

/* Test 3: Array comparisons with various condition codes */
NO_OPT int test_array_comparisons(void) {
    volatile double arr1[16];
    volatile double arr2[16];
    
    /* Initialize with mix of values */
    for (int i = 0; i < 16; i++) {
        arr1[i] = (i % 2 == 0) ? (double)i : NAN;
        arr2[i] = (i % 3 == 0) ? (double)(i * 2) : ((i % 5 == 0) ? NAN : (double)i);
    }
    
    int counts[8] = {0};  /* unord, ord, ueq, unge, ungt, unle, unlt, ltgt */
    
    for (int i = 0; i < 16; i++) {
        /* Generate various condition codes through different comparisons */
        if (isunordered(arr1[i], arr2[i])) counts[0]++;  /* UNORDERED */
        if (isgreaterequal(arr1[i], arr2[i])) counts[1]++; /* ORDERED/UNGE */
        if (!islessgreater(arr1[i], arr2[i])) counts[2]++; /* UNEQ */
        if (!isless(arr1[i], arr2[i])) counts[3]++; /* UNGE */
        if (!islessequal(arr1[i], arr2[i])) counts[4]++; /* UNGT */
        if (islessequal(arr1[i], arr2[i]) || isunordered(arr1[i], arr2[i])) counts[5]++; /* UNLE */
        if (isless(arr1[i], arr2[i]) || isunordered(arr1[i], arr2[i])) counts[6]++; /* UNLT */
        if (islessgreater(arr1[i], arr2[i])) counts[7]++; /* LTGT */
    }
    
    int sum = 0;
    for (int i = 0; i < 8; i++) sum += counts[i];
    return sum;
}

/* Test 4: Long double (x87) comparisons */
NO_OPT int test_long_double_comparisons(void) {
    volatile long double ld_nan = NAN;
    volatile long double ld_inf = INFINITY;
    volatile long double ld_val = 3.14159265358979323846L;
    volatile long double ld_zero = 0.0L;
    
    int results = 0;
    
    /* x87 style comparisons - may generate different condition codes */
    if (ld_nan != ld_val) results |= 1;      /* UNORDERED */
    if (ld_val == ld_val) results |= 2;      /* ORDERED */
    if (!(ld_nan < ld_inf)) results |= 4;    /* UNGE */
    if (!(ld_val <= ld_zero)) results |= 8;  /* UNGT */
    if (ld_val <= ld_inf || ld_val != ld_val) results |= 16; /* UNLE */
    if (ld_val < ld_inf || ld_val != ld_val) results |= 32;  /* UNLT */
    if (ld_val != ld_zero) results |= 64;    /* LTGT */
    
    /* Use builtin for direct x87 compare */
    int cmp_result;
    __asm__ volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "set%C0 %0"
        : "=r"(cmp_result)
        : "m"(ld_val), "m"(ld_nan)
        : "cc", "st"
    );
    
    results |= (cmp_result << 8);
    return results;
}

/* Test 5: Switch based on comparison results */
NO_OPT int test_switch_comparisons(void) {
    volatile double vals[4] = {NAN, 1.0, INFINITY, -INFINITY};
    int total = 0;
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int classification = 0;
            
            /* Complex branching to force multiple condition codes */
            if (isunordered(vals[i], vals[j])) {
                classification = 1;  /* UNORDERED */
            } else if (vals[i] == vals[j]) {
                classification = 2;  /* UNEQ */
            } else if (vals[i] > vals[j]) {
                classification = 3;  /* UNGT/UNGE */
            } else if (vals[i] < vals[j]) {
                classification = 4;  /* UNLT/UNLE */
            }
            
            /* Switch to prevent optimization */
            switch (classification) {
                case 1: total += 100; break;  /* UNORDERED */
                case 2: total += 200; break;  /* UNEQ */
                case 3: total += 300; break;  /* UNGT/UNGE */
                case 4: total += 400; break;  /* UNLT/UNLE */
                default: total += 500; break; /* LTGT/ORDERED */
            }
        }
    }
    
    return total;
}

/* Main function that runs all tests */
int main(void) {
    printf("Testing x86 condition code generation...\n");
    
    int checksum = 0;
    
    checksum += test_unordered_comparisons();
    printf("Test 1 result: %d\n", test_unordered_comparisons());
    
    checksum += test_asm_condition_codes();
    printf("Test 2 result: %d\n", test_asm_condition_codes());
    
    checksum += test_array_comparisons();
    printf("Test 3 result: %d\n", test_array_comparisons());
    
    checksum += test_long_double_comparisons();
    printf("Test 4 result: %d\n", test_long_double_comparisons());
    
    checksum += test_switch_comparisons();
    printf("Test 5 result: %d\n", test_switch_comparisons());
    
    printf("Total checksum: %d\n", checksum);
    
    /* Use result to prevent dead code elimination */
    if (checksum > 1000) {
        printf("Condition code tests completed successfully.\n");
    }
    
    return 0;
}
