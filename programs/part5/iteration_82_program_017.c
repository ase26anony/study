/* x87_comparison_test.c
 * Designed to trigger x87 comparison condition code output in i386.cc
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

/* Complex comparison function designed to use multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    int result = 0;
    
    /* Ordered comparisons */
    if (a < b) result |= 1;      /* LT */
    if (a > c) result |= 2;      /* GT */
    if (a <= d) result |= 4;     /* LE */
    if (b >= c) result |= 8;     /* GE */
    if (a == b) result |= 16;    /* EQ */
    if (c != d) result |= 32;    /* NEQ */
    
    /* Unordered comparisons with explicit NaN checks */
    volatile long double nan_val = __builtin_nanl("");
    if (!(a == a)) result |= 64;     /* Check if a is NaN (unordered with itself) */
    if (!(b == b)) result |= 128;    /* Check if b is NaN */
    
    /* Direct unordered comparisons */
    if (a != a || b != b) result |= 256;  /* UNORDERED */
    if ((a != a || b != b) || a == b) result |= 512;  /* UNEQ */
    
    return result;
}

/* Function with mixed precision comparisons */
int mixed_precision_comparisons(float f, double d, long double ld) {
    int result = 0;
    
    /* Mixed type comparisons causing promotions */
    if ((long double)f < ld) result |= 1;
    if (d > (long double)f) result |= 2;
    if ((long double)ld <= (long double)d) result |= 4;
    if (ld >= (long double)42.0) result |= 8;
    
    /* Integer constant comparisons */
    if (ld == (long double)100) result |= 16;
    if (ld != (long double)0) result |= 32;
    
    return result;
}

/* Function with arithmetic producing NaN */
void generate_nan_comparisons(long double *results, int *index) {
    volatile long double zero = 0.0L;
    volatile long double neg_one = -1.0L;
    volatile long double inf = __builtin_infl();
    
    /* Generate NaNs through various operations */
    long double nan1 = zero / zero;           /* 0/0 NaN */
    long double nan2 = inf / inf;             /* inf/inf NaN */
    long double nan3 = inf - inf;             /* inf-inf NaN */
    long double nan4 = sqrtl(neg_one);        /* sqrt(-1) NaN */
    long double nan5 = __builtin_nanl("");    /* Explicit quiet NaN */
    
    /* Compare NaNs with normal numbers */
    results[(*index)++] = (nan1 == 1.0L);      /* false - UNORDERED/UNEQ */
    results[(*index)++] = (nan1 != 2.0L);      /* true - UNORDERED/NEQ */
    results[(*index)++] = (nan1 < 3.0L);       /* false - UNORDERED */
    results[(*index)++] = (nan1 > 4.0L);       /* false - UNORDERED */
    results[(*index)++] = (nan1 <= 5.0L);      /* false - UNORDERED */
    results[(*index)++] = (nan1 >= 6.0L);      /* false - UNORDERED */
    
    /* Compare NaN with NaN */
    results[(*index)++] = (nan1 == nan2);      /* false - UNORDERED */
    results[(*index)++] = (nan1 != nan2);      /* true - UNORDERED/NEQ */
    results[(*index)++] = (nan1 < nan3);       /* false - UNORDERED */
    results[(*index)++] = (nan2 > nan4);       /* false - UNORDERED */
    
    /* Compare NaN with infinity */
    results[(*index)++] = (nan1 == inf);       /* false - UNORDERED */
    results[(*index)++] = (nan1 != inf);       /* true - UNORDERED/NEQ */
    results[(*index)++] = (nan1 < inf);        /* false - UNORDERED */
    results[(*index)++] = (nan1 > -inf);       /* false - UNORDERED */
}

/* Loop with long double termination condition */
int loop_with_ld_condition(long double start, long double end, long double step) {
    volatile long double counter = start;
    int iterations = 0;
    
    /* Loop condition using long double comparison */
    while (counter < end && iterations < 1000) {
        /* Nested comparison in loop body */
        if (counter != start && counter > (start + end) / 2.0L) {
            iterations += 2;
        } else {
            iterations++;
        }
        
        /* Complex condition with multiple comparisons */
        if (counter < end / 2.0L || counter > start * 2.0L) {
            iterations |= 0x100;
        }
        
        counter += step;
        
        /* Prevent infinite loops with NaN */
        if (counter != counter) break;  /* NaN check */
    }
    
    return iterations;
}

/* Switch statement based on comparison results */
int switch_on_comparison(long double a, long double b) {
    int result = 0;
    
    /* Use comparison results in switch */
    if (a < b) {
        result = 1;
    } else if (a > b) {
        result = 2;
    } else if (a == b) {
        result = 3;
    } else {
        /* This handles NaN cases (unordered) */
        result = 4;
    }
    
    /* Nested switch with complex conditions */
    switch (result) {
        case 1:
            if (a * 2.0L > b) result |= 0x10;
            break;
        case 2:
            if (b * 2.0L < a) result |= 0x20;
            break;
        case 3:
            if (a == a && b == b) result |= 0x40;  /* Both are not NaN */
            break;
        case 4:
            /* Handle NaN cases - unordered comparisons */
            if (a != a || b != b) result |= 0x80;
            break;
    }
    
    return result;
}

