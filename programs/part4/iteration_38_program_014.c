/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Loop with multiple carried dependencies and array accesses */
int loop_carried_deps(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependence */
    int i;
    
    /* Core pattern: a[i] depends on a[i-1] (distance-1 dependence) */
    for (i = 1; i < n; i++) {
        /* Multiple operations with carried dependencies */
        a[i] = a[i-1] * b[i] + c[i];  /* distance-1 dependence on a[i-1] */
        sum = sum + a[i];              /* accumulator with carried dependence */
        
        /* Additional operations to create instruction-level parallelism */
        b[i] = b[i-1] + c[i] * 2;      /* another distance-1 dependence */
        c[i] = c[i-1] ^ (a[i] & 0xFF); /* yet another carried dependence */
        
        /* Memory barrier to prevent over-optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    
    return sum;
}

/* Function 2: Nested loop with inner loop having carried dependency */
int nested_loop_carried(int n, int m, int *restrict x, int *restrict y) {
    volatile int total = 0;
    int i, j;
    
    for (i = 0; i < n; i++) {
        int acc = 0;
        /* Inner loop with carried dependency */
        for (j = 1; j < m; j++) {
            /* Pattern: y[j] depends on y[j-1] */
            y[j] = y[j-1] * x[i] + (j * 3);
            acc = acc + y[j];  /* accumulator */
            
            /* Additional operations */
            x[i] = x[i] ^ (y[j] & 0x0F);
        }
        total = total + acc;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(total) : "memory");
    }
    
    return total;
}

/* Function 3: Multiple interleaved accumulators with different dependencies */
int multi_accumulators(int n, int *restrict arr1, int *restrict arr2, 
                       int *restrict arr3) {
    volatile int sum1 = 0, sum2 = 0, sum3 = 0;
    int i;
    
    for (i = 1; i < n; i++) {
        /* Three separate carried dependency chains */
        arr1[i] = arr1[i-1] * 3 + arr2[i];    /* distance-1 on arr1 */
        sum1 = sum1 + arr1[i];
        
        arr2[i] = arr2[i-1] + arr1[i] * 2;    /* distance-1 on arr2 */
        sum2 = sum2 + arr2[i];
        
        arr3[i] = arr3[i-1] ^ (arr1[i] + arr2[i]); /* distance-1 on arr3 */
        sum3 = sum3 + arr3[i];
        
        /* Complex expression to create more scheduling opportunities */
        arr1[i] = (arr1[i] * 7) >> 1;
        arr2[i] = arr2[i] + (arr3[i-1] & 0x3F);
        
        /* Memory barrier */
        asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
    }
    
    return sum1 + sum2 + sum3;
}

/* Function 4: Loop with unknown trip count (parameter) */
int variable_loop(int n, int init_val, int *restrict data) {
    volatile int state = init_val;
    int i;
    
    /* Strong carried dependency through 'state' variable */
    for (i = 0; i < n; i++) {
        /* Multiple uses of the carried value */
        int temp = state * data[i];
        data[i] = temp + (i * 5);
        state = state ^ data[i];
        
        /* Additional arithmetic to create parallel opportunities */
        data[i] = data[i] + (state >> 2);
        state = state * 13 + 1;
        
        /* Force dependence preservation */
        asm volatile("" : : "r"(state) : "memory");
    }
    
    return state;
}

/* Function 5: Complex loop with mixed operations */
int complex_mixed_ops(int n, int *restrict p, int *restrict q, 
                      int *restrict r) {
    volatile int acc = 0;
    int i;
    
    for (i = 2; i < n; i++) {
        /* Multiple interleaved distance-1 dependencies */
        p[i] = p[i-1] + q[i-2] * r[i];      /* distance-1 on p, distance-2 on q */
        q[i] = q[i-1] - p[i] / 3;           /* distance-1 on q */
        r[i] = r[i-1] ^ (p[i-1] + q[i]);    /* distance-1 on r */
        
        /* Accumulator with complex expression */
        acc = acc + p[i] * q[i] - r[i];
        
        /* More operations to increase ILP */
        p[i] = p[i] | (q[i] & 0xFF);
        q[i] = (q[i] * 11) % 256;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(acc) : "memory");
    }
    
    return acc;
}

/* Main driver function */
int main(int argc, char *argv[]) {
    const int SIZE = 1024;
    int i, result = 0;
    
    /* Allocate and initialize arrays */
    int *a = (int *)malloc(SIZE * sizeof(int));
    int *b = (int *)malloc(SIZE * sizeof(int));
    int *c = (int *)malloc(SIZE * sizeof(int));
    int *x = (int *)malloc(SIZE * sizeof(int));
    int *y = (int *)malloc(SIZE * sizeof(int));
    int *arr1 = (int *)malloc(SIZE * sizeof(int));
    int *arr2 = (int *)malloc(SIZE * sizeof(int));
    int *arr3 = (int *)malloc(SIZE * sizeof(int));
    int *data = (int *)malloc(SIZE * sizeof(int));
    int *p = (int *)malloc(SIZE * sizeof(int));
    int *q = (int *)malloc(SIZE * sizeof(int));
    int *r = (int *)malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random but deterministic values */
    for (i = 0; i < SIZE; i++) {
        a[i] = i * 3 + 1;
        b[i] = i * 5 + 2;
        c[i] = i * 7 + 3;
        x[i] = i * 11 + 5;
        y[i] = i * 13 + 7;
        arr1[i] = i * 17 + 11;
        arr2[i] = i * 19 + 13;
        arr3[i] = i * 23 + 17;
        data[i] = i * 29 + 19;
        p[i] = i * 31 + 23;
        q[i] = i * 37 + 29;
        r[i] = i * 41 + 31;
    }
    
    /* Call all test functions to ensure they're compiled */
    result += loop_carried_deps(SIZE, a, b, c);
    result += nested_loop_carried(SIZE/16, 16, x, y);
    result += multi_accumulators(SIZE, arr1, arr2, arr3);
    result += variable_loop(SIZE, 42, data);
    result += complex_mixed_ops(SIZE, p, q, r);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(x); free(y);
    free(arr1); free(arr2); free(arr3);
    free(data);
    free(p); free(q); free(r);
    
    return 0;
}
