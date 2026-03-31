/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Loop with multiple carried dependencies and array accesses */
int loop_carried_deps(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependency */
    int i;
    
    /* Core pattern: a[i] depends on a[i-1] (distance-1 dependence) */
    /* This should create distance1_uses = true scenario */
    for (i = 1; i < n; i++) {
        /* Multiple operations to create non-trivial schedule */
        a[i] = a[i-1] * b[i] + c[i];      /* Distance-1 dependence on a[i-1] */
        sum += a[i];                       /* Accumulator with carried dependency */
        
        /* Additional operations for instruction-level parallelism */
        b[i] = b[i] * 3 - 7;
        c[i] = c[i-1] + sum * 2;          /* Another distance-1 dependence */
        
        /* Memory barrier to prevent over-optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    return sum;
}

/* Function 2: Nested loop with inner loop having carried dependency */
int nested_loop_deps(int n, int m, int *restrict x, int *restrict y) {
    volatile int acc1 = 0, acc2 = 0;
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Unrolled outer loop operations */
        acc1 = acc1 * 2 + x[i];
        
        /* Inner loop with carried dependency */
        for (j = 1; j < m; j++) {
            y[j] = y[j-1] + x[i] * j;      /* Distance-1 dependence in inner loop */
            acc2 += y[j] * 3;
            
            /* Complex expression for scheduling challenge */
            x[i] = (x[i] * 7 + y[j]) / 5;
        }
        
        /* Prevent dead code elimination */
        asm volatile("" : : "r"(acc1), "r"(acc2) : "memory");
    }
    return acc1 + acc2;
}

/* Function 3: Multiple interleaved carried dependencies */
int multi_accumulators(int n, int *restrict arr1, int *restrict arr2) {
    volatile int sum1 = 0, sum2 = 0, sum3 = 0;
    int i;
    
    for (i = 1; i < n; i++) {
        /* Three separate accumulators with different operations */
        sum1 = sum1 + arr1[i] * 2;          /* Simple accumulator */
        sum2 = sum2 * 3 + arr2[i];          /* Multiplicative accumulator */
        sum3 = arr1[i-1] + arr2[i-1] + sum3; /* Uses previous array values */
        
        /* Cross-dependencies between accumulators */
        arr1[i] = sum1 + sum2;
        arr2[i] = sum2 - sum3;
        
        /* Array access with stride creating various dependencies */
        if (i > 2) {
            arr1[i] += arr1[i-2] * 2;       /* Distance-2 dependence */
        }
        
        /* Force dependency preservation */
        asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
    }
    return sum1 + sum2 + sum3;
}

/* Function 4: Loop with unknown trip count (prevents unrolling) */
int variable_trip_loop(int n, int *restrict data, int coeff) {
    volatile int result = 0;
    int i;
    
    /* n is parameter - unknown at compile time */
    for (i = 1; i < n; i++) {
        /* Multiple carried dependencies */
        data[i] = data[i-1] * coeff + data[i];  /* Distance-1 */
        result = result + data[i] * i;           /* Accumulator */
        
        /* Complex expression tree */
        int temp = data[i] * data[i-1];
        result = result - temp / 4;
        coeff = (coeff + i) % 7;                 /* Modifies coefficient */
        
        /* Memory clobber */
        asm volatile("" : : "r"(result), "r"(coeff) : "memory");
    }
    return result;
}

/* Function 5: Software pipelining candidate with high ILP */
int high_ilp_loop(int n, int *restrict a, int *restrict b, 
                   int *restrict c, int *restrict d) {
    volatile int acc = 0;
    int i;
    
    for (i = 2; i < n; i++) {
        /* Multiple independent operations that can be pipelined */
        int t1 = a[i] * b[i-1];      /* Distance-1 on b */
        int t2 = c[i] + d[i-2];      /* Distance-2 on d */
        int t3 = a[i-1] * 3;         /* Distance-1 on a */
        
        /* Combine with carried dependency */
        acc = acc + t1 + t2 - t3;
        
        /* Update arrays creating future dependencies */
        a[i] = t1 + acc;
        b[i] = t2 - acc;
        c[i] = c[i-1] + t3;          /* Distance-1 on c */
        
        /* Force all values to be live */
        asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(acc) : "memory");
    }
    return acc;
}

/* Main driver that calls all test functions */
int main(int argc, char **argv) {
    const int N = 1000;
    const int M = 100;
    
    /* Allocate and initialize test arrays */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    int *d = (int*)malloc(N * sizeof(int));
    int *x = (int*)malloc(N * sizeof(int));
    int *y = (int*)malloc(N * sizeof(int));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N; i++) {
        a[i] = i * 3 + 7;
        b[i] = i * 5 - 2;
        c[i] = i * 11 + 1;
        d[i] = i * 13 - 5;
        x[i] = i * 17 + 3;
        y[i] = i * 19 - 7;
    }
    
    int total = 0;
    
    /* Call all test functions to ensure they're compiled */
    total += loop_carried_deps(N, a, b, c);
    total += nested_loop_deps(N/10, M, x, y);
    total += multi_accumulators(N, a, b);
    total += variable_trip_loop(N, c, 3);
    total += high_ilp_loop(N, d, a, b, c);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    free(a); free(b); free(c); free(d); free(x); free(y);
    return total != 0 ? 0 : 1;
}