/* Main test function */
int main() {
    /* Initialize array with various long double values */
    global_ld_array[0] = 1.0L;
    global_ld_array[1] = 2.0L;
    global_ld_array[2] = 3.14159265358979323846L;  /* pi */
    global_ld_array[3] = 2.71828182845904523536L;  /* e */
    global_ld_array[4] = 0.0L;
    global_ld_array[5] = -1.0L;
    global_ld_array[6] = 100.0L;
    global_ld_array[7] = 1.0e-10L;
    global_ld_array[8] = 1.0e10L;
    global_ld_array[9] = __builtin_infl();         /* +inf */
    global_ld_array[10] = -__builtin_infl();       /* -inf */
    global_ld_array[11] = __builtin_nanl("");      /* NaN */
    global_ld_array[12] = 0.0L / 0.0L;             /* NaN */
    global_ld_array[13] = sqrtl(-1.0L);            /* NaN */
    global_ld_array[14] = 42.0L;
    global_ld_array[15] = -999.999L;
    
    /* Array to store boolean results */
    int bool_results[256];
    int result_count = 0;
    
    /* Test 1: Basic long double comparisons */
    for (int i = 0; i < 8; i++) {
        long double a = get_ld_value(i);
        long double b = get_ld_value(i + 1);
        
        bool_results[result_count++] = (a < b);
        bool_results[result_count++] = (a > b);
        bool_results[result_count++] = (a <= b);
        bool_results[result_count++] = (a >= b);
        bool_results[result_count++] = (a == b);
        bool_results[result_count++] = (a != b);
    }
    
    /* Test 2: Complex x87 comparison function */
    for (int i = 0; i < 4; i++) {
        int complex_result = complex_x87_comparison(
            get_ld_value(i),
            get_ld_value(i + 4),
            get_ld_value(i + 8),
            get_ld_value(i + 12)
        );
        
        /* Store individual bits as boolean results */
        for (int bit = 0; bit < 10; bit++) {
            bool_results[result_count++] = (complex_result >> bit) & 1;
        }
    }
    
    /* Test 3: NaN comparisons */
    generate_nan_comparisons(bool_results, &result_count);
    
    /* Test 4: Mixed precision */
    for (int i = 0; i < 4; i++) {
        float f = (float)get_ld_value(i);
        double d = (double)get_ld_value(i + 4);
        long double ld = get_ld_value(i + 8);
        
        int mixed_result = mixed_precision_comparisons(f, d, ld);
        for (int bit = 0; bit < 6; bit++) {
            bool_results[result_count++] = (mixed_result >> bit) & 1;
        }
    }
    
    /* Test 5: Loop with long double conditions */
    int loop_result = loop_with_ld_condition(0.0L, 10.0L, 0.1L);
    for (int bit = 0; bit < 8; bit++) {
        bool_results[result_count++] = (loop_result >> bit) & 1;
    }
    
    /* Test 6: Switch on comparison results */
    for (int i = 0; i < 8; i++) {
        int switch_result = switch_on_comparison(
            get_ld_value(i),
            get_ld_value(i + 8)
        );
        for (int bit = 0; bit < 8; bit++) {
            bool_results[result_count++] = (switch_result >> bit) & 1;
        }
    }
    
    /* Test 7: Direct unordered comparison scenarios */
    volatile long double nan_val = __builtin_nanl("");
    volatile long double inf_val = __builtin_infl();
    volatile long double normal = 5.0L;
    
    /* These should trigger UNORDERED, UNEQ, UNGE, UNGT, UNLE, UNLT cases */
    bool_results[result_count++] = (nan_val == normal);   /* false - UNORDERED */
    bool_results[result_count++] = (nan_val != normal);   /* true - UNORDERED/NEQ */
    bool_results[result_count++] = !(nan_val < normal);   /* true - UNGE (nlt) */
    bool_results[result_count++] = !(nan_val <= normal);  /* true - UNGT (nle) */
    bool_results[result_count++] = (nan_val < normal || nan_val == normal);  /* false - UNLE? */
    bool_results[result_count++] = (nan_val <= normal || nan_val != nan_val); /* true? - complex */
    
    /* LTGT (une) case: not equal and ordered */
    bool_results[result_count++] = (normal != 6.0L);      /* true - LTGT if ordered */
    bool_results[result_count++] = (normal != normal);    /* false - not LTGT (unordered) */
    
    /* Final verification hash to prevent dead code elimination */
    unsigned int hash = 0;
    for (int i = 0; i < result_count; i++) {
        hash ^= (bool_results[i] << (i % 32));
    }
    
    printf("Test completed. Result hash: 0x%08x\n", hash);
    printf("Total comparisons performed: %d\n", result_count);
    
    return (hash != 0) ? 0 : 1;
}
