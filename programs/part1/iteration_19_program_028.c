/* test_condition_codes.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <stdint.h>

/* Prevent optimizations from removing our test code */
#define NOINLINE __attribute__((noinline, noipa))
#define VOLATILE_DOUBLE volatile double
#define VOLATILE_LONG_DOUBLE volatile long double

/* Test 1: Direct unordered comparisons with NaN */
NOINLINE int test_unordered_comparisons(void) {
    VOLATILE_DOUBLE nan_val = NAN;
    VOLATILE_DOUBLE inf_val = INFINITY;
    VOLATILE_DOUBLE normal1 = 1.5;
    VOLATILE_DOUBLE normal2 = 2.5;
    VOLATILE_DOUBLE zero = 0.0;
    
    int results[8] = {0};
    
    /* These should generate UNORDERED condition codes */
    results[0] = (nan_val != normal1) ? 1 : 0;      /* unord? */
    results[1] = (normal1 != nan_val) ? 1 : 0;      /* unord? */
    results[2] = (nan_val == nan_val) ? 1 : 0;      /* unord? (false for NaN) */
    
    /* Ordered comparisons */
    results[3] = (normal1 == normal2) ? 1 : 0;      /* ord? */
    results[4] = (normal1 != normal2) ? 1 : 0;      /* ord? */
    
    /* Mixed comparisons that might trigger various codes */
    results[5] = (normal1 < inf_val) ? 1 : 0;
    results[6] = (inf_val > normal1) ? 1 : 0;
    results[7] = (-inf_val < normal1) ? 1 : 0;
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 2: Using math.h comparison macros */
NOINLINE int test_math_macros(void) {
    VOLATILE_DOUBLE nan_val = NAN;
    VOLATILE_DOUBLE inf_val = INFINITY;
    VOLATILE_DOUBLE normal1 = 3.14;
    VOLATILE_DOUBLE normal2 = 2.71;
    
    int results[12] = {0};
    
    /* These map directly to various condition codes */
    results[0] = isunordered(nan_val, normal1);     /* UNORDERED */
    results[1] = !isunordered(normal1, normal2);    /* ORDERED */
    results[2] = isgreater(normal1, normal2);       /* UNLE? (inverted) */
    results[3] = isgreaterequal(normal1, normal2);  /* UNLT? (inverted) */
    results[4] = isless(normal2, normal1);          /* UNGE? (inverted) */
    results[5] = islessequal(normal2, normal1);     /* UNGT? (inverted) */
    
    /* More complex cases */
    results[6] = (!isgreater(nan_val, normal1) && !isless(nan_val, normal1)) ? 1 : 0;
    results[7] = (isunordered(nan_val, nan_val)) ? 1 : 0;
    results[8] = (!isunordered(inf_val, -inf_val)) ? 1 : 0;
    results[9] = (isless(inf_val, nan_val)) ? 0 : 1;  /* Should be false */
    results[10] = (isgreater(nan_val, inf_val)) ? 0 : 1;
    results[11] = (!isunordered(normal1, normal2) && normal1 != normal2) ? 1 : 0;
    
    int sum = 0;
    for (int i = 0; i < 12; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 3: Inline assembly with %C modifier */
NOINLINE int test_inline_asm(void) {
    VOLATILE_DOUBLE a = 1.0;
    VOLATILE_DOUBLE b = 2.0;
    VOLATILE_DOUBLE c = NAN;
    VOLATILE_DOUBLE d = INFINITY;
    
    uint8_t results[16] = {0};
    int idx = 0;
    
    /* Test various condition codes via inline assembly */
    #define TEST_COND(cond, x, y) do { \
        uint8_t result; \
        __asm__ volatile ( \
            "ucomisd %2, %1\n\t" \
            "set%C0 %0" \
            : "=r"(result) \
            : "x"(x), "x"(y) \
            : "cc" \
        ); \
        results[idx++] = result; \
    } while(0)
    
    /* These should generate various condition codes */
    TEST_COND(e, a, a);      /* eq/ueq */
    TEST_COND(ne, a, b);     /* ne/une */
    TEST_COND(lt, a, b);     /* lt/nge */
    TEST_COND(le, a, b);     /* le/ngt */
    TEST_COND(gt, b, a);     /* gt/nle */
    TEST_COND(ge, b, a);     /* ge/nlt */
    
    /* Unordered comparisons */
    TEST_COND(p, c, a);      /* unordered (parity) */
    TEST_COND(np, a, b);     /* ordered (no parity) */
    
    /* More unordered variants */
    TEST_COND(u, c, a);      /* unordered */
    TEST_COND(nu, a, b);     /* ordered */
    
    /* Test with NaN */
    TEST_COND(e, c, c);      /* unordered equal? */
    TEST_COND(ne, c, a);     /* unordered not equal? */
    TEST_COND(lt, c, a);     /* unordered less? */
    TEST_COND(le, c, a);     /* unordered less equal? */
    TEST_COND(gt, a, c);     /* unordered greater? */
    TEST_COND(ge, a, c);     /* unordered greater equal? */
    
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 4: Long double (x87) operations */
NOINLINE int test_long_double(void) {
    VOLATILE_LONG_DOUBLE ld_nan = NAN;
    VOLATILE_LONG_DOUBLE ld_inf = INFINITY;
    VOLATILE_LONG_DOUBLE ld1 = 3.14159265358979323846L;
    VOLATILE_LONG_DOUBLE ld2 = 2.71828182845904523536L;
    
    int results[10] = {0};
    
    /* x87 comparisons often generate different condition codes */
    results[0] = (ld1 != ld2) ? 1 : 0;
    results[1] = (ld1 == ld2) ? 0 : 1;
    results[2] = (ld1 < ld2) ? 0 : 1;
    results[3] = (ld1 > ld2) ? 1 : 0;
    results[4] = (ld1 <= ld2) ? 0 : 1;
    results[5] = (ld1 >= ld2) ? 1 : 0;
    
    /* NaN comparisons */
    results[6] = (ld_nan != ld1) ? 1 : 0;
    results[7] = (ld_nan == ld_nan) ? 0 : 1;  /* NaN != NaN */
    results[8] = (ld_inf > ld1) ? 1 : 0;
    results[9] = (-ld_inf < ld1) ? 1 : 0;
    
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 5: Array-based comparisons to force loops */
NOINLINE int test_array_comparisons(void) {
    #define ARRAY_SIZE 64
    VOLATILE_DOUBLE arr1[ARRAY_SIZE];
    VOLATILE_DOUBLE arr2[ARRAY_SIZE];
    
    /* Initialize with mix of values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (i % 8 == 0) {
            arr1[i] = NAN;
            arr2[i] = i * 0.1;
        } else if (i % 8 == 1) {
            arr1[i] = i * 0.2;
            arr2[i] = NAN;
        } else if (i % 8 == 2) {
            arr1[i] = INFINITY;
            arr2[i] = i * 0.3;
        } else if (i % 8 == 3) {
            arr1[i] = -INFINITY;
            arr2[i] = i * 0.4;
        } else {
            arr1[i] = i * 0.5;
            arr2[i] = (i + 1) * 0.5;
        }
    }
    
    int unordered_count = 0;
    int ordered_count = 0;
    int greater_count = 0;
    int less_count = 0;
    int equal_count = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        unordered_count += isunordered(arr1[i], arr2[i]);
        ordered_count += !isunordered(arr1[i], arr2[i]);
        greater_count += isgreater(arr1[i], arr2[i]);
        less_count += isless(arr1[i], arr2[i]);
        
        /* Check for equality (but not unordered) */
        if (!isunordered(arr1[i], arr2[i]) && arr1[i] == arr2[i]) {
            equal_count++;
        }
    }
    
    return unordered_count + ordered_count + greater_count + less_count + equal_count;
}

/* Test 6: Switch based on comparison results */
NOINLINE int test_switch_comparisons(void) {
    VOLATILE_DOUBLE vals[] = {NAN, INFINITY, -INFINITY, 0.0, 1.0, -1.0, 2.0, -2.0};
    int results[8] = {0};
    
    for (int i = 0; i < 8; i++) {
        VOLATILE_DOUBLE a = vals[i];
        VOLATILE_DOUBLE b = vals[(i + 1) % 8];
        
        /* Complex comparison that might generate various condition codes */
        if (isunordered(a, b)) {
            results[i] = 1;  /* UNORDERED */
        } else if (a == b) {
            results[i] = 2;  /* EQ/UEQ */
        } else if (a < b) {
            results[i] = 3;  /* LT/UNLT? */
        } else if (a > b) {
            results[i] = 4;  /* GT/UNGT? */
        } else if (a <= b) {
            results[i] = 5;  /* LE/UNLE? */
        } else if (a >= b) {
            results[i] = 6;  /* GE/UNGE? */
        } else {
            results[i] = 7;  /* LTGT/UNE? */
        }
    }
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 7: Direct builtin usage */
NOINLINE int test_builtins(void) {
    double a = 1.5;
    double b = 2.5;
    double c = NAN;
    
    int results[6] = {0};
    
    /* Use GCC x86 builtins */
    results[0] = __builtin_ia32_ucomisd(a, b);
    results[1] = __builtin_ia32_ucomisd(b, a);
    results[2] = __builtin_ia32_ucomisd(a, a);
    results[3] = __builtin_ia32_ucomisd(c, a);
    results[4] = __builtin_ia32_ucomisd(a, c);
    results[5] = __builtin_ia32_ucomisd(c, c);
    
    /* Convert flags to integer results */
    int sum = 0;
    for (int i = 0; i < 6; i++) {
        /* The builtin returns flags in EFLAGS, we extract specific bits */
        int zf = (results[i] & 0x40) ? 1 : 0;
        int pf = (results[i] & 0x04) ? 1 : 0;
        int cf = (results[i] & 0x01) ? 1 : 0;
        sum += zf + pf + cf;
    }
    return sum;
}

int main(void) {
    printf("Testing x86 condition code generation...\n");
    
    int total = 0;
    
    total += test_unordered_comparisons();
    total += test_math_macros();
    total += test_inline_asm();
    total += test_long_double();
    total += test_array_comparisons();
    total += test_switch_comparisons();
    total += test_builtins();
    
    printf("Total checksum: %d\n", total);
    
    /* Also test some direct prints to see condition codes */
    volatile double x = NAN;
    volatile double y = 1.0;
    
    /* Force generation of condition code names */
    if (x != y) {
        printf("x != y (unordered case)\n");
    }
    
    if (!(x == y)) {
        printf("!(x == y) (ordered case)\n");
    }
    
    if (isunordered(x, y)) {
        printf("isunordered(x, y)\n");
    }
    
    if (!isunordered(y, y)) {
        printf("!isunordered(y, y)\n");
    }
    
    return 0;
}
