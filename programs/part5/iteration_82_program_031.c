/* x87_comparison_test.c
 * Designed to trigger x87 floating-point comparison output logic
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -o x87_test x87_comparison_test.c
 */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ld_array[16];
volatile int global_index = 0;

/* Helper to get dynamic long double values */
long double get_ld_value(int idx) {
    return global_ld_array[idx % 16];
}

/* Complex multi-operand comparison function targeting specific x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    int result = 0;
    
    /* Ordered comparisons (should generate "ord" type codes) */
    if (a < b && !(a != a) && !(b != b)) {
        result |= 1;  /* LT with ordered operands */
    }
    
    if (c > d && c == c && d == d) {
        result |= 2;  /* GT with ordered operands */
    }
    
    /* Unordered comparisons with NaN */
    long double nan_val = __builtin_nanl("");
    if (a != nan_val) {  /* UNORDERED/UNEQ when a is NaN */
        result |= 4;
    }
    
    if (!(b == b)) {  /* b is NaN - UNORDERED check */
        result |= 8;
    }
    
    /* Mixed comparisons: NaN vs normal numbers */
    if (nan_val >= c) {  /* UNGE: "nlt" - not less than (true when unordered) */
        result |= 16;
    }
    
    if (nan_val > d) {   /* UNGT: "nle" - not less or equal */
        result |= 32;
    }
    
    if (nan_val <= a) {  /* UNLE: "ule" - unordered or less or equal */
        result |= 64;
    }
    
    if (nan_val < b) {   /* UNLT: "ult" - unordered or less than */
        result |= 128;
    }
    
    /* LTGT: "une" - not equal and ordered */
    if (c != d && c == c && d == d) {
        result |= 256;
    }
    
    /* UNEQ: unordered or equal */
    if (!(a == b) || a != a || b != b) {
        /* This is actually the inverse, but helps generate the code */
        result |= 512;
    }
    
    return result;
}

/* Function performing all standard comparisons */
int perform_all_comparisons(long double x, long double y) {
    int results = 0;
    
    /* Basic ordered comparisons */
    results |= (x < y)   ? 0x0001 : 0;
    results |= (x > y)   ? 0x0002 : 0;
    results |= (x <= y)  ? 0x0004 : 0;
    results |= (x >= y)  ? 0x0008 : 0;
    results |= (x == y)  ? 0x0010 : 0;
    results |= (x != y)  ? 0x0020 : 0;
    
    /* Explicit NaN checks */
    results |= (x != x)  ? 0x0040 : 0;  /* is NaN */
    results |= (y != y)  ? 0x0080 : 0;  /* is NaN */
    
    /* Mixed type comparisons */
    double d = (double)x;
    float f = (float)y;
    results |= (x > d)   ? 0x0100 : 0;
    results |= (f < y)   ? 0x0200 : 0;
    
    return results;
}

/* Loop with long double termination condition */
int loop_with_ld_condition(long double start, long double limit) {
    volatile long double counter = start;
    int iterations = 0;
    
    /* Use multiple conditions to force different comparison types */
    while (counter < limit && counter == counter && limit == limit) {
        counter += 1.0L;
        iterations++;
        
        /* Nested if with complex condition */
        if (counter != limit || __builtin_nanl("") > start) {
            iterations ^= 0x55;
        }
    }
    
    /* Additional unordered check */
    if (counter != counter) {  /* NaN check */
        iterations |= 0x80000000;
    }
    
    return iterations;
}

/* Switch statement based on comparison results */
int switch_on_comparison(long double a, long double b, long double c) {
    int result = 0;
    
    /* First level of comparisons */
    if (a < b) {
        result = 1;
        /* Nested comparison */
        if (b > c && c == c) {  /* Ordered GT */
            result = 2;
        }
    } else if (a > b) {
        result = 3;
        if (b != b) {  /* b is NaN - unordered */
            result = 4;
        }
    } else if (a == b) {
        result = 5;
        /* Check if either is NaN (unordered equal) */
        if (a != a || b != b) {
            result = 6;
        }
    } else {
        /* a and b are unordered (at least one NaN) */
        result = 7;
        
        /* Further unordered comparisons */
        if (__builtin_nanl("") >= c) {  /* UNGE */
            result = 8;
        }
        if (__builtin_nanl("") <= a) {  /* UNLE */
            result = 9;
        }
    }
    
    return result;
}

