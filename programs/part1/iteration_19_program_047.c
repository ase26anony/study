/* test_x86_condcodes.c
 * Compile with: gcc -O2 -march=x86-64 -mfpmath=sse -ffast-math -S test_x86_condcodes.c
 * Also try: gcc -O3 -m32 -mfpmath=387 -march=i686 -S test_x86_condcodes.c
 */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Prevent optimizations from removing critical code */
#define NOINLINE __attribute__((noinline, noipa))

/* Test 1: Direct unordered comparisons with NaN */
NOINLINE int test_unordered_comparisons(void) {
    volatile double nan_val = NAN;
    volatile double normal_val = 3.14159;
    volatile double inf_val = INFINITY;
    int results[8] = {0};
    
    /* These should generate UNORDERED/ORDERED condition codes */
    results[0] = (nan_val != normal_val) ? 1 : 0;      /* May generate "unord" */
    results[1] = (nan_val == nan_val) ? 1 : 0;         /* May generate "ord" */
    results[2] = !(nan_val < normal_val) ? 1 : 0;      /* May generate "nlt" (UNGE) */
    results[3] = !(nan_val <= normal_val) ? 1 : 0;     /* May generate "nle" (UNGT) */
    
    /* Direct use of comparison macros */
    results[4] = isunordered(nan_val, normal_val);
    results[5] = isgreater(normal_val, nan_val);
    results[6] = isless(nan_val, normal_val);
    results[7] = !islessgreater(normal_val, inf_val);  /* May generate "ueq" */
    
    int sum = 0;
    for (int i = 0; i < 8; i++) sum += results[i];
    return sum;
}

/* Test 2: Inline assembly with %C modifier for condition codes */
NOINLINE int test_asm_condition_codes(void) {
    volatile double a = NAN;
    volatile double b = 2.71828;
    volatile double c = -INFINITY;
    int results[6] = {0};
    
    /* Force generation of various condition codes through inline asm */
    for (int i = 0; i < 6; i++) {
        int tmp;
        switch (i) {
            case 0: /* UNORDERED */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "set%C0 %0"
                    : "=r"(tmp) 
                    : "x"(a), "x"(b)
                    : "cc"
                );
                break;
            case 1: /* ORDERED */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "set%C0 %0"
                    : "=r"(tmp)
                    : "x"(b), "x"(a)
                    : "cc"
                );
                break;
            case 2: /* UNEQ */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "set%C0 %0"
                    : "=r"(tmp)
                    : "x"(b), "x"(b)
                    : "cc"
                );
                break;
            case 3: /* UNGE */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "set%C0 %0"
                    : "=r"(tmp)
                    : "x"(c), "x"(b)
                    : "cc"
                );
                break;
            case 4: /* UNGT */
                __asm__ volatile (
                    "comisd %1, %2\n\t"
                    "set%C0 %0"
                    : "=r"(tmp)
                    : "x"(b), "x"(c)
                    : "cc"
                );
                break;
            case 5: /* LTGT */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "set%C0 %0"
                    : "=r"(tmp)
                    : "x"(a), "x"(c)
                    : "cc"
                );
                break;
        }
        results[i] = tmp;
    }
    
    int sum = 0;
    for (int i = 0; i < 6; i++) sum += results[i];
    return sum;
}

/* Test 3: Array operations with various comparisons */
NOINLINE int test_array_comparisons(void) {
    volatile double arr1[8];
    volatile double arr2[8];
    
    /* Initialize with mix of values */
    for (int i = 0; i < 8; i++) {
        arr1[i] = (i % 2 == 0) ? (double)i : NAN;
        arr2[i] = (i % 3 == 0) ? (double)(i * 2) : INFINITY;
    }
    
    int counts[7] = {0}; /* For different condition codes */
    
    for (int i = 0; i < 8; i++) {
        /* Test various conditions that might generate different codes */
        if (isunordered(arr1[i], arr2[i])) counts[0]++;  /* UNORDERED */
        if (!isunordered(arr1[i], arr2[i])) counts[1]++; /* ORDERED */
        if (!isless(arr1[i], arr2[i])) counts[2]++;      /* UNGE (nlt) */
        if (!islessequal(arr1[i], arr2[i])) counts[3]++; /* UNGT (nle) */
        if (islessequal(arr1[i], arr2[i])) counts[4]++;  /* UNLE (ule) */
        if (isless(arr1[i], arr2[i])) counts[5]++;       /* UNLT (ult) */
        if (islessgreater(arr1[i], arr2[i])) counts[6]++; /* LTGT (une) */
    }
    
    int sum = 0;
    for (int i = 0; i < 7; i++) sum += counts[i];
    return sum;
}

