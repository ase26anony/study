/* Test program to trigger x86 floating-point condition code output */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Prevent optimizations from removing critical code */
#define NOINLINE __attribute__((noinline, noipa))
#define VOLATILE_DOUBLE volatile double
#define VOLATILE_LONG_DOUBLE volatile long double

/* Test 1: Direct unordered comparisons with NaN */
NOINLINE int test_unordered_comparisons(VOLATILE_DOUBLE a, VOLATILE_DOUBLE b) {
    int results[8] = {0};
    
    /* These should generate various condition codes */
    results[0] = (a != b) ? 1 : 0;          /* May generate UNORDERED or NE */
    results[1] = (a == b) ? 1 : 0;          /* May generate ORDERED or EQ */
    
    /* Use math.h comparison macros */
    results[2] = isunordered(a, b) ? 1 : 0;  /* Should generate UNORDERED */
    results[3] = !isunordered(a, b) ? 1 : 0; /* Should generate ORDERED */
    
    results[4] = isgreater(a, b) ? 1 : 0;    /* Should generate GT */
    results[5] = isless(a, b) ? 1 : 0;       /* Should generate LT */
    results[6] = isgreaterequal(a, b) ? 1 : 0; /* Should generate GE */
    results[7] = islessequal(a, b) ? 1 : 0;  /* Should generate LE */
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 2: Inline assembly with %C modifier for condition codes */
NOINLINE int test_asm_condition_codes(VOLATILE_DOUBLE a, VOLATILE_DOUBLE b) {
    int result_unord = 0, result_ord = 0, result_ueq = 0;
    int result_unge = 0, result_ungt = 0, result_unle = 0;
    int result_unlt = 0, result_ltgt = 0;
    
    /* Test UNORDERED condition */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result_unord)
        : "x"(a), "x"(b), "C"(UNORDERED)
        : "cc"
    );
    
    /* Test ORDERED condition */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result_ord)
        : "x"(a), "x"(b), "C"(ORDERED)
        : "cc"
    );
    
    /* Test UNEQ condition */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result_ueq)
        : "x"(a), "x"(b), "C"(UNEQ)
        : "cc"
    );
    
    /* Test UNGE condition */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result_unge)
        : "x"(a), "x"(b), "C"(UNGE)
        : "cc"
    );
    
    /* Test UNGT condition */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result_ungt)
        : "x"(a), "x"(b), "C"(UNGT)
        : "cc"
    );
    
    /* Test UNLE condition */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result_unle)
        : "x"(a), "x"(b), "C"(UNLE)
        : "cc"
    );
    
    /* Test UNLT condition */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result_unlt)
        : "x"(a), "x"(b), "C"(UNLT)
        : "cc"
    );
    
    /* Test LTGT condition */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result_ltgt)
        : "x"(a), "x"(b), "C"(LTGT)
        : "cc"
    );
    
    return result_unord + result_ord + result_ueq + result_unge +
           result_ungt + result_unle + result_unlt + result_ltgt;
}

/* Test 3: Array-based unordered comparisons */
NOINLINE int test_array_comparisons(VOLATILE_DOUBLE *arr1, VOLATILE_DOUBLE *arr2, int n) {
    int unordered_count = 0;
    int ordered_count = 0;
    int greater_count = 0;
    int less_count = 0;
    
    for (int i = 0; i < n; i++) {
        /* Force multiple different comparison types */
        if (isunordered(arr1[i], arr2[i])) {
            unordered_count++;
        } else if (!isunordered(arr1[i], arr2[i])) {
            ordered_count++;
        }
        
        if (isgreater(arr1[i], arr2[i])) {
            greater_count++;
        }
        
        if (isless(arr1[i], arr2[i])) {
            less_count++;
        }
        
        /* Additional comparisons to trigger more condition codes */
        VOLATILE_DOUBLE x = arr1[i];
        VOLATILE_DOUBLE y = arr2[i];
        
        /* This should generate various condition codes */
        if (x != y) {
            /* May generate UNORDERED or NEQ */
        }
        
        if (x == y) {
            /* May generate ORDERED or EQ */
        }
    }
    
    return unordered_count + ordered_count + greater_count + less_count;
}

/* Test 4: Long double (x87) comparisons */
NOINLINE int test_long_double_comparisons(VOLATILE_LONG_DOUBLE a, VOLATILE_LONG_DOUBLE b) {
    int results = 0;
    
    /* x87 comparisons may generate different condition codes */
    if (a != b) results |= 1;
    if (a == b) results |= 2;
    if (a > b) results |= 4;
    if (a < b) results |= 8;
    if (a >= b) results |= 16;
    if (a <= b) results |= 32;
    
    /* Explicit unordered check for long double */
    if (isunordered((double)a, (double)b)) results |= 64;
    
    return results;
}

