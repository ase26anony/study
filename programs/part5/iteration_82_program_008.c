/* x87_comparison_test.c
 * Designed to trigger x87 comparison condition code output in i386.cc
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -o x87_test x87_comparison_test.c
 */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ld_array[16];
volatile int array_index = 0;

/* Helper to get unpredictable long double values */
long double get_ld_value(int idx) {
    return global_ld_array[idx % 16];
}

/* Complex comparison function using multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    int result = 0;
    
    /* Ordered comparisons (should generate "ord" type codes) */
    if (a < b && !(a != a) && !(b != b)) {
        result |= 1;  /* LT ordered */
    }
    
    if (c > d && c == c && d == d) {
        result |= 2;  /* GT ordered */
    }
    
    /* Equality comparisons */
    if (a == b) {
        result |= 4;  /* EQ */
    }
    
    if (c != d) {
        result |= 8;  /* NEQ/UNE */
    }
    
    /* Unordered comparisons with explicit NaN checks */
    if (a != a || b != b) {  /* UNORDERED */
        result |= 16;
    }
    
    if (!(a != a) && !(b != b)) {  /* ORDERED */
        result |= 32;
    }
    
    /* UNEQ: unordered or equal */
    if ((a != a || b != b) || (a == b)) {
        result |= 64;
    }
    
    /* UNGE: unordered or greater-or-equal */
    if ((a != a || b != b) || (a >= b)) {
        result |= 128;
    }
    
    /* UNGT: unordered or greater */
    if ((a != a || b != b) || (a > b)) {
        result |= 256;
    }
    
    /* UNLE: unordered or less-or-equal */
    if ((a != a || b != b) || (a <= b)) {
        result |= 512;
    }
    
    /* UNLT: unordered or less */
    if ((a != a || b != b) || (a < b)) {
        result |= 1024;
    }
    
    /* LTGT: less or greater (ordered and not equal) */
    if ((a < b || a > b) && (a == a && b == b)) {
        result |= 2048;
    }
    
    return result;
}

/* Function with nested comparisons */
int nested_x87_comparisons(long double x, long double y, long double z) {
    if (x < y) {
        if (y > z) {
            if (x != z) {
                return 1;
            } else if (x == z) {
                return 2;
            }
        } else if (y <= z) {
            if (x >= 0.0L) {
                return 3;
            }
        }
    } else if (x > y) {
        if (y < z) {
            return 4;
        }
    } else {  /* x == y */
        if (z != z) {  /* z is NaN */
            return 5;
        }
    }
    
    /* Complex boolean expression */
    if ((x < y && y > z) || (x > y && y < z) || (x == y && z != z)) {
        return 6;
    }
    
    return 0;
}

/* Loop with long double termination condition */
int loop_with_x87_condition(long double start, long double limit) {
    volatile long double counter = start;
    int iterations = 0;
    
    /* Use volatile to prevent optimization */
    while (counter < limit && iterations < 100) {
        counter += 0.5L;
        iterations++;
        
        /* Additional comparison inside loop */
        if (counter != counter) {  /* Check for NaN */
            break;
        }
    }
    
    return iterations;
}

/* Mixed precision comparisons */
int mixed_precision_comparisons(float f, double d, long double ld) {
    int result = 0;
    
    /* Compare different precisions (will promote to long double) */
    if ((long double)f < ld) {
        result |= 1;
    }
    
    if (d > (double)ld) {  /* Cast to double for different comparison */
        result |= 2;
    }
    
    if ((long double)f == (long double)d) {
        result |= 4;
    }
    
    /* Compare with integer constant */
    if (ld > 10.0L) {
        result |= 8;
    }
    
    if ((long double)(int)f <= ld) {
        result |= 16;
    }
    
    return result;
}

/* NaN-specific comparisons */
int nan_comparison_tests(void) {
    int result = 0;
    
    /* Generate NaNs in different ways */
    long double nan1 = __builtin_nanl("");
    long double nan2 = 0.0L / 0.0L;
    long double nan3 = sqrtl(-1.0L);
    long double inf = __builtin_infl();
    long double normal = 3.14159265358979323846L;
    
    /* Compare NaN with normal numbers (all should be false for ordered comparisons) */
    if (!(nan1 < normal))  result |= 1;
    if (!(nan1 > normal))  result |= 2;
    if (!(nan1 <= normal)) result |= 4;
    if (!(nan1 >= normal)) result |= 8;
    if (!(nan1 == normal)) result |= 16;
    if (nan1 != normal)    result |= 32;  /* This should be true! */
    
    /* Compare NaN with infinity */
    if (!(nan2 < inf))     result |= 64;
    if (!(nan2 > inf))     result |= 128;
    if (nan2 != inf)       result |= 256;
    
    /* Compare NaN with NaN */
    if (!(nan1 == nan2))   result |= 512;
    if (nan1 != nan2)      result |= 1024;  /* NaN != NaN is true */
    
    /* Check for unordered */
    if (nan3 != nan3)      result |= 2048;  /* NaN != NaN test */
    
    /* Ordered comparison that should be false with NaN */
    if (!(nan1 < nan1 && nan1 == nan1)) result |= 4096;
    
    return result;
}

