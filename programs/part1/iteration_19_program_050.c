/* test_float_conds.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <stdint.h>

/* Prevent optimizations from removing critical code */
#define NOINLINE __attribute__((noinline, noipa))
#define VOLATILE_DOUBLE volatile double
#define VOLATILE_LONG_DOUBLE volatile long double

/* Global volatile variables to prevent constant folding */
VOLATILE_DOUBLE g_nan = NAN;
VOLATILE_DOUBLE g_inf = INFINITY;
VOLATILE_DOUBLE g_normal = 3.14159;
VOLATILE_DOUBLE g_zero = 0.0;
VOLATILE_LONG_DOUBLE g_ld_nan = NAN;
VOLATILE_LONG_DOUBLE g_ld_inf = INFINITY;
VOLATILE_LONG_DOUBLE g_ld_normal = 3.14159265358979323846L;

/* Test 1: Direct unordered comparisons that should generate UNORDERED/ORDERED */
NOINLINE int test_unordered_comparisons(void) {
    int results[8] = {0};
    VOLATILE_DOUBLE nan1 = g_nan;
    VOLATILE_DOUBLE nan2 = g_nan;
    VOLATILE_DOUBLE normal = g_normal;
    
    /* These should generate UNORDERED condition code */
    results[0] = (nan1 != normal) ? 1 : 0;  /* unordered comparison */
    results[1] = (normal != nan1) ? 1 : 0;
    results[2] = (nan1 != nan2) ? 1 : 0;
    
    /* These should generate ORDERED condition code */
    results[3] = (normal == normal) ? 1 : 0;  /* ordered comparison */
    results[4] = (normal != normal) ? 0 : 1;  /* inverse of unordered */
    
    /* Mixed comparisons */
    results[5] = (normal < nan1) ? 1 : 0;
    results[6] = (nan1 > normal) ? 1 : 0;
    results[7] = (nan1 == nan1) ? 1 : 0;
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 2: Using math.h comparison macros */
NOINLINE int test_math_macros(void) {
    int results[12] = {0};
    VOLATILE_DOUBLE a = g_nan;
    VOLATILE_DOUBLE b = g_normal;
    VOLATILE_DOUBLE c = 2.71828;
    VOLATILE_DOUBLE d = g_nan;
    
    /* These use different condition codes */
    results[0] = isunordered(a, b);   /* UNORDERED */
    results[1] = isordered(b, c);     /* ORDERED */
    results[2] = !isgreater(a, b);    /* UNLE? */
    results[3] = !isless(b, a);       /* UNGE? */
    results[4] = islessequal(c, b);   /* LE but with unordered handling */
    results[5] = isgreaterequal(b, c); /* GE but with unordered handling */
    
    /* Direct comparisons that map to specific condition codes */
    results[6] = (a == a) ? 0 : 1;    /* UNEQ inverse? Actually LTGT */
    results[7] = !(a < b);            /* UNGE */
    results[8] = !(b > a);            /* UNLE */
    results[9] = !(a <= b);           /* UNGT */
    results[10] = !(b >= a);          /* UNLT */
    results[11] = (a != a) ? 1 : 0;   /* UNORDERED */
    
    int sum = 0;
    for (int i = 0; i < 12; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 3: Inline assembly with %C modifier to force condition code output */
NOINLINE int test_inline_asm(void) {
    int results[8] = {0};
    VOLATILE_DOUBLE x = g_nan;
    VOLATILE_DOUBLE y = g_normal;
    VOLATILE_DOUBLE z = 2.0;
    
    /* Test various condition codes via inline assembly */
    for (int i = 0; i < 8; i++) {
        int result;
        switch (i) {
            case 0:
                /* UNORDERED */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "setp %%al\n\t"
                    "movzbl %%al, %0"
                    : "=r"(result)
                    : "x"(x), "x"(y)
                    : "al", "cc"
                );
                results[i] = result;
                break;
                
            case 1:
                /* ORDERED */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "setnp %%al\n\t"
                    "movzbl %%al, %0"
                    : "=r"(result)
                    : "x"(g_normal), "x"(z)
                    : "al", "cc"
                );
                results[i] = result;
                break;
                
            case 2:
                /* UNEQ - unordered or equal */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "setbe %%al\n\t"
                    "movzbl %%al, %0"
                    : "=r"(result)
                    : "x"(x), "x"(x)
                    : "al", "cc"
                );
                results[i] = result;
                break;
                
            case 3:
                /* UNGE - not less than (unordered or greater or equal) */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "setnb %%al\n\t"
                    "movzbl %%al, %0"
                    : "=r"(result)
                    : "x"(y), "x"(z)
                    : "al", "cc"
                );
                results[i] = result;
                break;
                
            case 4:
                /* UNGT - not less or equal (unordered or greater) */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "setnbe %%al\n\t"
                    "movzbl %%al, %0"
                    : "=r"(result)
                    : "x"(y), "x"(z)
                    : "al", "cc"
                );
                results[i] = result;
                break;
                
            case 5:
                /* UNLE - unordered or less or equal */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "setna %%al\n\t"
                    "movzbl %%al, %0"
                    : "=r"(result)
                    : "x"(z), "x"(y)
                    : "al", "cc"
                );
                results[i] = result;
                break;
                
            case 6:
                /* UNLT - unordered or less than */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "setb %%al\n\t"
                    "movzbl %%al, %0"
                    : "=r"(result)
                    : "x"(z), "x"(y)
                    : "al", "cc"
                );
                results[i] = result;
                break;
                
            case 7:
                /* LTGT - less or greater (unordered gives false) */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "setne %%al\n\t"
                    "movzbl %%al, %0"
                    : "=r"(result)
                    : "x"(y), "x"(z)
                    : "al", "cc"
                );
                results[i] = result;
                break;
        }
    }
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 4: Array operations with unordered comparisons */
NOINLINE int test_array_operations(void) {
    VOLATILE_DOUBLE arr1[16];
    VOLATILE_DOUBLE arr2[16];
    
    /* Initialize arrays with mix of NaN and normal values */
    for (int i = 0; i < 16; i++) {
        if (i % 4 == 0) {
            arr1[i] = g_nan;
            arr2[i] = g_normal;
        } else if (i % 4 == 1) {
            arr1[i] = g_normal + i;
            arr2[i] = g_nan;
        } else if (i % 4 == 2) {
            arr1[i] = g_nan;
            arr2[i] = g_nan;
        } else {
            arr1[i] = g_normal + i;
            arr2[i] = g_normal - i;
        }
    }
    
    int counts[7] = {0};
    
    /* Count various comparison results */
    for (int i = 0; i < 16; i++) {
        counts[0] += isunordered(arr1[i], arr2[i]);    /* UNORDERED */
        counts[1] += isordered(arr1[i], arr2[i]);      /* ORDERED */
        counts[2] += (arr1[i] != arr2[i]) ? 1 : 0;     /* NE (includes UNORDERED) */
        counts[3] += (arr1[i] == arr2[i]) ? 1 : 0;     /* EQ (excludes UNORDERED) */
        counts[4] += isgreater(arr1[i], arr2[i]);      /* GT (excludes UNORDERED) */
        counts[5] += isless(arr1[i], arr2[i]);         /* LT (excludes UNORDERED) */
        counts[6] += !isunordered(arr1[i], arr2[i]) && 
                     (arr1[i] != arr2[i]);             /* LTGT */
    }
    
    int sum = 0;
    for (int i = 0; i < 7; i++) {
        sum += counts[i];
    }
    return sum;
}

