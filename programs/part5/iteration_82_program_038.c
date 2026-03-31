/* x87_comparison_test.c
 * Designed to trigger x87 comparison condition code output in i386.cc
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -fno-fast-math -o x87_test x87_comparison_test.c
 */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ldbl_array[16];
volatile int array_index = 0;

/* Helper to get dynamic long double values */
long double get_ldbl(int idx) {
    return global_ldbl_array[idx % 16];
}

/* Complex comparison function designed to use multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    int result = 0;
    
    /* Ordered comparisons */
    if (a < b) result |= 1;      /* LT */
    if (a > c) result |= 2;      /* GT */
    if (b <= c) result |= 4;     /* LE */
    if (c >= d) result |= 8;     /* GE */
    if (a == b) result |= 16;    /* EQ */
    if (b != c) result |= 32;    /* NEQ */
    
    /* Unordered comparisons with potential NaN */
    volatile long double nan_val = __builtin_nanl("");
    if (!(a == a)) result |= 64;         /* Check if a is NaN (UNORDERED) */
    if (a != a) result |= 128;           /* Alternative NaN check */
    
    /* Compare with NaN explicitly */
    if (a == nan_val) result |= 256;     /* UNEQ? */
    if (a != nan_val) result |= 512;     /* LTGT? */
    if (!(a < nan_val)) result |= 1024;  /* UNGE: not less than NaN */
    if (!(a > nan_val)) result |= 2048;  /* UNLE: not greater than NaN */
    
    return result;
}

/* Function focusing on unordered comparisons */
int unordered_comparisons(long double x, long double y) {
    int result = 0;
    
    /* Generate NaN through arithmetic */
    volatile long double zero = 0.0L;
    volatile long double nan1 = zero / zero;          /* 0/0 = NaN */
    volatile long double nan2 = __builtin_nanl("");   /* Quiet NaN */
    volatile long double inf = __builtin_infl();
    
    /* Comparisons involving NaN (should trigger UNORDERED cases) */
    if (x < nan1) result |= 1;        /* UNORDERED comparison */
    if (x > nan2) result |= 2;        /* UNORDERED comparison */
    if (x <= nan1) result |= 4;       /* UNORDERED comparison */
    if (x >= nan2) result |= 8;       /* UNORDERED comparison */
    if (x == nan1) result |= 16;      /* UNEQ comparison */
    if (x != nan2) result |= 32;      /* LTGT comparison */
    
    /* Compare NaN with infinity */
    if (nan1 < inf) result |= 64;     /* UNORDERED */
    if (nan2 > -inf) result |= 128;   /* UNORDERED */
    
    /* Compare two NaNs */
    if (nan1 == nan2) result |= 256;  /* UNORDERED/UNEQ */
    if (nan1 != nan2) result |= 512;  /* UNORDERED/LTGT */
    
    return result;
}

/* Function with mixed precision comparisons */
int mixed_precision_comparisons(float f, double d, long double ld) {
    int result = 0;
    
    /* Mixed precision with explicit casts */
    if ((long double)f < ld) result |= 1;
    if (d > (long double)f) result |= 2;
    if ((long double)ld <= d) result |= 4;
    if (ld >= (long double)42) result |= 8;  /* Integer constant cast */
    
    /* Complex expression */
    if ((f != d) && (ld <= (long double)100.0)) result |= 16;
    
    return result;
}

/* Loop with long double termination condition */
int loop_with_ldbl_condition(long double start, long double limit) {
    volatile long double counter = start;
    int iterations = 0;
    
    /* Loop condition using long double comparison */
    while (counter < limit && iterations < 100) {
        /* Prevent optimization of loop body */
        global_ldbl_array[iterations % 16] = counter;
        counter += 1.0L;
        iterations++;
        
        /* Additional comparison in loop body */
        if (counter != limit) {
            global_ldbl_array[(iterations + 1) % 16] = counter * 2.0L;
        }
    }
    
    return iterations;
}

