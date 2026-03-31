/* test_condition_codes.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent optimizations from removing crucial operations */
#define NO_OPT __attribute__((noinline, noipa, noclone))

/* Test 1: Direct unordered comparisons with NaN */
NO_OPT int test_unordered_comparisons(void) {
    volatile double nan_val = NAN;
    volatile double normal_val = 3.14159;
    volatile double inf_val = INFINITY;
    int results[8] = {0};
    
    /* These should generate UNORDERED/ORDERED condition codes */
    results[0] = (nan_val != normal_val) ? 1 : 0;      /* UNORDERED */
    results[1] = (nan_val == nan_val) ? 1 : 0;         /* UNORDERED */
    results[2] = (normal_val == normal_val) ? 1 : 0;   /* ORDERED */
    results[3] = (normal_val != inf_val) ? 1 : 0;      /* ORDERED */
    
    /* Mixed comparisons */
    results[4] = (isunordered(nan_val, normal_val)) ? 1 : 0;
    results[5] = (isgreater(normal_val, nan_val)) ? 1 : 0;   /* UNGT */
    results[6] = (isless(nan_val, normal_val)) ? 1 : 0;      /* UNLT */
    results[7] = (islessequal(inf_val, nan_val)) ? 1 : 0;    /* UNLE */
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 2: Inline assembly with %C modifier */
NO_OPT int test_asm_condition_codes(void) {
    volatile double a = 1.0;
    volatile double b = NAN;
    volatile double c = 2.0;
    volatile double d = -INFINITY;
    
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    
    /* Use different condition codes in inline assembly */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(r1)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C1 %0"
        : "=r"(r2)
        : "x"(c), "x"(d)
        : "cc"
    );
    
    /* Try with x87 instructions for long double */
    volatile long double ld1 = 1.0L;
    volatile long double ld2 = NAN;
    
    __asm__ volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "set%C2 %0"
        : "=r"(r3)
        : "m"(ld1), "m"(ld2)
        : "cc", "st"
    );
    
    return r1 + r2 + r3 + r4;
}

/* Test 3: Array comparisons with various condition codes */
NO_OPT int test_array_comparisons(void) {
    volatile double arr1[16];
    volatile double arr2[16];
    
    /* Initialize with mix of values */
    for (int i = 0; i < 16; i++) {
        if (i % 4 == 0) {
            arr1[i] = NAN;
            arr2[i] = (double)i;
        } else if (i % 4 == 1) {
            arr1[i] = (double)i;
            arr2[i] = NAN;
        } else if (i % 4 == 2) {
            arr1[i] = INFINITY;
            arr2[i] = -(double)i;
        } else {
            arr1[i] = (double)i;
            arr2[i] = (double)(i * 2);
        }
    }
    
    int counts[6] = {0};
    
    for (int i = 0; i < 16; i++) {
        /* Test all the uncovered condition codes */
        counts[0] += isunordered(arr1[i], arr2[i]) ? 1 : 0;  /* UNORDERED */
        counts[1] += (arr1[i] == arr2[i]) ? 1 : 0;           /* UNEQ when NaN involved */
        counts[2] += isgreater(arr1[i], arr2[i]) ? 1 : 0;    /* UNGT */
        counts[3] += isless(arr1[i], arr2[i]) ? 1 : 0;       /* UNLT */
        counts[4] += islessequal(arr1[i], arr2[i]) ? 1 : 0;  /* UNLE */
        counts[5] += isgreaterequal(arr1[i], arr2[i]) ? 1 : 0; /* UNGE */
    }
    
    int sum = 0;
    for (int i = 0; i < 6; i++) {
        sum += counts[i];
    }
    return sum;
}