/* Initialize array with mix of values */
void init_global_array(void) {
    for (int i = 0; i < 16; i++) {
        switch (i % 8) {
            case 0: global_ld_array[i] = 0.0L; break;
            case 1: global_ld_array[i] = 1.0L; break;
            case 2: global_ld_array[i] = -1.0L; break;
            case 3: global_ld_array[i] = 100.0L; break;
            case 4: global_ld_array[i] = __builtin_infl(); break;
            case 5: global_ld_array[i] = -__builtin_infl(); break;
            case 6: global_ld_array[i] = __builtin_nanl(""); break;
            case 7: global_ld_array[i] = 0.0L / 0.0L; break;
        }
    }
}

int main(void) {
    init_global_array();
    
    int bool_results[100];
    int result_index = 0;
    
    /* Test 1: Complex comparisons */
    bool_results[result_index++] = complex_x87_comparison(
        get_ld_value(0), get_ld_value(1),
        get_ld_value(2), get_ld_value(3)
    ) != 0;
    
    /* Test 2: Nested comparisons */
    bool_results[result_index++] = nested_x87_comparisons(
        get_ld_value(4), get_ld_value(5), get_ld_value(6)
    ) > 0;
    
    /* Test 3: Loop with x87 condition */
    bool_results[result_index++] = loop_with_x87_condition(0.0L, 10.0L) > 5;
    
    /* Test 4: Mixed precision */
    bool_results[result_index++] = mixed_precision_comparisons(
        3.14f, 2.71828, get_ld_value(7)
    ) != 0;
    
    /* Test 5: NaN comparisons */
    bool_results[result_index++] = nan_comparison_tests() > 0;
    
    /* Additional direct comparisons to hit specific cases */
    volatile long double v1 = get_ld_value(8);
    volatile long double v2 = get_ld_value(9);
    volatile long double v3 = get_ld_value(10);
    volatile long double v4 = get_ld_value(11);
    
    /* Generate various comparison patterns */
    bool_results[result_index++] = (v1 < v2) && (v3 > v4);
    bool_results[result_index++] = (v1 <= v2) || (v3 >= v4);
    bool_results[result_index++] = (v1 == v2) != (v3 == v4);
    bool_results[result_index++] = (v1 != v2) && !(v3 != v3);
    bool_results[result_index++] = !(v1 < v2) && !(v1 > v2) && (v1 == v1);
    
    /* Explicit unordered checks */
    bool_results[result_index++] = (v1 != v1) || (v2 != v2);
    bool_results[result_index++] = !(v1 != v1) && !(v2 != v2);
    
    /* More complex expressions */
    for (int i = 0; i < 8 && result_index < 95; i++) {
        long double a = get_ld_value(i);
        long double b = get_ld_value(i + 1);
        long double c = get_ld_value(i + 2);
        
        bool_results[result_index++] = (a < b) && (b > c);
        bool_results[result_index++] = (a > b) || (b < c);
        bool_results[result_index++] = (a == b) != (b == c);
        bool_results[result_index++] = (a != b) && (b != c);
        bool_results[result_index++] = !(a < b) && !(a > b);
        bool_results[result_index++] = (a != a) || (b != b) || (c != c);
        bool_results[result_index++] = ((a != a) || (b != b)) && (a == b);
        bool_results[result_index++] = ((a != a) || (b != b)) || (a >= b);
        bool_results[result_index++] = ((a != a) || (b != b)) || (a > b);
        bool_results[result_index++] = ((a != a) || (b != b)) || (a <= b);
        bool_results[result_index++] = ((a != a) || (b != b)) || (a < b);
        bool_results[result_index++] = (a < b || a > b) && (a == a && b == b);
    }
    
    /* Compute verification hash to prevent dead code elimination */
    int verification_hash = 0;
    for (int i = 0; i < result_index; i++) {
        verification_hash ^= (bool_results[i] << (i % 32));
    }
    
    printf("Verification hash: %d\n", verification_hash);
    printf("Number of tests executed: %d\n", result_index);
    
    return (verification_hash == 0) ? 0 : 1;
}
