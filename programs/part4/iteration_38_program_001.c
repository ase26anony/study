/* test_modulo_sched.c - Program to trigger GCC modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Loop with multiple carried dependencies and array accesses */
int loop_carried_deps(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependencies */
    int i;
    
    /* Core pattern: a[i] depends on a[i-1] (distance-1 dependence) */
    for (i = 1; i < n; i++) {
        /* Multiple operations with carried dependencies */
        a[i] = a[i-1] * b[i] + c[i];  /* Distance-1 dependence via a[i-1] */
        
        /* Additional accumulator with distance-1 dependence */
        sum = sum + a[i] * 3;  /* sum carries across iterations */
        
        /* Another distance-1 pattern with different arrays */
        b[i] = b[i-1] + c[i] * 2;
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    
    return sum + a[n-1];
}

/* Function 2: Nested loops with inner loop having carried dependency */
int nested_loop_carried(int n, int m, int *restrict x, int *restrict y) {
    volatile int acc = 0;
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Unroll pragma to encourage modulo scheduling */
        #pragma GCC unroll 2
        for (j = 1; j < m; j++) {
            /* Inner loop with distance-1 dependence */
            x[j] = x[j-1] + y[j] * i;
            
            /* Multiple accumulators */
            acc = acc + x[j];
            
            /* Another distance-1 pattern */
            y[j] = y[j-1] * 2 - x[j];
            
            /* Force dependence preservation */
            asm volatile("" : "+r"(acc) : : "memory");
        }
    }
    
    return acc;
}

/* Function 3: Multiple interleaved carried dependencies */
int multi_carried_deps(int n, int *restrict p, int *restrict q, 
                       int *restrict r, int *restrict s) {
    volatile int sum1 = 0, sum2 = 0;
    int i;
    
    for (i = 2; i < n; i++) {
        /* Pattern 1: p depends on two iterations back */
        p[i] = p[i-2] * q[i] + r[i];
        
        /* Pattern 2: q has distance-1 dependence */
        q[i] = q[i-1] + s[i] * 3;
        
        /* Pattern 3: r has distance-1 from different array */
        r[i] = r[i-1] - p[i] * 2;
        
        /* Two separate accumulators with carried dependencies */
        sum1 = sum1 + p[i] * q[i];
        sum2 = sum2 + r[i] + s[i];
        
        /* Prevent optimization */
        asm volatile("" : : "r"(sum1), "r"(sum2) : "memory");
    }
    
    return sum1 + sum2;
}

/* Function 4: Loop with unknown trip count (parameter) */
int variable_trip_count(int start, int end, int *restrict arr1, 
                        int *restrict arr2, int *restrict arr3) {
    volatile int total = 0;
    int i;
    
    /* Loop count not known at compile time */
    for (i = start + 1; i < end; i++) {
        /* Complex distance-1 patterns */
        arr1[i] = arr1[i-1] * arr2[i] + arr3[i % 16];
        arr2[i] = arr2[i-1] + arr1[i] * 7;
        
        /* Accumulator with multiple uses */
        total = total + arr1[i] - arr2[i];
        
        /* Memory clobber to preserve all dependencies */
        asm volatile("" : : "r"(total) : "memory");
    }
    
    return total;
}

/* Function 5: Mixed operations with high ILP potential */
int high_ilp_loop(int n, int *restrict d1, int *restrict d2, 
                  int *restrict d3, int *restrict d4) {
    volatile int acc1 = 0, acc2 = 0, acc3 = 0;
    int i;
    
    for (i = 1; i < n; i++) {
        /* Multiple independent chains with distance-1 deps */
        d1[i] = d1[i-1] + d2[i] * 5;
        d2[i] = d2[i-1] - d3[i] * 3;
        d3[i] = d3[i-1] + d4[i] * 2;
        d4[i] = d4[i-1] * d1[i] + 7;
        
        /* Multiple accumulators creating pressure */
        acc1 = acc1 + d1[i] * d2[i];
        acc2 = acc2 + d3[i] - d4[i];
        acc3 = acc3 + d1[i] + d3[i] * 2;
        
        /* Force all dependencies to be respected */
        asm volatile("" : : "r"(acc1), "r"(acc2), "r"(acc3) : "memory");
    }
    
    return acc1 + acc2 + acc3;
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int *a, int n) {
    int i;
    for (i = 0; i < n; i++) {
        a[i] = (i * 13 + 7) % 100;
    }
}

int main() {
    const int N = 1024;
    int *a, *b, *c, *x, *y, *p, *q, *r, *s;
    int *d1, *d2, *d3, *d4;
    int result = 0;
    
    /* Allocate and initialize arrays */
    a = malloc(N * sizeof(int));
    b = malloc(N * sizeof(int));
    c = malloc(N * sizeof(int));
    x = malloc(N * sizeof(int));
    y = malloc(N * sizeof(int));
    p = malloc(N * sizeof(int));
    q = malloc(N * sizeof(int));
    r = malloc(N * sizeof(int));
    s = malloc(N * sizeof(int));
    d1 = malloc(N * sizeof(int));
    d2 = malloc(N * sizeof(int));
    d3 = malloc(N * sizeof(int));
    d4 = malloc(N * sizeof(int));
    
    if (!a || !b || !c || !x || !y || !p || !q || !r || !s || 
        !d1 || !d2 || !d3 || !d4) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize all arrays */
    init_arrays(a, N);
    init_arrays(b, N);
    init_arrays(c, N);
    init_arrays(x, N);
    init_arrays(y, N);
    init_arrays(p, N);
    init_arrays(q, N);
    init_arrays(r, N);
    init_arrays(s, N);
    init_arrays(d1, N);
    init_arrays(d2, N);
    init_arrays(d3, N);
    init_arrays(d4, N);
    
    /* Call all test functions to ensure they're compiled */
    result += loop_carried_deps(N, a, b, c);
    result += nested_loop_carried(8, N/8, x, y);
    result += multi_carried_deps(N, p, q, r, s);
    result += variable_trip_count(10, N-10, a, b, c);
    result += high_ilp_loop(N, d1, d2, d3, d4);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(x); free(y);
    free(p); free(q); free(r); free(s);
    free(d1); free(d2); free(d3); free(d4);
    
    return 0;
}