/* Switch statement based on comparison results */
int switch_on_comparison(long double a, long double b, long double c) {
    int result = 0;
    
    /* Build a value based on multiple comparisons */
    int case_val = 0;
    if (a < b) case_val += 1;
    if (b > c) case_val += 2;
    if (a == c) case_val += 4;
    
    /* Switch on the combined result */
    switch (case_val) {
        case 0:
            result = complex_x87_comparison(a, b, c, __builtin_nanl(""));
            break;
        case 1:
            result = unordered_comparisons(a, b);
            break;
        case 2:
            result = mixed_precision_comparisons((float)a, (double)b, c);
            break;
        case 3:
            result = loop_with_ldbl_condition(a, b);
            break;
        case 4:
            result = complex_x87_comparison(b, c, a, __builtin_infl());
            break;
        case 5:
            result = unordered_comparisons(__builtin_nanl(""), c);
            break;
        case 6:
            result = mixed_precision_comparisons((float)b, (double)c, a);
            break;
        case 7:
            result = loop_with_ldbl_condition(0.0L, c);
            break;
        default:
            result = -1;
    }
    
    return result;
}

/* Main test function */
int main() {
    /* Initialize array with various long double values */
    for (int i = 0; i < 16; i++) {
        switch (i % 8) {
            case 0: global_ldbl_array[i] = 0.0L; break;
            case 1: global_ldbl_array[i] = 1.0L; break;
            case 2: global_ldbl_array[i] = -1.0L; break;
            case 3: global_ldbl_array[i] = 3.14159265358979323846L; break;
            case 4: global_ldbl_array[i] = __builtin_infl(); break;
            case 5: global_ldbl_array[i] = -__builtin_infl(); break;
            case 6: global_ldbl_array[i] = __builtin_nanl(""); break;
            case 7: global_ldbl_array[i] = 0.0L / 0.0L; break; /* Another NaN */
        }
    }
    
    int results[32];
    int result_count = 0;
    
    /* Test 1: Basic ordered comparisons */
    results[result_count++] = complex_x87_comparison(
        get_ldbl(0), get_ldbl(1), get_ldbl(2), get_ldbl(3));
    
    /* Test 2: Unordered comparisons with NaN */
    results[result_count++] = unordered_comparisons(
        get_ldbl(6), get_ldbl(4));
    
    /* Test 3: Mixed precision */
    results[result_count++] = mixed_precision_comparisons(
        3.14f, 2.718281828459045, get_ldbl(3));
    
    /* Test 4: Loop with long double condition */
    results[result_count++] = loop_with_ldbl_condition(
        get_ldbl(0), get_ldbl(5));
    
    /* Test 5: Switch based on comparisons */
    results[result_count++] = switch_on_comparison(
        get_ldbl(1), get_ldbl(2), get_ldbl(3));
    
    /* Test 6: More complex scenarios */
    for (int i = 0; i < 8 && result_count < 32; i++) {
        results[result_count++] = complex_x87_comparison(
            get_ldbl(i), 
            get_ldbl((i + 1) % 8), 
            get_ldbl((i + 2) % 8), 
            get_ldbl((i + 3) % 8));
    }
    
    /* Test 7: Direct NaN comparisons */
    volatile long double nan_val = __builtin_nanl("");
    volatile long double inf_val = __builtin_infl();
    for (int i = 0; i < 4 && result_count < 32; i++) {
        results[result_count++] = (nan_val < get_ldbl(i)) ? 1 : 0;
        results[result_count++] = (get_ldbl(i) > nan_val) ? 1 : 0;
        results[result_count++] = (nan_val <= inf_val) ? 1 : 0;
        results[result_count++] = (inf_val >= nan_val) ? 1 : 0;
        results[result_count++] = (nan_val == nan_val) ? 1 : 0;
        results[result_count++] = (nan_val != nan_val) ? 1 : 0;
    }
    
    /* Compute verification hash (XOR of all results) */
    int verification_hash = 0;
    for (int i = 0; i < result_count; i++) {
        verification_hash ^= results[i];
    }
    
    /* Print hash to prevent dead code elimination */
    printf("Verification hash: %d\n", verification_hash);
    printf("Number of tests executed: %d\n", result_count);
    
    return 0;
}
