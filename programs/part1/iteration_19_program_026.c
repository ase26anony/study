/* test_x86_condcodes.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

/* Prevent optimizations from removing critical code */
#define NOOPT __attribute__((noinline, noipa, optimize("O0")))

/* Global volatile variables to prevent constant folding */
volatile double vnan = NAN;
volatile double vinf = INFINITY;
volatile double vneg_inf = -INFINITY;
volatile double vzero = 0.0;
volatile double vone = 1.0;
volatile double vneg_one = -1.0;

/* Test 1: Direct unordered comparisons that should generate UNORDERED/ORDERED */
NOOPT int test_unordered_ordered(void) {
    volatile double a = vnan;
    volatile double b = vone;
    volatile double c = vzero;
    volatile double d = vinf;
    
    int results[8] = {0};
    
    /* These should generate UNORDERED condition code */
    results[0] = (a != b) ? 1 : 0;  /* NaN != 1.0 -> unordered */
    results[1] = (b != a) ? 1 : 0;  /* 1.0 != NaN -> unordered */
    results[2] = (a != a) ? 1 : 0;  /* NaN != NaN -> unordered */
    
    /* These should generate ORDERED condition code */
    results[3] = (c == d) ? 1 : 0;  /* 0.0 == INF -> false but ordered */
    results[4] = (b == c) ? 1 : 0;  /* 1.0 == 0.0 -> false but ordered */
    
    /* Mixed ordered/unordered comparisons */
    results[5] = (a == b) ? 1 : 0;  /* NaN == 1.0 -> false, unordered */
    results[6] = (b == a) ? 1 : 0;  /* 1.0 == NaN -> false, unordered */
    results[7] = (c != d) ? 1 : 0;  /* 0.0 != INF -> true, ordered */
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 2: Using math.h comparison macros */
NOOPT int test_math_macros(void) {
    volatile double a = vnan;
    volatile double b = vone;
    volatile double c = vzero;
    volatile double d = vinf;
    volatile double e = vneg_inf;
    
    int results[12] = {0};
    
    /* UNEQ: unordered or equal */
    results[0] = !isgreater(a, b) && !isless(a, b);
    
    /* UNGE: unordered or greater or equal */
    results[1] = !isless(a, b);
    
    /* UNGT: unordered or greater */
    results[2] = !isless(a, b) && !isunordered(a, b);
    
    /* UNLE: unordered or less or equal */
    results[3] = !isgreater(a, b);
    
    /* UNLT: unordered or less */
    results[4] = !isgreater(a, b) && !isunordered(a, b);
    
    /* LTGT: less or greater (ordered and not equal) */
    results[5] = isless(c, b) || isgreater(b, c);
    
    /* Test all macros explicitly */
    results[6] = isunordered(a, b);
    results[7] = isgreater(b, c);
    results[8] = isgreaterequal(b, c);
    results[9] = isless(c, b);
    results[10] = islessequal(c, b);
    results[11] = islessgreater(b, c);
    
    int sum = 0;
    for (int i = 0; i < 12; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 3: Inline assembly with %C modifier */
NOOPT int test_inline_asm(void) {
    volatile double a = vnan;
    volatile double b = vone;
    volatile double c = vzero;
    int result1 = 0, result2 = 0, result3 = 0;
    
    /* Test UNORDERED with inline assembly */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C0 %0"
        : "=r"(result1)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* Test ORDERED with inline assembly */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C0 %0"
        : "=r"(result2)
        : "x"(c), "x"(b)
        : "cc"
    );
    
    /* Test UNEQ with inline assembly */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C0 %0"
        : "=r"(result3)
        : "x"(c), "x"(c)
        : "cc"
    );
    
    return result1 + result2 + result3;
}

/* Test 4: Long double (x87) comparisons */
NOOPT int test_long_double(void) {
    volatile long double a = vnan;
    volatile long double b = vone;
    volatile long double c = vzero;
    volatile long double d = vinf;
    
    int results[6] = {0};
    
    /* x87 style comparisons */
    results[0] = (a != b) ? 1 : 0;
    results[1] = (b == c) ? 1 : 0;
    results[2] = (c < d) ? 1 : 0;
    results[3] = (d > c) ? 1 : 0;
    results[4] = (a == a) ? 1 : 0;
    results[5] = (b != c) ? 1 : 0;
    
    int sum = 0;
    for (int i = 0; i < 6; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 5: Array-based unordered comparisons */
NOOPT int test_array_comparisons(void) {
    volatile double arr1[8];
    volatile double arr2[8];
    
    /* Initialize with mix of values */
    for (int i = 0; i < 8; i++) {
        if (i % 4 == 0) {
            arr1[i] = vnan;
            arr2[i] = (double)i;
        } else if (i % 4 == 1) {
            arr1[i] = (double)i;
            arr2[i] = vnan;
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
    
    for (int i = 0; i < 8; i++) {
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
    }
    
    return unordered_count + ordered_count + greater_count + less_count;
}

/* Test 6: Switch statement based on comparison results */
NOOPT int test_switch_comparisons(void) {
    volatile double a = vnan;
    volatile double b = vone;
    volatile double c = vzero;
    volatile double d = vinf;
    
    int result = 0;
    
    /* Force compiler to generate multiple condition code checks */
    if (isunordered(a, b)) {
        result |= 1;  /* UNORDERED */
    }
    
    if (!isunordered(c, d)) {
        result |= 2;  /* ORDERED */
    }
    
    if (!isgreater(a, b) && !isless(a, b)) {
        result |= 4;  /* UNEQ */
    }
    
    if (!isless(a, b)) {
        result |= 8;  /* UNGE */
    }
    
    if (!isless(a, b) && !isunordered(a, b)) {
        result |= 16; /* UNGT */
    }
    
    if (!isgreater(a, b)) {
        result |= 32; /* UNLE */
    }
    
    if (!isgreater(a, b) && !isunordered(a, b)) {
        result |= 64; /* UNLT */
    }
    
    if (isless(c, b) || isgreater(b, c)) {
        result |= 128; /* LTGT */
    }
    
    return result;
}

/* Test 7: Direct GCC builtins for SSE2 unordered compares */
NOOPT int test_sse2_builtins(void) {
    volatile double a = vnan;
    volatile double b = vone;
    volatile double c = vzero;
    
    int results[4] = {0};
    
    /* Use GCC's x86 intrinsics */
    results[0] = __builtin_ia32_ucomisd(a, b);
    results[1] = __builtin_ia32_ucomisd(b, c);
    results[2] = __builtin_ia32_ucomisd(c, c);
    results[3] = __builtin_ia32_ucomisd(a, a);
    
    /* Extract condition code results */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        /* The result is in EFLAGS, we need to check it */
        int zf, pf, cf;
        __asm__ volatile (
            "pushf\n\t"
            "pop %0\n\t"
            : "=r"(sum)
        );
        /* Check specific flag bits for unordered/ordered */
        if (results[i] & 0x44) {  /* Check ZF and PF bits */
            sum += 1;
        }
    }
    
    return sum;
}

int main(void) {
    int total = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Run all tests */
    total += test_unordered_ordered();
    total += test_math_macros();
    total += test_inline_asm();
    total += test_long_double();
    total += test_array_comparisons();
    total += test_switch_comparisons();
    total += test_sse2_builtins();
    
    printf("Total checksum: %d\n", total);
    
    /* Also test with volatile function calls to prevent dead code elimination */
    volatile int dummy = total;
    
    return 0;
}