/* Test 5: Long double (x87) operations */
NOINLINE int test_long_double_ops(void) {
    VOLATILE_LONG_DOUBLE a = g_ld_nan;
    VOLATILE_LONG_DOUBLE b = g_ld_normal;
    VOLATILE_LONG_DOUBLE c = 2.71828182845904523536L;
    
    int results[10] = {0};
    
    /* x87 style comparisons - these often generate different condition codes */
    results[0] = (a != b) ? 1 : 0;      /* unordered comparison */
    results[1] = (b == c) ? 1 : 0;      /* ordered comparison */
    results[2] = !(a < b) ? 1 : 0;      /* UNGE */
    results[3] = !(b > a) ? 1 : 0;      /* UNLE */
    results[4] = !(a <= b) ? 1 : 0;     /* UNGT */
    results[5] = !(b >= a) ? 1 : 0;     /* UNLT */
    results[6] = (a == a) ? 0 : 1;      /* LTGT (false for NaN) */
    results[7] = (b != c) ? 1 : 0;      /* NE (ordered) */
    
    /* Complex expression to force multiple condition codes */
    results[8] = ((a != a) || (b > c)) ? 1 : 0;
    results[9] = ((b == b) && (c < b)) ? 1 : 0;
    
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 6: Switch statement based on comparison results */
NOINLINE int test_switch_comparisons(void) {
    VOLATILE_DOUBLE vals[6] = {g_nan, g_normal, 2.71828, g_inf, -g_inf, 0.0};
    int total = 0;
    
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            int result = 0;
            
            /* This switch should generate various condition code checks */
            if (isunordered(vals[i], vals[j])) {
                result = 1;  /* UNORDERED */
            } else if (vals[i] < vals[j]) {
                result = 2;  /* LT */
            } else if (vals[i] > vals[j]) {
                result = 3;  /* GT */
            } else if (vals[i] == vals[j]) {
                result = 4;  /* EQ */
            } else {
                result = 5;  /* Should be UNORDERED case */
            }
            
            /* Additional checks to use more condition codes */
            if (!(vals[i] >= vals[j])) {  /* UNLT when unordered */
                result += 10;
            }
            if (!(vals[i] <= vals[j])) {  /* UNGT when unordered */
                result += 20;
            }
            if (vals[i] != vals[j]) {     /* NE (includes UNORDERED) */
                result += 30;
            }
            
            total += result;
        }
    }
    
    return total;
}

