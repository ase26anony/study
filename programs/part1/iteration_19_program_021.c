#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

/* Prevent optimizations from removing crucial operations */
#define NOINLINE __attribute__((noinline, noipa))

/* Test 1: Direct unordered comparisons with NaN */
NOINLINE int test_unordered_comparisons(void) {
    volatile double nan_val = NAN;
    volatile double normal_val = 3.14159;
    volatile double inf_val = INFINITY;
    int results[8] = {0};
    
    /* These should generate UNORDERED/ORDERED condition codes */
    results[0] = (nan_val != normal_val) ? 1 : 0;      /* unordered comparison */
    results[1] = (nan_val == nan_val) ? 1 : 0;         /* ordered comparison */
    results[2] = (normal_val == normal_val) ? 1 : 0;   /* ordered */
    results[3] = (inf_val != inf_val) ? 1 : 0;         /* unordered with INF */
    
    /* Force use of different condition codes through branching */
    if (isunordered(nan_val, normal_val)) results[4] = 1;  /* UNORDERED */
    if (!isunordered(normal_val, normal_val)) results[5] = 1; /* ORDERED */
    
    /* UNEQ: unordered or equal */
    if (!isless(nan_val, normal_val) && !isgreater(nan_val, normal_val)) 
        results[6] = 1;
    
    /* LTGT: less or greater (ordered and not equal) */
    if (isless(normal_val, 10.0) || isgreater(normal_val, 1.0))
        results[7] = 1;
    
    int sum = 0;
    for (int i = 0; i < 8; i++) sum += results[i];
    return sum;
}

/* Test 2: Inline assembly with %C modifier for condition codes */
NOINLINE int test_asm_condition_codes(void) {
    volatile double a = 1.5;
    volatile double b = 2.5;
    volatile double c = NAN;
    volatile double d = 3.0;
    
    int result1 = 0, result2 = 0, result3 = 0, result4 = 0;
    
    /* Using %C modifier to get condition code names */
    /* UNORDERED comparison */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C0 %0"
        : "=r"(result1)
        : "x"(a), "x"(c)
        : "cc"
    );
    
    /* ORDERED comparison */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C1 %0"
        : "=r"(result2)
        : "x"(a), "x"(b), "1"(7)  /* Condition code for ORDERED */
        : "cc"
    );
    
    /* UNLT (ult) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C2 %0"
        : "=r"(result3)
        : "x"(a), "x"(b), "2"(6)  /* Condition code for UNLT */
        : "cc"
    );
    
    /* UNLE (ule) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C3 %0"
        : "=r"(result4)
        : "x"(b), "x"(d), "3"(5)  /* Condition code for UNLE */
        : "cc"
    );
    
    return result1 + result2 + result3 + result4;
}

/* Test 3: Array operations with various comparison macros */
NOINLINE int test_array_comparisons(void) {
    volatile double arr1[16];
    volatile double arr2[16];
    int counts[8] = {0};  /* For different condition codes */
    
    /* Initialize with mix of normal values and NaN */
    for (int i = 0; i < 16; i++) {
        arr1[i] = (i % 2 == 0) ? (double)i : NAN;
        arr2[i] = (i % 3 == 0) ? (double)(i * 2) : NAN;
    }
    
    /* Perform various comparisons that should generate different condition codes */
    for (int i = 0; i < 16; i++) {
        counts[0] += isunordered(arr1[i], arr2[i]) ? 1 : 0;      /* UNORDERED */
        counts[1] += !isunordered(arr1[i], arr2[i]) ? 1 : 0;     /* ORDERED */
        counts[2] += isgreater(arr1[i], arr2[i]) ? 1 : 0;        /* UNLE? Actually generates GT */
        counts[3] += isless(arr1[i], arr2[i]) ? 1 : 0;           /* UNGE? Actually generates LT */
        counts[4] += !isless(arr1[i], arr2[i]) ? 1 : 0;          /* UNGE (nlt) */
        counts[5] += !isgreater(arr1[i], arr2[i]) ? 1 : 0;       /* UNLE (nle) */
        
        /* UNEQ: unordered or equal */
        if (!isless(arr1[i], arr2[i]) && !isgreater(arr1[i], arr2[i]))
            counts[6]++;
            
        /* LTGT: less or greater (ordered and not equal) */
        if (isless(arr1[i], arr2[i]) || isgreater(arr1[i], arr2[i]))
            counts[7]++;
    }
    
    int total = 0;
    for (int i = 0; i < 8; i++) total += counts[i];
    return total;
}