/* Test 4: Switch based on floating-point comparisons */
NO_OPT int test_switch_comparisons(void) {
    volatile double x = NAN;
    volatile double y = 2.0;
    volatile double z = -0.0;
    volatile double w = INFINITY;
    
    int result = 0;
    
    /* Complex switch to force multiple condition code generations */
    for (int i = 0; i < 4; i++) {
        volatile double a = (i == 0) ? x : (i == 1) ? y : (i == 2) ? z : w;
        volatile double b = (i == 3) ? x : (i == 2) ? y : (i == 1) ? z : w;
        
        int cmp_result;
        if (isunordered(a, b)) {
            cmp_result = 0;  /* UNORDERED */
        } else if (a == b) {
            cmp_result = 1;  /* UNEQ or ORDERED EQ */
        } else if (a > b) {
            cmp_result = 2;  /* UNGT or GT */
        } else if (a < b) {
            cmp_result = 3;  /* UNLT or LT */
        } else {
            cmp_result = 4;  /* LTGT */
        }
        
        switch (cmp_result) {
            case 0: result += 1; break;
            case 1: result += 2; break;
            case 2: result += 3; break;
            case 3: result += 4; break;
            case 4: result += 5; break;
        }
    }
    
    return result;
}

/* Test 5: Direct builtin usage */
NO_OPT int test_builtin_comparisons(void) {
    volatile double a = 1.5;
    volatile double b = NAN;
    volatile double c = 2.5;
    volatile double d = -INFINITY;
    
    int results = 0;
    
    /* Use GCC builtins that map directly to x86 instructions */
    int cmp1 = __builtin_ia32_ucomisd(a, b);
    int cmp2 = __builtin_ia32_ucomisd(c, d);
    int cmp3 = __builtin_ia32_ucomisd(b, b);
    int cmp4 = __builtin_ia32_ucomisd(a, c);
    
    /* Extract condition code results */
    results += (cmp1 & 0x40) ? 1 : 0;  /* ZF: equal */
    results += (cmp1 & 0x01) ? 2 : 0;  /* CF: less than */
    results += (cmp1 & 0x45) == 0 ? 4 : 0;  /* ordered and not equal */
    
    results += (cmp2 & 0x40) ? 8 : 0;
    results += (cmp2 & 0x01) ? 16 : 0;
    
    results += (cmp3 & 0x45) == 0x40 ? 32 : 0;  /* unordered */
    
    results += (cmp4 & 0x45) == 0 ? 64 : 0;  /* ordered and not equal */
    
    return results;
}

/* Test 6: Long double (x87) specific comparisons */
NO_OPT int test_long_double_comparisons(void) {
    volatile long double ld_nan = NAN;
    volatile long double ld_inf = INFINITY;
    volatile long double ld_normal = 3.14159265358979323846L;
    volatile long double ld_zero = 0.0L;
    
    int results[8] = {0};
    
    /* x87 comparisons often generate different condition codes */
    results[0] = (ld_nan != ld_normal) ? 1 : 0;
    results[1] = (ld_nan == ld_nan) ? 1 : 0;
    results[2] = (ld_normal == ld_normal) ? 1 : 0;
    results[3] = (ld_inf > ld_normal) ? 1 : 0;
    results[4] = (ld_zero < ld_normal) ? 1 : 0;
    results[5] = (ld_inf <= ld_nan) ? 1 : 0;
    results[6] = (ld_nan >= ld_zero) ? 1 : 0;
    results[7] = (!(ld_nan < ld_normal) && !(ld_nan > ld_normal) && 
                  !(ld_nan == ld_normal)) ? 1 : 0;  /* LTGT */
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

int main(void) {
    printf("Testing x86 condition code generation...\n");
    
    int total = 0;
    
    total += test_unordered_comparisons();
    total += test_asm_condition_codes();
    total += test_array_comparisons();
    total += test_switch_comparisons();
    total += test_builtin_comparisons();
    total += test_long_double_comparisons();
    
    printf("Total checksum: %d\n", total);
    
    /* Also test with volatile sink to prevent dead code elimination */
    volatile int sink = total;
    
    return (sink > 100) ? 0 : 1;
}