/* Test 5: Switch statement based on comparison results */
NOINLINE int test_switch_comparisons(VOLATILE_DOUBLE a, VOLATILE_DOUBLE b) {
    int result = 0;
    
    /* Complex switch to force multiple condition code generations */
    if (isunordered(a, b)) {
        result = 1;  /* UNORDERED */
    } else if (a == b) {
        result = 2;  /* EQ or ORDERED+EQ */
    } else if (a > b) {
        result = 3;  /* GT */
    } else if (a < b) {
        result = 4;  /* LT */
    } else if (a >= b) {
        result = 5;  /* GE */
    } else if (a <= b) {
        result = 6;  /* LE */
    }
    
    /* Additional comparisons to trigger UNEQ, UNGE, etc. */
    if (!isunordered(a, b) && a == b) {
        result += 10;  /* UNEQ? */
    }
    
    if (!isunordered(a, b) && a >= b) {
        result += 20;  /* UNGE? */
    }
    
    if (!isunordered(a, b) && a > b) {
        result += 30;  /* UNGT? */
    }
    
    if (!isunordered(a, b) && a <= b) {
        result += 40;  /* UNLE? */
    }
    
    if (!isunordered(a, b) && a < b) {
        result += 50;  /* UNLT? */
    }
    
    if (a != b && !isunordered(a, b)) {
        result += 60;  /* LTGT? */
    }
    
    return result;
}

/* Test 6: Mixed SSE and x87 operations */
NOINLINE int test_mixed_operations(VOLATILE_DOUBLE a, VOLATILE_DOUBLE b) {
    VOLATILE_LONG_DOUBLE la = (VOLATILE_LONG_DOUBLE)a;
    VOLATILE_LONG_DOUBLE lb = (VOLATILE_LONG_DOUBLE)b;
    
    int result = 0;
    
    /* SSE2 comparison */
    if (a != b) {
        result += 1;
    }
    
    /* x87 comparison */
    if (la != lb) {
        result += 2;
    }
    
    /* Mixed: convert long double to double and compare */
    VOLATILE_DOUBLE da = (VOLATILE_DOUBLE)la;
    VOLATILE_DOUBLE db = (VOLATILE_DOUBLE)lb;
    
    if (da != db) {
        result += 4;
    }
    
    /* Use builtin for direct unordered compare */
    int cmp = __builtin_ia32_ucomisd((double)a, (double)b);
    if (cmp & 0x45) {  /* Check parity flag (unordered) or ZF=0,PF=0,CF=0 (greater) */
        result += 8;
    }
    
    return result;
}

int main(void) {
    int total_checksum = 0;
    
    /* Create test values including NaN */
    VOLATILE_DOUBLE nan_val = NAN;
    VOLATILE_DOUBLE inf_val = INFINITY;
    VOLATILE_DOUBLE normal1 = 3.14159;
    VOLATILE_DOUBLE normal2 = 2.71828;
    VOLATILE_DOUBLE zero = 0.0;
    
    VOLATILE_LONG_DOUBLE lnan = (VOLATILE_LONG_DOUBLE)NAN;
    VOLATILE_LONG_DOUBLE lnormal1 = (VOLATILE_LONG_DOUBLE)3.14159;
    
    /* Test 1: Various comparisons */
    printf("Test 1: Unordered comparisons\n");
    total_checksum += test_unordered_comparisons(nan_val, normal1);
    total_checksum += test_unordered_comparisons(normal1, normal2);
    total_checksum += test_unordered_comparisons(nan_val, nan_val);
    total_checksum += test_unordered_comparisons(inf_val, normal1);
    
    /* Test 2: Inline assembly with condition codes */
    printf("Test 2: Inline assembly condition codes\n");
    total_checksum += test_asm_condition_codes(nan_val, normal1);
    total_checksum += test_asm_condition_codes(normal1, normal2);
    total_checksum += test_asm_condition_codes(normal1, normal1);
    
    /* Test 3: Array comparisons */
    printf("Test 3: Array comparisons\n");
    VOLATILE_DOUBLE arr1[10], arr2[10];
    for (int i = 0; i < 10; i++) {
        arr1[i] = (i % 3 == 0) ? nan_val : (VOLATILE_DOUBLE)i;
        arr2[i] = (i % 4 == 0) ? nan_val : (VOLATILE_DOUBLE)(i * 0.5);
    }
    total_checksum += test_array_comparisons(arr1, arr2, 10);
    
    /* Test 4: Long double comparisons */
    printf("Test 4: Long double comparisons\n");
    total_checksum += test_long_double_comparisons(lnan, lnormal1);
    total_checksum += test_long_double_comparisons(lnormal1, (VOLATILE_LONG_DOUBLE)2.71828);
    
    /* Test 5: Switch-based comparisons */
    printf("Test 5: Switch comparisons\n");
    total_checksum += test_switch_comparisons(nan_val, normal1);
    total_checksum += test_switch_comparisons(normal1, normal2);
    total_checksum += test_switch_comparisons(normal1, normal1);
    
    /* Test 6: Mixed operations */
    printf("Test 6: Mixed SSE/x87 operations\n");
    total_checksum += test_mixed_operations(nan_val, normal1);
    total_checksum += test_mixed_operations(normal1, normal2);
    
    printf("Total checksum: %d\n", total_checksum);
    
    return 0;
}
