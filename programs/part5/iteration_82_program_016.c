/* x87_comparison_test.c
 * Designed to trigger x87 comparison condition code output in i386.cc
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -fno-fast-math -o x87_test x87_comparison_test.c
 */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ldbl_array[16];

/* Helper to get unpredictable long double values */
long double get_ldbl(int idx) {
    static volatile long double values[] = {
        1.0L, 2.0L, 3.14159265358979323846L, -1.0L,
        0.0L, -0.0L, 1.0e308L, -1.0e308L
    };
    return values[idx & 7];
}

/* Complex comparison function designed to use multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    /* Ordered comparisons */
    int r1 = (a < b);   /* Should generate x87 comparison + conditional */
    int r2 = (c > d);   /* Another ordered comparison */
    
    /* Equality comparisons */
    int r3 = (a == b);
    int r4 = (c != d);
    
    /* Combined ordered comparisons */
    int r5 = (a <= b) && (c >= d);
    int r6 = (a > b) || (c < d);
    
    /* Return complex result to force code generation */
    return (r1 ^ r2) | (r3 & r4) | (r5 ^ r6);
}

/* Function focusing on unordered comparisons with NaN */
int unordered_comparisons(long double x, long double y) {
    int results = 0;
    
    /* Generate NaN values */
    long double nan1 = __builtin_nanl("");
    long double nan2 = 0.0L / 0.0L;
    long double nan3 = sqrtl(-1.0L);
    
    /* Comparisons involving NaN (should trigger UNORDERED, UNEQ, etc.) */
    results |= (x != nan1) << 0;      /* une */
    results |= (x == nan1) << 1;      /* unordered? */
    results |= (x < nan1) << 2;       /* unordered */
    results |= (x > nan1) << 3;       /* unordered */
    results |= (x <= nan1) << 4;      /* unordered */
    results |= (x >= nan1) << 5;      /* unordered */
    
    /* NaN vs NaN comparisons */
    results |= (nan1 == nan2) << 6;   /* unordered */
    results |= (nan1 != nan2) << 7;   /* une */
    results |= (nan1 < nan2) << 8;    /* unordered */
    results |= (nan1 > nan2) << 9;    /* unordered */
    
    /* Normal vs NaN */
    results |= (1.0L < nan3) << 10;   /* unordered */
    results |= (1.0L > nan3) << 11;   /* unordered */
    results |= (1.0L == nan3) << 12;  /* unordered */
    results |= (1.0L != nan3) << 13;  /* une */
    
    return results;
}

/* Mixed precision comparisons */
int mixed_precision_comparisons(float f, double d, long double ld) {
    int results = 0;
    
    /* Float to long double promotion */
    results |= (f < ld) << 0;
    results |= (f > ld) << 1;
    results |= (f == ld) << 2;
    results |= (f != ld) << 3;
    
    /* Double to long double promotion */
    results |= (d < ld) << 4;
    results |= (d > ld) << 5;
    results |= (d == ld) << 6;
    results |= (d != ld) << 7;
    
    /* With integer constants */
    results |= (ld < 10) << 8;
    results |= (ld > -5) << 9;
    results |= (ld == 0) << 10;
    results |= (ld != 1) << 11;
    
    /* Explicit casts */
    results |= (ld < (long double)f) << 12;
    results |= (ld > (long double)d) << 13;
    
    return results;
}

/* Control flow based on long double comparisons */
int control_flow_comparisons(long double a, long double b, long double c) {
    int result = 0;
    
    /* if-else chain */
    if (a < b) {
        result += 1;
    } else if (a > b) {
        result += 2;
    } else if (a == b) {
        result += 3;
    } else {
        /* This should be taken for NaN comparisons */
        result += 4;
    }
    
    /* Nested comparisons */
    if ((a < b) && (b < c)) {
        result += 10;
    }
    
    if ((a > b) || (b > c)) {
        result += 20;
    }
    
    /* Complex condition */
    if ((a != b) && (c <= 100.0L) && !__builtin_isnan(a)) {
        result += 30;
    }
    
    return result;
}

