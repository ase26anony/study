/* Test program to trigger x86 floating-point condition code output */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Prevent optimizations from removing critical code */
#define NOINLINE __attribute__((noinline, noipa))
#define VOLATILE_DOUBLE volatile double
#define VOLATILE_LONG_DOUBLE volatile long double

/* Function 1: Direct unordered comparisons with NaN */
NOINLINE int test_unordered_comparisons(void) {
    VOLATILE_DOUBLE nan_val = NAN;
    VOLATILE_DOUBLE normal_val = 3.14159;
    VOLATILE_DOUBLE zero_val = 0.0;
    VOLATILE_DOUBLE inf_val = INFINITY;
    
    int results[8] = {0};
    
    /* These should generate UNORDERED/ORDERED condition codes */
    results[0] = (nan_val != normal_val) ? 1 : 0;      /* UNORDERED */
    results[1] = (nan_val == nan_val) ? 1 : 0;         /* ORDERED (false for NaN) */
    results[2] = (normal_val == normal_val) ? 1 : 0;   /* ORDERED */
    
    /* More complex comparisons */
    results[3] = (nan_val < normal_val) ? 1 : 0;       /* UNORDERED */
    results[4] = (normal_val > nan_val) ? 1 : 0;       /* UNORDERED */
    results[5] = (normal_val != nan_val) ? 1 : 0;      /* ORDERED */
    
    /* Comparisons involving infinity */
    results[6] = (inf_val > normal_val) ? 1 : 0;       /* ORDERED */
    results[7] = (inf_val < nan_val) ? 1 : 0;          /* UNORDERED */
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Function 2: Using <math.h> comparison macros */
NOINLINE int test_math_macros(void) {
    VOLATILE_DOUBLE nan1 = NAN;
    VOLATILE_DOUBLE nan2 = strtod("NAN", NULL);
    VOLATILE_DOUBLE val1 = 1.5;
    VOLATILE_DOUBLE val2 = 2.5;
    
    int results[12] = {0};
    
    /* These map directly to various condition codes */
    results[0] = isunordered(nan1, val1);      /* UNORDERED */
    results[1] = !isunordered(val1, val2);     /* ORDERED */
    results[2] = isgreater(val1, val2);        /* UNLE? Actually generates GT/LE */
    results[3] = isless(val1, val2);           /* UNGE? Actually generates LT/GE */
    results[4] = isgreaterequal(val1, val2);   /* UNLT */
    results[5] = islessequal(val1, val2);      /* UNGT */
    
    /* Special cases for UNEQ and LTGT */
    results[6] = (val1 == val1) && !isunordered(val1, val1); /* ORDERED EQ */
    results[7] = (val1 != val2) && !isunordered(val1, val2); /* ORDERED NEQ (LTGT) */
    
    /* Mixed NaN comparisons */
    results[8] = isunordered(nan1, nan2);      /* UNORDERED */
    results[9] = isgreater(nan1, val1);        /* UNORDERED */
    results[10] = islessequal(nan1, val1);     /* UNORDERED */
    results[11] = !isunordered(val1, val1);    /* ORDERED */
    
    int sum = 0;
    for (int i = 0; i < 12; i++) {
        sum += results[i];
    }
    return sum;
}

/* Function 3: Inline assembly with %C modifier */
NOINLINE int test_inline_asm(void) {
    VOLATILE_DOUBLE a = 1.0;
    VOLATILE_DOUBLE b = 2.0;
    VOLATILE_DOUBLE c = NAN;
    VOLATILE_DOUBLE d = NAN;
    
    int results[6] = {0};
    uint8_t byte_result;
    
    /* Test 1: Ordered comparison */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(byte_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    results[0] = byte_result;
    
    /* Test 2: Unordered comparison with NaN */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(byte_result)
        : "x"(a), "x"(c)
        : "cc"
    );
    results[1] = byte_result;
    
    /* Test 3: NaN vs NaN */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(byte_result)
        : "x"(c), "x"(d)
        : "cc"
    );
    results[2] = byte_result;
    
    /* Test 4: Different condition codes */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(byte_result)
        : "x"(b), "x"(a)  /* b > a */
        : "cc"
    );
    results[3] = byte_result;
    
    /* Test 5: Using fucomip for x87 style */
    double x = 3.0;
    double y = 3.0;
    __asm__ volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "set%C0 %0"
        : "=r"(byte_result)
        : "m"(x), "m"(y)
        : "cc", "st"
    );
    results[4] = byte_result;
    
    /* Test 6: Unordered with fucomip */
    double nan = NAN;
    __asm__ volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "set%C0 %0"
        : "=r"(byte_result)
        : "m"(x), "m"(nan)
        : "cc", "st"
    );
    results[5] = byte_result;
    
    int sum = 0;
    for (int i = 0; i < 6; i++) {
        sum += results[i];
    }
    return sum;
}

