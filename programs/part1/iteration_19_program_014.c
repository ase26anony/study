/* Test program to trigger x86 floating-point condition code output */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Prevent optimizations from removing critical code */
#define NOINLINE __attribute__((noinline, noipa))

/* Volatile variables to prevent constant folding */
static volatile double dnan = NAN;
static volatile double dinf = INFINITY;
static volatile double dneg = -1.0;
static volatile double dpos = 1.0;
static volatile double dzero = 0.0;

static volatile long double lnan;
static volatile long double linf;
static volatile long double lneg = -1.0L;
static volatile long double lpos = 1.0L;

/* Initialize long double NaN/INF */
__attribute__((constructor))
static void init_long_doubles(void) {
    lnan = strtold("NAN", NULL);
    linf = strtold("INF", NULL);
}

/* Test 1: Direct unordered comparisons with != operator */
NOINLINE
static int test_unordered_comparisons(void) {
    int results[8] = {0};
    
    /* These should generate UNORDERED/ORDERED condition codes */
    results[0] = (dnan != dpos) ? 1 : 0;      /* UNORDERED expected */
    results[1] = (dpos == dpos) ? 1 : 0;      /* ORDERED expected */
    results[2] = (dnan == dnan) ? 1 : 0;      /* UNORDERED/UNEQ? */
    results[3] = (dinf != dinf) ? 1 : 0;      /* ORDERED? */
    
    /* Mixed comparisons */
    results[4] = (dnan != dzero) ? 1 : 0;
    results[5] = (dzero == dnan) ? 1 : 0;
    results[6] = (dinf == dpos) ? 1 : 0;
    results[7] = (dneg != dinf) ? 1 : 0;
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 2: Using math.h comparison macros */
NOINLINE
static int test_math_macros(void) {
    int results[12] = {0};
    
    /* These map to specific condition codes */
    results[0] = isunordered(dnan, dpos);     /* UNORDERED */
    results[1] = !isunordered(dpos, dpos);    /* ORDERED */
    results[2] = isgreater(dpos, dneg);       /* UNLE? Actually generates GT */
    results[3] = isgreaterequal(dpos, dzero); /* UNLT? Actually generates GE */
    results[4] = isless(dneg, dpos);          /* UNGE? Actually generates LT */
    results[5] = islessequal(dzero, dzero);   /* UNGT? Actually generates LE */
    
    /* Direct unordered comparisons */
    results[6] = (dnan < dpos) ? 1 : 0;       /* UNORDERED/UNGE? */
    results[7] = (dpos > dnan) ? 1 : 0;       /* UNORDERED/UNLE? */
    results[8] = (dnan <= dpos) ? 1 : 0;      /* UNORDERED/UNGT? */
    results[9] = (dpos >= dnan) ? 1 : 0;      /* UNORDERED/UNLT? */
    
    /* Equality with NaN */
    results[10] = (dnan == dnan) ? 1 : 0;     /* UNEQ? */
    results[11] = (dnan != dnan) ? 1 : 0;     /* LTGT? */
    
    int sum = 0;
    for (int i = 0; i < 12; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 3: Inline assembly with %C modifier */
NOINLINE
static int test_inline_asm(void) {
    int results[6] = {0};
    unsigned char byte_result;
    
    /* Test various condition codes via inline assembly */
    for (int i = 0; i < 6; i++) {
        double a = (i & 1) ? dnan : dpos;
        double b = (i & 2) ? dnan : dneg;
        
        /* Using %C to force condition code substitution */
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "set%C0 %0"
            : "=r"(byte_result)
            : "x"(a), "x"(b)
            : "cc"
        );
        results[i] = byte_result;
    }
    
    int sum = 0;
    for (int i = 0; i < 6; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 4: Long double x87 comparisons */
NOINLINE
static int test_long_double_comparisons(void) {
    int results[8] = {0};
    
    /* x87 style comparisons - may generate different condition codes */
    results[0] = (lnan != lpos) ? 1 : 0;
    results[1] = (lpos == lpos) ? 1 : 0;
    results[2] = (lnan == lnan) ? 1 : 0;
    results[3] = (linf != linf) ? 1 : 0;
    
    /* Force x87 instructions */
    volatile long double a = lnan;
    volatile long double b = lpos;
    volatile long double c = lneg;
    
    results[4] = (a > b) ? 1 : 0;
    results[5] = (b < c) ? 1 : 0;
    results[6] = (a >= b) ? 1 : 0;
    results[7] = (b <= c) ? 1 : 0;
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 5: Array-based unordered comparison counting */
NOINLINE
static int test_array_unordered(void) {
    volatile double arr1[16];
    volatile double arr2[16];
    
    /* Fill arrays with mix of normal values and NaN */
    for (int i = 0; i < 16; i++) {
        if (i % 3 == 0) {
            arr1[i] = dnan;
            arr2[i] = (double)i;
        } else if (i % 3 == 1) {
            arr1[i] = (double)i;
            arr2[i] = dnan;
        } else {
            arr1[i] = (double)i;
            arr2[i] = (double)(15 - i);
        }
    }
    
    /* Count various comparison results */
    int unordered_count = 0;
    int ordered_count = 0;
    int greater_count = 0;
    int less_count = 0;
    
    for (int i = 0; i < 16; i++) {
        unordered_count += isunordered(arr1[i], arr2[i]);
        ordered_count += !isunordered(arr1[i], arr2[i]);
        greater_count += isgreater(arr1[i], arr2[i]);
        less_count += isless(arr1[i], arr2[i]);
    }
    
    return unordered_count + ordered_count + greater_count + less_count;
}

/* Test 6: Switch based on floating-point classification */
NOINLINE
static int test_fpclassify_switch(void) {
    volatile double values[8] = {
        dnan, dinf, -dinf, dzero, -dzero, dpos, dneg, 2.5
    };
    
    int results[8] = {0};
    
    for (int i = 0; i < 8; i++) {
        int c = fpclassify(values[i]);
        
        /* Switch to force multiple condition code usages */
        switch (c) {
            case FP_NAN:
                results[i] = 1;
                break;
            case FP_INFINITE:
                results[i] = 2;
                break;
            case FP_ZERO:
                results[i] = 3;
                break;
            case FP_SUBNORMAL:
                results[i] = 4;
                break;
            case FP_NORMAL:
                results[i] = 5;
                break;
            default:
                results[i] = 0;
        }
    }
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 7: Direct GCC builtins for SSE2 unordered compare */
NOINLINE
static int test_sse2_builtins(void) {
    int results[4] = {0};
    
    /* Using GCC's x86 intrinsics */
    __v2df a = { dnan, dpos };
    __v2df b = { dpos, dnan };
    
    /* This may generate condition code output */
    __v2di cmp_result = __builtin_ia32_cmpneqpd(a, b);
    results[0] = cmp_result[0] != 0;
    results[1] = cmp_result[1] != 0;
    
    /* Ordered compare */
    cmp_result = __builtin_ia32_cmpeqpd(a, b);
    results[2] = cmp_result[0] != 0;
    results[3] = cmp_result[1] != 0;
    
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += results[i];
    }
    return sum;
}

/* Main function that runs all tests */
int main(void) {
    int total = 0;
    
    printf("Running floating-point condition code tests...\n");
    
    total += test_unordered_comparisons();
    total += test_math_macros();
    total += test_inline_asm();
    total += test_long_double_comparisons();
    total += test_array_unordered();
    total += test_fpclassify_switch();
    total += test_sse2_builtins();
    
    printf("Total checksum: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total > 1000) {
        printf("Unexpected large result\n");
    }
    
    return 0;
}
