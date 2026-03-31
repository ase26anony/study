/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Loop with multiple carried dependencies and array accesses */
int loop_carried_deps(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependency */
    int i;
    
    /* Core pattern: a[i] depends on a[i-1] (distance-1 dependence) */
    /* This should set distance1_uses = true */
    for (i = 1; i < n; i++) {
        a[i] = a[i-1] * b[i] + c[i];  /* Distance-1 carried dependence */
        sum += a[i];                   /* Accumulator with carried dependence */
        
        /* Memory clobber to prevent optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    
    /* Additional operations to create more scheduling opportunities */
    for (i = 0; i < n-1; i++) {
        b[i] = b[i+1] * 3 - a[i];      /* Another distance-1 pattern */
        sum ^= b[i];                   /* Different operation for variety */
    }
    
    return sum;
}

/* Function 2: Nested loops with inner loop carried dependency */
int nested_loops(int n, int m, int *restrict x, int *restrict y) {
    volatile int acc = 0;
    int i, j;
    
    /* Outer loop partially unrolled */
    for (i = 0; i < n; i += 2) {
        int temp = 0;
        
        /* Inner loop with carried dependency */
        for (j = 1; j < m; j++) {
            y[j] = y[j-1] * x[j] + i;  /* Distance-1 in inner loop */
            temp += y[j] * (j % 7);
            
            /* Force dependency preservation */
            asm volatile("" : "+r"(temp) : : "memory");
        }
        
        acc += temp;
        x[i] = acc;  /* Store result with outer loop dependency */
    }
    
    return acc;
}

/* Function 3: Multiple interleaved accumulators */
int multi_accumulators(int n, int *restrict data, int *restrict coeffs) {
    volatile int sum1 = 0, sum2 = 0, sum3 = 0;
    int i;
    
    /* Three separate carried dependencies */
    for (i = 1; i < n; i++) {
        /* First accumulator: simple running sum */
        sum1 = sum1 + data[i] * coeffs[i];  /* Classic distance-1 */
        
        /* Second accumulator: with feedback */
        sum2 = sum2 * 2 + data[i-1];        /* Uses previous iteration's data */
        
        /* Third: more complex dependency chain */
        sum3 = (sum3 + sum1) * coeffs[i] - sum2;
        
        data[i] = sum1 + sum2 + sum3;  /* Write back with all dependencies */
        
        /* Prevent optimization */
        asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
    }
    
    return sum1 + sum2 + sum3;
}

/* Function 4: Loop with unknown trip count (parameter) */
int variable_loop(int n, int init_val, int *restrict arr) {
    volatile int state = init_val;
    int i;
    
    /* Loop count not known at compile time - forces modulo scheduling analysis */
    for (i = 0; i < n; i++) {
        /* Multiple operations with carried state */
        int t1 = state * 3 + 7;
        int t2 = t1 ^ arr[i];
        state = t2 - (i % 5);
        arr[i] = state;
        
        /* Complex expression to create more instruction-level parallelism */
        state = (state << 3) | (state >> 5);  /* Rotation */
        state = state * 0x9e3779b9;           /* Multiplication with constant */
        
        asm volatile("" : "+r"(state) : : "memory");
    }
    
    return state;
}

/* Function 5: Software pipelining candidate with high ILP */
int high_ilp_loop(int n, int *restrict a, int *restrict b, 
                  int *restrict c, int *restrict d) {
    volatile int acc1 = 0, acc2 = 0;
    int i;
    
    /* Loop designed to maximize instruction-level parallelism */
    for (i = 2; i < n; i++) {
        /* Multiple independent chains with carried dependencies */
        int t1 = a[i-2] * b[i-1] + c[i];    /* Distance-2 and distance-1 */
        int t2 = d[i] - a[i-1] * 3;         /* Another distance-1 */
        int t3 = t1 ^ t2;
        
        acc1 = acc1 + t3;
        acc2 = acc2 * 2 + t1;
        
        a[i] = acc1;
        b[i] = acc2;
        
        /* Cross-iteration dependencies */
        c[i] = c[i-1] + t2;                 /* Explicit distance-1 */
        d[i] = d[i-2] * 5 - t3;             /* Distance-2 */
        
        asm volatile("" : : "r"(acc1), "r"(acc2) : "memory");
    }
    
    return acc1 + acc2;
}

/* Main driver that calls all test functions */
int main(int argc, char **argv) {
    int n = 1000;
    int m = 500;
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    int *d = (int*)malloc(n * sizeof(int));
    int *x = (int*)malloc(n * sizeof(int));
    int *y = (int*)malloc(n * sizeof(int));
    int *data = (int*)malloc(n * sizeof(int));
    int *coeffs = (int*)malloc(n * sizeof(int));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < n; i++) {
        a[i] = i * 3 + 7;
        b[i] = i * 5 - 3;
        c[i] = i * 11 + 1;
        d[i] = i * 13 - 5;
        x[i] = i * 17 + 9;
        y[i] = i * 19 - 7;
        data[i] = i * 23 + 11;
        coeffs[i] = (i % 17) + 1;
    }
    
    /* Call all test functions to ensure they're compiled */
    int result = 0;
    
    result += loop_carried_deps(n, a, b, c);
    printf("Result 1: %d\n", result);
    
    result += nested_loops(n/4, m, x, y);
    printf("Result 2: %d\n", result);
    
    result += multi_accumulators(n, data, coeffs);
    printf("Result 3: %d\n", result);
    
    result += variable_loop(n, argc, a);  /* Use argc for variability */
    printf("Result 4: %d\n", result);
    
    result += high_ilp_loop(n, a, b, c, d);
    printf("Final result: %d\n", result);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(x); free(y); free(data); free(coeffs);
    
    return result != 0;  /* Non-zero return to indicate computation was done */
}