/* Test 4: Long double (x87) operations */
NOINLINE int test_long_double_ops(void) {
    volatile long double ld_nan = NAN;
    volatile long double ld_inf = INFINITY;
    volatile long double ld_val = 3.14159265358979323846L;
    
    int results = 0;
    
    /* x87 comparisons - may generate different condition codes */
    if (ld_nan != ld_val) results += 1;      /* UNORDERED */
    if (ld_nan == ld_nan) results += 2;      /* ORDERED */
    if (!(ld_val < ld_inf)) results += 4;    /* UNGE */
    if (!(ld_val <= ld_inf)) results += 8;   /* UNGT */
    if (ld_val <= ld_inf) results += 16;     /* UNLE */
    if (ld_val < ld_inf) results += 32;      /* UNLT */
    if (ld_val != ld_inf) results += 64;     /* LTGT */
    
    /* Force x87 compare instruction */
    volatile long double a = 1.0L;
    volatile long double b = 2.0L;
    int cmp_result;
    __asm__ volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "setne %0"
        : "=r"(cmp_result)
        : "m"(a), "m"(b)
        : "cc", "st"
    );
    
    return results + cmp_result;
}

/* Test 5: Switch statement based on comparison results */
NOINLINE int test_switch_comparisons(void) {
    volatile double vals[4] = {NAN, INFINITY, -INFINITY, 42.0};
    int total = 0;
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int code = 0;
            
            /* Complex comparison that might generate various condition codes */
            if (isunordered(vals[i], vals[j])) {
                code = 1;  /* UNORDERED */
            } else if (!isless(vals[i], vals[j]) && !isgreater(vals[i], vals[j])) {
                code = 2;  /* UNEQ */
            } else if (isless(vals[i], vals[j])) {
                code = 3;  /* UNLT */
            } else if (isgreater(vals[i], vals[j])) {
                code = 4;  /* UNGT */
            }
            
            /* Switch to force multiple conditional jumps */
            switch (code) {
                case 1: total += 100; break;  /* UNORDERED path */
                case 2: total += 200; break;  /* UNEQ path */
                case 3: total += 300; break;  /* UNLT path */
                case 4: total += 400; break;  /* UNGT path */
                default: total += 500; break; /* ORDERED but not equal */
            }
        }
    }
    
    return total;
}

/* Test 6: GCC builtins for direct unordered compares */
NOINLINE int test_gcc_builtins(void) {
    volatile double a = NAN;
    volatile double b = 1.0;
    volatile double c = 2.0;
    int res = 0;
    
    /* Use GCC's x86-specific builtins */
    res += __builtin_ia32_ucomisd(a, b);  /* UNORDERED compare */
    res += __builtin_ia32_ucomisd(b, a);  /* ORDERED compare */
    res += __builtin_ia32_ucomisd(b, c);  /* UNLT or UNGT */
    res += __builtin_ia32_ucomisd(c, b);  /* Opposite condition */
    
    /* Conditional move based on compare */
    double d = 0.0;
    if (__builtin_ia32_ucomisd(a, b) & 0x40) {  /* Check parity flag */
        d = 1.0;  /* UNORDERED case */
    }
    
    return res + (int)d;
}

int main(void) {
    int checksum = 0;
    
    printf("Testing x86 condition code generation...\n");
    
    /* Run all tests to trigger various condition codes */
    checksum += test_unordered_comparisons();
    checksum += test_asm_condition_codes();
    checksum += test_array_comparisons();
    checksum += test_long_double_ops();
    checksum += test_switch_comparisons();
    checksum += test_gcc_builtins();
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