/* Generate NaN values through various methods */
void generate_nan_scenarios(int *results) {
    long double nan1 = __builtin_nanl("");
    long double nan2 = 0.0L / 0.0L;
    long double nan3 = __builtin_nanl("0x1234");
    long double inf = __builtin_infl();
    long double neg_inf = -__builtin_infl();
    
    /* Compare NaN with everything */
    results[0] = (nan1 == 1.0L) ? 1 : 0;
    results[1] = (nan1 != 1.0L) ? 1 : 0;
    results[2] = (nan1 < 1.0L) ? 1 : 0;
    results[3] = (nan1 > 1.0L) ? 1 : 0;
    results[4] = (nan1 <= 1.0L) ? 1 : 0;
    results[5] = (nan1 >= 1.0L) ? 1 : 0;
    
    /* NaN vs NaN */
    results[6] = (nan1 == nan2) ? 1 : 0;
    results[7] = (nan1 != nan2) ? 1 : 0;
    
    /* NaN vs Inf */
    results[8] = (nan1 == inf) ? 1 : 0;
    results[9] = (nan1 < inf) ? 1 : 0;
    results[10] = (nan1 > neg_inf) ? 1 : 0;
    
    /* Arithmetic producing NaN */
    long double zero = 0.0L;
    long double neg_one = -1.0L;
    results[11] = (sqrtl(neg_one) != sqrtl(neg_one)) ? 1 : 0;  /* sqrt(-1) gives NaN */
    results[12] = (inf / inf != inf / inf) ? 1 : 0;  /* inf/inf gives NaN */
    results[13] = (zero / zero != zero / zero) ? 1 : 0;  /* 0/0 gives NaN */
}

int main() {
    /* Initialize array with mixed values */
    global_ld_array[0] = 1.0L;
    global_ld_array[1] = 2.0L;
    global_ld_array[2] = 3.14159265358979323846L;
    global_ld_array[3] = -1.0L;
    global_ld_array[4] = 0.0L;
    global_ld_array[5] = __builtin_infl();
    global_ld_array[6] = -__builtin_infl();
    global_ld_array[7] = __builtin_nanl("");
    global_ld_array[8] = 0.0L / 0.0L;  /* Another NaN */
    global_ld_array[9] = 1e-4932L;  /* Very small */
    global_ld_array[10] = 1e4932L;   /* Very large */
    global_ld_array[11] = -3.5L;
    global_ld_array[12] = 100.0L;
    global_ld_array[13] = __builtin_nanl("0xABCD");
    global_ld_array[14] = 2.71828182845904523536L;  /* e */
    global_ld_array[15] = sqrtl(-1.0L);  /* NaN from sqrt(-1) */
    
    int bool_results[256];
    int result_index = 0;
    
    /* Test 1: Basic ordered comparisons */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            bool_results[result_index++] = perform_all_comparisons(
                get_ld_value(i), 
                get_ld_value(j)
            ) & 1;  /* Just take LSB */
        }
    }
    
    /* Test 2: Complex x87 comparisons */
    bool_results[result_index++] = complex_x87_comparison(
        get_ld_value(0), get_ld_value(1),
        get_ld_value(2), get_ld_value(3)
    ) & 1;
    
    bool_results[result_index++] = complex_x87_comparison(
        get_ld_value(7), get_ld_value(5),  /* NaN vs Inf */
        get_ld_value(13), get_ld_value(4)  /* NaN vs 0 */
    ) & 1;
    
    /* Test 3: Loop with LD conditions */
    bool_results[result_index++] = loop_with_ld_condition(0.0L, 10.0L) & 1;
    bool_results[result_index++] = loop_with_ld_condition(
        get_ld_value(4), get_ld_value(12)
    ) & 1;
    
    /* Test 4: Switch on comparison results */
    bool_results[result_index++] = switch_on_comparison(
        get_ld_value(0), get_ld_value(1), get_ld_value(2)
    ) & 1;
    
    bool_results[result_index++] = switch_on_comparison(
        get_ld_value(7), get_ld_value(0), get_ld_value(1)  /* NaN first */
    ) & 1;
    
    /* Test 5: Explicit NaN scenarios */
    int nan_results[20];
    generate_nan_scenarios(nan_results);
    for (int i = 0; i < 14 && result_index < 255; i++) {
        bool_results[result_index++] = nan_results[i];
    }
    
    /* Test 6: Mixed precision comparisons */
    volatile double d1 = 3.14159;
    volatile float f1 = 2.71828f;
    bool_results[result_index++] = (get_ld_value(0) > d1) ? 1 : 0;
    bool_results[result_index++] = (f1 < get_ld_value(1)) ? 1 : 0;
    bool_results[result_index++] = ((long double)d1 == get_ld_value(2)) ? 1 : 0;
    bool_results[result_index++] = ((long double)f1 != get_ld_value(3)) ? 1 : 0;
    
    /* Test 7: Integer constant comparisons */
    bool_results[result_index++] = (get_ld_value(0) < 5) ? 1 : 0;
    bool_results[result_index++] = (get_ld_value(1) > -10) ? 1 : 0;
    bool_results[result_index++] = (100 == get_ld_value(12)) ? 1 : 0;
    bool_results[result_index++] = (0 != get_ld_value(4)) ? 1 : 0;
    
    /* Compute verification hash (XOR of all results) */
    int verification_hash = 0;
    for (int i = 0; i < result_index; i++) {
        verification_hash ^= bool_results[i];
    }
    
    /* Print hash to prevent dead code elimination */
    printf("Verification hash: %d\n", verification_hash);
    printf("Total comparisons performed: %d\n", result_index);
    
    return verification_hash != 0 ? 0 : 1;
}
