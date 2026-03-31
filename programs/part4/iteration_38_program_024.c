/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Loop with multiple carried dependencies and array accesses */
int loop_carried_deps(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependencies */
    int i;
    
    /* Core pattern: a[i] depends on a[i-1] (distance-1 dependence) */
    a[0] = b[0] + c[0];
    for (i = 1; i < n; i++) {
        /* Multiple carried dependencies to create complex dependence graph */
        a[i] = a[i-1] * b[i] + c[i];      /* Distance-1 dependence on a[i-1] */
        sum = sum + a[i];                  /* Accumulator pattern */
        
        /* Additional operation with another carried dependency */
        b[i] = b[i-1] + sum;               /* Another distance-1 dependence */
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    return sum;
}

/* Function 2: Nested loop with inner loop having carried dependency */
int nested_loop_carried(int n, int m, int *restrict x, int *restrict y) {
    volatile int acc1 = 0, acc2 = 0;
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Outer loop with some computation */
        acc1 = acc1 + x[i];
        
        /* Inner loop with carried dependency */
        y[0] = x[i] * 2;
        for (j = 1; j < m; j++) {
            /* Distance-1 dependence in inner loop */
            y[j] = y[j-1] + x[i] * j + acc1;
            acc2 = acc2 + y[j];
            
            /* Create complex dependence web */
            if (j > 1) {
                y[j] = y[j] + y[j-2] / 4;  /* Distance-2 dependence */
            }
        }
        
        /* Prevent dead code elimination */
        asm volatile("" : : "r"(acc1), "r"(acc2) : "memory");
    }
    
    return acc1 + acc2;
}

/* Function 3: Multiple interleaved accumulators with unknown trip count */
int multi_accumulators(int n, int *restrict data, int coeff1, int coeff2) {
    volatile int sum1 = 0, sum2 = 0, sum3 = 0;
    int i;
    
    /* Unknown trip count prevents complete unrolling */
    for (i = 0; i < n; i++) {
        /* Three separate accumulators with different operations */
        sum1 = sum1 + data[i] * coeff1;           /* Simple accumulator */
        sum2 = sum2 + sum1 * coeff2;              /* Depends on sum1 */
        sum3 = sum3 + (sum1 + sum2) * data[i];    /* Depends on both */
        
        /* Array access with stride-1 pattern */
        if (i > 0) {
            data[i] = data[i-1] + sum3;           /* Distance-1 dependence */
        }
        
        /* Complex expression to create multiple dependence edges */
        coeff1 = (coeff1 * 13 + 7) & 0xFF;
        coeff2 = (coeff2 * 17 + 11) & 0xFF;
        
        /* Memory clobber to preserve all dependencies */
        asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
    }
    
    return sum1 + sum2 + sum3;
}

/* Function 4: Loop with if-converted dependencies */
int conditional_carried(int n, int *restrict arr, int threshold) {
    volatile int count = 0;
    int i;
    
    arr[0] = 1;
    for (i = 1; i < n; i++) {
        /* Conditional update with carried dependency */
        if (arr[i-1] > threshold) {
            arr[i] = arr[i-1] / 2;      /* Distance-1 dependence in true path */
            count = count + 1;
        } else {
            arr[i] = arr[i-1] * 3 + 1;  /* Distance-1 dependence in false path */
            count = count - 1;
        }
        
        /* Additional operation that depends on the conditional result */
        arr[i] = arr[i] + count * i;
        
        /* Force dependence preservation */
        asm volatile("" : : "r"(count), "r"(arr[i]) : "memory");
    }
    
    return count;
}

/* Function 5: Reduction with multiple dependency types */
int complex_reduction(int n, int *restrict src1, int *restrict src2) {
    volatile int prod_sum = 0;
    volatile int diff_sum = 0;
    int i;
    
    /* Initialize first elements */
    src1[0] = src2[0] * 2;
    
    for (i = 1; i < n; i++) {
        /* Multiple carried dependencies in parallel */
        int temp1 = src1[i-1] * src2[i];      /* Distance-1 on src1 */
        int temp2 = src2[i-1] + src1[i];      /* Distance-1 on src2 */
        
        src1[i] = temp1 + i;
        src2[i] = temp2 - i;
        
        /* Two separate reductions */
        prod_sum = prod_sum + src1[i] * src2[i];
        diff_sum = diff_sum + (src1[i] - src2[i]);
        
        /* Cross-iteration dependency through reduction variables */
        if (i % 4 == 0) {
            src1[i] = src1[i] + prod_sum;
            src2[i] = src2[i] + diff_sum;
        }
        
        /* Prevent optimization */
        asm volatile("" : : "r"(prod_sum), "r"(diff_sum) : "memory");
    }
    
    return prod_sum + diff_sum;
}

/* Main driver that calls all test functions */
int main(int argc, char **argv) {
    const int N = 1000;
    const int M = 100;
    int i, result = 0;
    
    /* Allocate and initialize arrays */
    int *a = (int *)malloc(N * sizeof(int));
    int *b = (int *)malloc(N * sizeof(int));
    int *c = (int *)malloc(N * sizeof(int));
    int *x = (int *)malloc(N * sizeof(int));
    int *y = (int *)malloc(M * sizeof(int));
    int *data = (int *)malloc(N * sizeof(int));
    int *arr = (int *)malloc(N * sizeof(int));
    int *src1 = (int *)malloc(N * sizeof(int));
    int *src2 = (int *)malloc(N * sizeof(int));
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (i = 0; i < N; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        x[i] = rand() % 100;
        data[i] = rand() % 100;
        arr[i] = rand() % 100;
        src1[i] = rand() % 100;
        src2[i] = rand() % 100;
    }
    for (i = 0; i < M; i++) {
        y[i] = rand() % 100;
    }
    
    /* Call all test functions to ensure they're compiled */
    result += loop_carried_deps(N, a, b, c);
    result += nested_loop_carried(N/10, M, x, y);
    result += multi_accumulators(N, data, 3, 7);
    result += conditional_carried(N, arr, 50);
    result += complex_reduction(N, src1, src2);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(a); free(b); free(c); free(x); free(y);
    free(data); free(arr); free(src1); free(src2);
    
    return 0;
}