/* Test 4: Long double (x87) operations for x87-specific condition codes */
NOINLINE int test_long_double_ops(void) {
    volatile long double ld_nan = NAN;
    volatile long double ld1 = 3.14159265358979323846L;
    volatile long double ld2 = 2.71828182845904523536L;
    volatile long double ld_inf = INFINITY;
    
    int results = 0;
    
    /* x87 style comparisons - may generate different condition codes */
    if (ld_nan != ld1) results += 1;          /* UNORDERED */
    if (ld1 == ld1) results += 2;             /* ORDERED */
    if (isless(ld1, ld2)) results += 4;       /* UNGE? */
    if (isgreater(ld1, ld2)) results += 8;    /* UNLE? */
    if (!isless(ld1, ld_inf)) results += 16;  /* UNGE (nlt) with INF */
    if (!isgreater(ld_inf, ld1)) results += 32; /* UNLE (nle) with INF */
    
    /* Complex expression to force multiple condition codes */
    volatile long double temp = ld1 + ld2;
    if (isunordered(temp, ld_nan) || islessequal(temp, ld_inf))
        results += 64;
    
    return results;
}

/* Test 5: Switch statement based on comparison results */
NOINLINE int test_switch_comparisons(void) {
    volatile double vals[] = {NAN, 1.0, 2.0, INFINITY, -INFINITY, 0.0};
    int result = 0;
    
    for (int i = 0; i < 6; i++) {
        double a = vals[i];
        double b = vals[(i + 1) % 6];
        
        /* Switch on comparison results - forces multiple condition codes */
        int cmp_result = 0;
        if (isunordered(a, b)) cmp_result = 1;        /* UNORDERED */
        else if (!isunordered(a, b)) cmp_result = 2;  /* ORDERED */
        else if (isgreater(a, b)) cmp_result = 3;     /* UNLE? */
        else if (isless(a, b)) cmp_result = 4;        /* UNGE? */
        else if (!isgreater(a, b)) cmp_result = 5;    /* UNLE (nle) */
        else if (!isless(a, b)) cmp_result = 6;       /* UNGE (nlt) */
        else cmp_result = 7;                          /* UNEQ or LTGT */
        
        switch (cmp_result) {
            case 1: result += 1; break;   /* UNORDERED */
            case 2: result += 2; break;   /* ORDERED */
            case 3: result += 4; break;   /* UNLE? */
            case 4: result += 8; break;   /* UNGE? */
            case 5: result += 16; break;  /* UNLE (nle) */
            case 6: result += 32; break;  /* UNGE (nlt) */
            case 7: result += 64; break;  /* UNEQ/LTGT */
        }
    }
    
    return result;
}

/* Test 6: Direct use of GCC builtins for SSE2 unordered compares */
NOINLINE int test_sse2_builtins(void) {
    volatile double a = 1.0;
    volatile double b = NAN;
    volatile double c = 2.0;
    volatile double d = 3.0;
    
    int results = 0;
    
    /* Using GCC's IA32 builtins for direct control */
    int cmp1 = __builtin_ia32_ucomisd(a, b);  /* UNORDERED compare */
    int cmp2 = __builtin_ia32_ucomisd(a, c);  /* ORDERED compare (a < c) */
    int cmp3 = __builtin_ia32_ucomisd(c, a);  /* ORDERED compare (c > a) */
    int cmp4 = __builtin_ia32_ucomisd(a, a);  /* ORDERED compare (equal) */
    
    /* Extract condition codes from comparison results */
    if (cmp1 & 4) results += 1;   /* UNORDERED flag */
    if (!(cmp2 & 4)) results += 2; /* ORDERED flag */
    if (cmp2 & 1) results += 4;   /* Less than flag */
    if (cmp3 & 0) results += 8;   /* Greater than (no flags) */
    if (cmp4 & 0x40) results += 16; /* Equal flag */
    
    /* Force generation of condition code names through conditional moves */
    double res1 = (isunordered(a, b)) ? a : b;
    double res2 = (!isunordered(c, d)) ? c : d;
    double res3 = (isgreater(a, c)) ? a : c;
    double res4 = (!isless(c, d)) ? c : d;
    
    results += (int)(res1 + res2 + res3 + res4);
    
    return results;
}

int main(void) {
    int total = 0;
    
    printf("Testing x86 floating-point condition code generation...\n");
    
    /* Run all tests to trigger various condition code paths */
    total += test_unordered_comparisons();
    total += test_asm_condition_codes();
    total += test_array_comparisons();
    total += test_long_double_ops();
    total += test_switch_comparisons();
    total += test_sse2_builtins();
    
    printf("Total checksum: %d\n", total);
    
    /* Additional volatile operations to prevent dead code elimination */
    volatile double final_check = NAN;
    if (isunordered(final_check, 0.0)) {
        printf("Final unordered check passed\n");
    }
    
    return 0;
}
