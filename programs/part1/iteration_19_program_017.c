/* Test program to trigger x86 condition code printing for floating-point comparisons */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Prevent optimizations from removing critical code */
#define NOINLINE __attribute__((noinline, noipa))

/* Volatile variables to prevent constant folding */
volatile double dnan = NAN;
volatile double dinf = INFINITY;
volatile double dneg = -1.0;
volatile double dpos = 1.0;
volatile double dzero = 0.0;

volatile long double lnan = NAN;
volatile long double linf = INFINITY;
volatile long double lneg = -1.0L;
volatile long double lpos = 1.0L;
volatile long double lzero = 0.0L;

/* Test 1: Direct unordered comparisons with != operator */
NOINLINE int test_unordered_comparisons(void) {
    int results[8] = {0};
    
    /* These should generate UNORDERED/ORDERED condition codes */
    results[0] = (dnan != dpos) ? 1 : 0;      /* UNORDERED likely */
    results[1] = (dpos != dnan) ? 1 : 0;      /* UNORDERED likely */
    results[2] = (dnan == dnan) ? 1 : 0;      /* UNORDERED likely */
    results[3] = (dpos == dpos) ? 1 : 0;      /* ORDERED likely */
    
    /* Mixed comparisons */
    results[4] = (dinf != dnan) ? 1 : 0;
    results[5] = (dzero == dnan) ? 1 : 0;
    results[6] = (dnan != dinf) ? 1 : 0;
    results[7] = (dinf == dinf) ? 1 : 0;
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 2: Using math.h comparison macros */
NOINLINE int test_math_macros(void) {
    int results[12] = {0};
    
    /* These map to specific condition codes */
    results[0] = isunordered(dnan, dpos);     /* UNORDERED */
    results[1] = isordered(dpos, dneg);       /* ORDERED */
    results[2] = !isgreater(dnan, dpos);      /* UNGT/UNLE? */
    results[3] = isless(dnan, dpos);          /* UNLT? */
    results[4] = islessequal(dnan, dpos);     /* UNLE */
    results[5] = isgreaterequal(dnan, dpos);  /* UNGE */
    
    /* More complex expressions */
    results[6] = isunordered(dinf, dnan) || isless(dneg, dpos);
    results[7] = isordered(dzero, dpos) && isgreater(dpos, dneg);
    results[8] = !isunordered(dpos, dneg) && islessequal(dneg, dzero);
    results[9] = isgreater(dinf, dpos) || isunordered(dnan, dnan);
    results[10] = isless(dneg, dinf) && isordered(dinf, dinf);
    results[11] = isgreaterequal(dnan, dnan) || isless(dpos, dnan);
    
    int sum = 0;
    for (int i = 0; i < 12; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 3: Inline assembly with %C modifier for condition codes */
NOINLINE int test_inline_asm(void) {
    int results[6] = {0};
    unsigned char byte_result;
    
    /* Test various condition codes through inline assembly */
    for (int i = 0; i < 6; i++) {
        double a = (i & 1) ? dnan : dpos;
        double b = (i & 2) ? dnan : dneg;
        
        /* Using %C to get condition code name */
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

/* Test 4: Long double (x87) comparisons */
NOINLINE int test_long_double_comparisons(void) {
    int results[8] = {0};
    
    /* x87 style comparisons - may generate different condition codes */
    results[0] = (lnan != lpos) ? 1 : 0;
    results[1] = (lpos == lneg) ? 1 : 0;
    results[2] = (linf > lpos) ? 1 : 0;
    results[3] = (lneg < lzero) ? 1 : 0;
    results[4] = (lnan >= lnan) ? 1 : 0;
    results[5] = (lpos <= linf) ? 1 : 0;
    results[6] = (lnan == lnan) ? 1 : 0;
    results[7] = (linf != linf) ? 1 : 0;
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 5: Array-based comparisons with control flow */
NOINLINE int test_array_comparisons(void) {
    volatile double arr1[16], arr2[16];
    int counts[6] = {0};  /* unordered, ordered, greater, less, equal, nequal */
    
    /* Initialize arrays with mix of values */
    for (int i = 0; i < 16; i++) {
        switch (i % 4) {
            case 0: arr1[i] = dnan; arr2[i] = dpos; break;
            case 1: arr1[i] = dpos; arr2[i] = dneg; break;
            case 2: arr1[i] = dinf; arr2[i] = dinf; break;
            case 3: arr1[i] = dzero; arr2[i] = dnan; break;
        }
    }
    
    /* Complex loop with multiple comparisons */
    for (int i = 0; i < 16; i++) {
        if (isunordered(arr1[i], arr2[i])) {
            counts[0]++;  /* UNORDERED */
        } else if (isordered(arr1[i], arr2[i])) {
            counts[1]++;  /* ORDERED */
        }
        
        if (isgreater(arr1[i], arr2[i])) {
            counts[2]++;  /* GT */
        }
        
        if (isless(arr1[i], arr2[i])) {
            counts[3]++;  /* LT */
        }
        
        if (arr1[i] == arr2[i]) {
            counts[4]++;  /* EQ */
        }
        
        if (arr1[i] != arr2[i]) {
            counts[5]++;  /* NEQ/UNEQ */
        }
    }
    
    int sum = 0;
    for (int i = 0; i < 6; i++) {
        sum += counts[i];
    }
    return sum;
}

/* Test 6: Switch based on comparison results */
NOINLINE int test_switch_comparisons(void) {
    int result = 0;
    
    /* Force compiler to generate multiple condition code checks */
    for (int i = 0; i < 8; i++) {
        double a = (i & 1) ? dnan : (i * 1.5);
        double b = (i & 2) ? dinf : (i * 0.5);
        
        int cmp_result;
        if (isunordered(a, b)) {
            cmp_result = 0;  /* UNORDERED */
        } else if (isgreater(a, b)) {
            cmp_result = 1;  /* GT */
        } else if (isless(a, b)) {
            cmp_result = 2;  /* LT */
        } else if (a == b) {
            cmp_result = 3;  /* EQ */
        } else {
            cmp_result = 4;  /* UNEQ/LTGT */
        }
        
        /* Switch to force different code paths */
        switch (cmp_result) {
            case 0: result += 1; break;  /* UNORDERED */
            case 1: result += 2; break;  /* GT */
            case 2: result += 3; break;  /* LT */
            case 3: result += 4; break;  /* EQ */
            case 4: result += 5; break;  /* UNEQ/LTGT */
        }
    }
    
    return result;
}

/* Test 7: Direct GCC builtins for SSE2 comparisons */
NOINLINE int test_sse2_builtins(void) {
    int results[4] = {0};
    
    /* Using GCC's x86 intrinsics */
    for (int i = 0; i < 4; i++) {
        double a = (i & 1) ? dnan : dpos;
        double b = (i & 2) ? dnan : dneg;
        
        /* __builtin_ia32_ucomisd generates UNORDERED comparison */
        int cmp = __builtin_ia32_ucomisd(a, b);
        
        /* Check different condition flags */
        if (cmp & 1) {  /* Unordered */
            results[0]++;
        }
        if (cmp & 2) {  /* Greater than */
            results[1]++;
        }
        if (cmp & 4) {  /* Less than */
            results[2]++;
        }
        if (cmp == 0) {  /* Equal */
            results[3]++;
        }
    }
    
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += results[i];
    }
    return sum;
}

/* Main function that runs all tests */
int main(void) {
    int total = 0;
    
    printf("Running x86 condition code tests...\n");
    
    /* Run all test functions */
    total += test_unordered_comparisons();
    total += test_math_macros();
    total += test_inline_asm();
    total += test_long_double_comparisons();
    total += test_array_comparisons();
    total += test_switch_comparisons();
    total += test_sse2_builtins();
    
    printf("Total checksum: %d\n", total);
    
    /* Also do some direct prints to force assembly output */
    volatile double x = NAN;
    volatile double y = 1.0;
    
    /* These should generate condition code output in assembly */
    if (x != y) {
        printf("unordered\n");
    }
    
    if (isunordered(x, y)) {
        printf("isunordered\n");
    }
    
    if (!isgreater(x, y)) {
        printf("not greater\n");
    }
    
    if (isless(x, y)) {
        printf("is less\n");
    }
    
    return total != 0 ? 0 : 1;
}