/* Loop with long double termination condition */
int loop_with_ldbl_condition(long double start, long double limit) {
    volatile long double x = start;
    int count = 0;
    
    /* while loop with long double comparison */
    while (x < limit && count < 100) {
        x = x * 1.1L;
        count++;
        
        /* Additional comparison inside loop */
        if (x > limit / 2.0L) {
            count += 2;
        }
    }
    
    /* do-while with unordered check */
    volatile long double y = start;
    do {
        y = y / 1.5L;
        count--;
    } while (y > 0.0L && !__builtin_isnan(y));
    
    return count;
}

/* Main test function */
int main(void) {
    /* Initialize global array with mixed values */
    global_ldbl_array[0] = 1.0L;
    global_ldbl_array[1] = __builtin_nanl("");
    global_ldbl_array[2] = __builtin_infl();
    global_ldbl_array[3] = -__builtin_infl();
    global_ldbl_array[4] = 0.0L;
    global_ldbl_array[5] = -0.0L;
    global_ldbl_array[6] = 3.14159265358979323846L;
    global_ldbl_array[7] = 1.0e-308L;
    
    /* Results array to prevent dead code elimination */
    int results[64];
    int result_index = 0;
    
    /* Test 1: Basic ordered comparisons */
    for (int i = 0; i < 8; i += 2) {
        results[result_index++] = complex_x87_comparison(
            global_ldbl_array[i],
            global_ldbl_array[i + 1],
            get_ldbl(i),
            get_ldbl(i + 1)
        );
    }
    
    /* Test 2: Unordered comparisons with NaN */
    results[result_index++] = unordered_comparisons(
        global_ldbl_array[0],
        global_ldbl_array[1]
    );
    
    /* Test 3: Mixed precision */
    results[result_index++] = mixed_precision_comparisons(
        3.14f,
        2.718281828459045,
        global_ldbl_array[6]
    );
    
    /* Test 4: Control flow */
    results[result_index++] = control_flow_comparisons(
        global_ldbl_array[0],
        global_ldbl_array[1],
        global_ldbl_array[2]
    );
    
    /* Test 5: Loop conditions */
    results[result_index++] = loop_with_ldbl_condition(1.0L, 100.0L);
    
    /* Additional direct comparisons to hit specific cases */
    volatile long double v1 = get_ldbl(0);
    volatile long double v2 = get_ldbl(1);
    volatile long double v3 = get_ldbl(2);
    volatile long double v4 = get_ldbl(3);
    
    /* Direct comparisons that should generate specific condition codes */
    results[result_index++] = (v1 < v2);    /* LT */
    results[result_index++] = (v1 > v2);    /* GT */
    results[result_index++] = (v1 <= v2);   /* LE */
    results[result_index++] = (v1 >= v2);   /* GE */
    results[result_index++] = (v1 == v2);   /* EQ */
    results[result_index++] = (v1 != v2);   /* NEQ */
    
    /* NaN comparisons that should hit the uncovered cases */
    volatile long double nan_val = __builtin_nanl("");
    results[result_index++] = (v1 < nan_val);   /* UNORDERED */
    results[result_index++] = (v1 > nan_val);   /* UNORDERED */
    results[result_index++] = (v1 == nan_val);  /* UNORDERED */
    results[result_index++] = (v1 != nan_val);  /* UNEQ or LTGT */
    results[result_index++] = (v1 <= nan_val);  /* UNLE */
    results[result_index++] = (v1 >= nan_val);  /* UNGE */
    
    /* More complex expressions */
    results[result_index++] = ((v1 < v2) && (v3 > v4)) || (v1 != v3);
    results[result_index++] = !((v1 == v2) || (v3 <= v4));
    
    /* Compute final hash to ensure all code executes */
    int final_hash = 0;
    for (int i = 0; i < result_index; i++) {
        final_hash ^= results[i] ^ i;
    }
    
    printf("Result hash: %d\n", final_hash);
    printf("Number of comparisons performed: %d\n", result_index);
    
    return final_hash != 0 ? 0 : 1;
}