/* Function 4: Array-based unordered comparisons */
NOINLINE int test_array_comparisons(void) {
    const int SIZE = 32;
    VOLATILE_DOUBLE arr1[SIZE];
    VOLATILE_DOUBLE arr2[SIZE];
    
    /* Initialize with mix of normal values and NaN */
    for (int i = 0; i < SIZE; i++) {
        if (i % 5 == 0) {
            arr1[i] = NAN;
            arr2[i] = (double)i;
        } else if (i % 5 == 1) {
            arr1[i] = (double)i;
            arr2[i] = NAN;
        } else if (i % 5 == 2) {
            arr1[i] = NAN;
            arr2[i] = NAN;
        } else {
            arr1[i] = (double)i;
            arr2[i] = (double)(i * 2);
        }
    }
    
    int unordered_count = 0;
    int ordered_count = 0;
    int greater_count = 0;
    int less_count = 0;
    int equal_count = 0;
    
    for (int i = 0; i < SIZE; i++) {
        /* Count various comparison results */
        unordered_count += isunordered(arr1[i], arr2[i]);
        ordered_count += !isunordered(arr1[i], arr2[i]);
        greater_count += isgreater(arr1[i], arr2[i]);
        less_count += isless(arr1[i], arr2[i]);
        
        /* For equality, need to handle NaN carefully */
        if (!isunordered(arr1[i], arr2[i]) && arr1[i] == arr2[i]) {
            equal_count++;
        }
    }
    
    return unordered_count + ordered_count + greater_count + less_count + equal_count;
}

/* Function 5: Long double (x87) specific tests */
NOINLINE int test_long_double(void) {
    VOLATILE_LONG_DOUBLE ld_nan = NAN;
    VOLATILE_LONG_DOUBLE ld_val1 = 3.14159265358979323846L;
    VOLATILE_LONG_DOUBLE ld_val2 = 2.71828182845904523536L;
    VOLATILE_LONG_DOUBLE ld_inf = INFINITY;
    
    int results[10] = {0};
    
    /* x87 style comparisons - these often generate different condition codes */
    results[0] = (ld_nan != ld_val1) ? 1 : 0;
    results[1] = (ld_val1 == ld_val1) ? 1 : 0;
    results[2] = (ld_val1 > ld_val2) ? 1 : 0;
    results[3] = (ld_val1 < ld_val2) ? 1 : 0;
    results[4] = (ld_val1 >= ld_val2) ? 1 : 0;
    results[5] = (ld_val1 <= ld_val2) ? 1 : 0;
    results[6] = (ld_inf > ld_val1) ? 1 : 0;
    results[7] = (ld_nan == ld_nan) ? 1 : 0;
    results[8] = (ld_nan < ld_inf) ? 1 : 0;
    results[9] = (ld_inf == ld_inf) ? 1 : 0;
    
    /* Force x87 instructions with volatile asm */
    long double x = 1.0L;
    long double y = 2.0L;
    int cmp_result;
    
    __asm__ volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "seta %0"
        : "=r"(cmp_result)
        : "m"(x), "m"(y)
        : "cc", "st"
    );
    
    int sum = cmp_result;
    for (int i = 0; i < 10; i++) {
        sum += results[i];
    }
    return sum;
}

/* Function 6: Switch statement based on comparison results */
NOINLINE int test_switch_comparisons(void) {
    VOLATILE_DOUBLE vals[] = {1.0, NAN, 2.0, INFINITY, -INFINITY, 0.0};
    int total = 0;
    
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            int result = 0;
            
            /* Complex branching to force different condition codes */
            if (isunordered(vals[i], vals[j])) {
                result = 1;  /* UNORDERED */
            } else if (vals[i] == vals[j]) {
                result = 2;  /* ORDERED EQ / UNEQ? */
            } else if (vals[i] > vals[j]) {
                result = 3;  /* ORDERED GT / UNLE? */
            } else if (vals[i] < vals[j]) {
                result = 4;  /* ORDERED LT / UNGE? */
            }
            
            /* Additional check for special cases */
            if (!isunordered(vals[i], vals[j]) && vals[i] != vals[j]) {
                result += 10;  /* ORDERED NEQ / LTGT? */
            }
            
            total += result;
        }
    }
    
    return total;
}

/* Main function that runs all tests */
int main(void) {
    printf("Starting floating-point condition code tests...\n");
    
    int total = 0;
    
    total += test_unordered_comparisons();
    total += test_math_macros();
    total += test_inline_asm();
    total += test_array_comparisons();
    total += test_long_double();
    total += test_switch_comparisons();
    
    printf("Total checksum: %d\n", total);
    printf("If non-zero, tests executed successfully.\n");
    
    return 0;
}