/* Test 7: GCC builtins for direct unordered comparisons */
NOINLINE int test_gcc_builtins(void) {
    double d1 = g_nan;
    double d2 = g_normal;
    double d3 = 2.0;
    int results[6] = {0};
    
    /* Using GCC's x86 specific builtins */
    results[0] = __builtin_isgreater(d2, d3);
    results[1] = __builtin_isless(d3, d2);
    results[2] = __builtin_isunordered(d1, d2);
    results[3] = __builtin_isordered(d2, d3);
    results[4] = __builtin_isgreaterequal(d2, d3);
    results[5] = __builtin_islessequal(d3, d2);
    
    int sum = 0;
    for (int i = 0; i < 6; i++) {
        sum += results[i];
    }
    return sum;
}

int main(void) {
    printf("Testing x86 floating-point condition codes...\n");
    
    int total = 0;
    
    total += test_unordered_comparisons();
    printf("Test 1 result: %d\n", test_unordered_comparisons());
    
    total += test_math_macros();
    printf("Test 2 result: %d\n", test_math_macros());
    
    total += test_inline_asm();
    printf("Test 3 result: %d\n", test_inline_asm());
    
    total += test_array_operations();
    printf("Test 4 result: %d\n", test_array_operations());
    
    total += test_long_double_ops();
    printf("Test 5 result: %d\n", test_long_double_ops());
    
    total += test_switch_comparisons();
    printf("Test 6 result: %d\n", test_switch_comparisons());
    
    total += test_gcc_builtins();
    printf("Test 7 result: %d\n", test_gcc_builtins());
    
    printf("Total checksum: %d\n", total);
    
    return 0;
}
