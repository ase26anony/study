/* x87_comparison_test.c
 * Designed to trigger x87 comparison mnemonics in i386.cc lines 13992-14017
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -fno-fast-math x87_comparison_test.c -lm
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ld_array[16];
volatile int array_index = 0;

/* Helper to get dynamic long double values */
long double get_ld_value(int idx) {
    return global_ld_array[idx % 16];
}

/* Complex multi-operand comparison designed to use multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    /* This should generate various x87 comparison patterns */
    if (a != b) {
        if (c <= d) {
            if (a > c) {
                return 1;
            } else if (b < d) {
                return 2;
            }
        }
    }
    
    if (a == b && c >= d) {
        return 3;
    }
    
    /* Unordered comparisons with potential NaN */
    if (!(a == a) || !(b == b)) {  /* Check for NaN */
        return 4;
    }
    
    return 0;
}

/* Test ordered comparisons */
int test_ordered_comparisons(long double x, long double y) {
    int result = 0;
    
    /* All standard ordered comparisons */
    if (x < y)  result |= 1;
    if (x > y)  result |= 2;
    if (x <= y) result |= 4;
    if (x >= y) result |= 8;
    if (x == y) result |= 16;
    if (x != y) result |= 32;
    
    return result;
}

/* Test unordered comparisons with NaN */
int test_unordered_comparisons(long double x, long double y) {
    int result = 0;
    
    /* Comparisons that may involve NaN */
    volatile long double nan_val = __builtin_nanl("");
    volatile long double inf_val = __builtin_infl();
    
    /* Compare with NaN (unordered cases) */
    if (x == nan_val) result |= 1;      /* Should be false, may generate UNEQ */
    if (x != nan_val) result |= 2;      /* Should be true for non-NaN, may generate UNORDERED */
    if (x < nan_val)  result |= 4;      /* Should be false, may generate UNLT */
    if (x > nan_val)  result |= 8;      /* Should be false, may generate UNGT */
    if (x <= nan_val) result |= 16;     /* Should be false, may generate UNLE */
    if (x >= nan_val) result |= 32;     /* Should be false, may generate UNGE */
    
    /* Compare NaN with NaN */
    if (nan_val == nan_val) result |= 64;   /* Should be false */
    if (nan_val != nan_val) result |= 128;  /* Should be true (UNORDERED case) */
    
    /* Compare with infinity */
    if (x == inf_val) result |= 256;
    if (x < inf_val)  result |= 512;
    if (x > -inf_val) result |= 1024;
    
    return result;
}

/* Generate NaN through arithmetic */
long double generate_nan(int method) {
    volatile long double result;
    
    switch (method) {
        case 0:
            result = 0.0L / 0.0L;          /* Division by zero */
            break;
        case 1:
            result = __builtin_nanl("");   /* Built-in NaN */
            break;
        case 2:
            result = sqrtl(-1.0L);         /* sqrt of negative */
            break;
        case 3:
            result = __builtin_infl() * 0.0L; /* Infinity * 0 */
            break;
        default:
            result = __builtin_nanl("");
    }
    
    return result;
}

/* Loop with long double termination condition */
int loop_with_ld_condition(long double start, long double limit) {
    volatile long double counter = start;
    int iterations = 0;
    
    /* Loop condition based on long double comparison */
    while (counter < limit && iterations < 100) {
        counter += 0.5L;
        iterations++;
        
        /* Nested comparison inside loop */
        if (counter != start && counter <= limit) {
            /* Do something */
            iterations += (int)(counter - start);
        }
    }
    
    return iterations;
}

/* Mixed precision comparisons */
int test_mixed_precision(float f, double d, long double ld) {
    int result = 0;
    
    /* Compare different floating types (promotions to long double) */
    if ((long double)f < ld) result |= 1;
    if ((long double)d > ld) result |= 2;
    if (f == (float)ld) result |= 4;
    if (d != (double)ld) result |= 8;
    
    /* Compare with integer constant cast to long double */
    if (ld < (long double)100) result |= 16;
    if (ld > (long double)-50) result |= 32;
    
    return result;
}

/* Switch statement based on comparison results */
int switch_on_comparison(long double a, long double b, long double c) {
    int result = 0;
    
    /* Complex condition that may use various x87 codes */
    if (a < b) {
        if (b > c) {
            result = 1;
        } else if (b == c) {
            result = 2;
        } else {
            result = 3;
        }
    } else if (a > b) {
        if (b < c) {
            result = 4;
        } else if (b != c) {
            result = 5;
        } else {
            result = 6;
        }
    } else { /* a == b */
        if (c != c) { /* c is NaN */
            result = 7;
        } else if (c <= a) {
            result = 8;
        } else {
            result = 9;
        }
    }
    
    return result;
}

/* Main test function */
int main() {
    /* Initialize global array with mixed values */
    for (int i = 0; i < 16; i++) {
        switch (i % 8) {
            case 0: global_ld_array[i] = 0.0L; break;
            case 1: global_ld_array[i] = 1.0L; break;
            case 2: global_ld_array[i] = -1.0L; break;
            case 3: global_ld_array[i] = 3.14159265358979323846L; break;
            case 4: global_ld_array[i] = __builtin_infl(); break;
            case 5: global_ld_array[i] = -__builtin_infl(); break;
            case 6: global_ld_array[i] = generate_nan(i % 4); break;
            case 7: global_ld_array[i] = 100.0L / (i + 1); break;
        }
    }
    
    int results[32];
    int result_count = 0;
    
    /* Test 1: Ordered comparisons */
    for (int i = 0; i < 8; i++) {
        long double a = get_ld_value(i);
        long double b = get_ld_value(i + 1);
        results[result_count++] = test_ordered_comparisons(a, b);
    }
    
    /* Test 2: Unordered comparisons with NaN */
    volatile long double nan_val = generate_nan(0);
    for (int i = 0; i < 8; i++) {
        long double a = get_ld_value(i);
        results[result_count++] = test_unordered_comparisons(a, nan_val);
    }
    
    /* Test 3: Complex multi-operand comparison */
    for (int i = 0; i < 4; i++) {
        long double a = get_ld_value(i * 2);
        long double b = get_ld_value(i * 2 + 1);
        long double c = get_ld_value(i * 2 + 2);
        long double d = get_ld_value(i * 2 + 3);
        results[result_count++] = complex_x87_comparison(a, b, c, d);
    }
    
    /* Test 4: Loop with long double condition */
    results[result_count++] = loop_with_ld_condition(0.0L, 10.0L);
    results[result_count++] = loop_with_ld_condition(-5.0L, get_ld_value(3));
    
    /* Test 5: Mixed precision */
    float f = 3.14f;
    double d = 2.718281828459045;
    for (int i = 0; i < 4; i++) {
        long double ld = get_ld_value(i);
        results[result_count++] = test_mixed_precision(f, d, ld);
        f += 1.0f;
        d *= 1.1;
    }
    
    /* Test 6: Switch on comparison */
    for (int i = 0; i < 4; i++) {
        long double a = get_ld_value(i);
        long double b = get_ld_value(i + 4);
        long double c = get_ld_value(i + 8);
        results[result_count++] = switch_on_comparison(a, b, c);
    }
    
    /* Compute verification hash (XOR of all results) */
    int verification_hash = 0;
    for (int i = 0; i < result_count; i++) {
        verification_hash ^= results[i];
    }
    
    /* Print verification hash to prevent dead code elimination */
    printf("Verification hash: %d\n", verification_hash);
    printf("Total tests executed: %d\n", result_count);
    
    return verification_hash != 0 ? 0 : 1;
}
