/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Loop with multiple carried dependencies and array accesses */
int loop_carried_deps(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependency */
    int i;
    
    /* Core pattern: a[i] depends on a[i-1] (distance-1 dependence) */
    a[0] = b[0] + c[0];
    for (i = 1; i < n; i++) {
        /* Multiple operations with carried dependencies */
        a[i] = a[i-1] * b[i] + c[i];  /* Distance-1 dependence on a[i-1] */
        sum += a[i];                   /* Accumulator with carried dependency */
        
        /* Additional arithmetic to create instruction-level parallelism */
        b[i] = b[i-1] + i;             /* Another distance-1 dependence */
        c[i] = c[i-1] * 2 - i;         /* Yet another distance-1 dependence */
        
        /* Memory barrier to prevent over-optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    return sum;
}

/* Function 2: Nested loops with inner loop carried dependency */
int nested_loop_deps(int n, int m, int *restrict x, int *restrict y) {
    volatile int total = 0;
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Initialize with dependency */
        x[i*m] = y[i*m] + i;
        
        for (j = 1; j < m; j++) {
            /* Inner loop with carried dependency across j */
            x[i*m + j] = x[i*m + j-1] * y[i*m + j] + (i + j);
            total += x[i*m + j];
            
            /* Cross-iteration dependency in y */
            y[i*m + j] = y[i*m + j-1] + x[i*m + j];
            
            asm volatile("" : : "r"(total) : "memory");
        }
    }
    return total;
}

/* Function 3: Multiple interleaved accumulators with complex dependencies */
int multi_accumulator(int n, int *restrict arr1, int *restrict arr2) {
    volatile int sum1 = 0, sum2 = 0, sum3 = 0;
    int i;
    
    /* Initialize first elements with dependencies */
    arr1[0] = 1;
    arr2[0] = 2;
    
    for (i = 1; i < n; i++) {
        /* Pattern 1: Linear recurrence */
        arr1[i] = arr1[i-1] * 3 + arr2[i-1];
        sum1 += arr1[i];
        
        /* Pattern 2: Another recurrence */
        arr2[i] = arr2[i-1] + arr1[i-1] * 2;
        sum2 += arr2[i];
        
        /* Pattern 3: Mixed dependency */
        int temp = arr1[i] - arr2[i];
        sum3 += temp * i;
        
        /* Complex expression with multiple uses */
        arr1[i] = (arr1[i] + arr2[i]) * (i % 7);
        arr2[i] = (arr2[i] - arr1[i]) / ((i % 5) + 1);
        
        /* Force dependency preservation */
        asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
    }
    
    return sum1 + sum2 + sum3;
}

/* Function 4: Loop with unknown trip count (prevents unrolling) */
int variable_loop(int n, int *restrict data, int coeff) {
    volatile int result = 0;
    int i;
    
    /* Initialize with simple value */
    data[0] = coeff;
    
    for (i = 1; i < n; i++) {
        /* Strong carried dependency */
        data[i] = data[i-1] * coeff + i;
        
        /* Multiple operations on the result */
        result += data[i];
        result *= (i % 3) + 1;
        result -= data[i-1];
        
        /* Additional dependency chain */
        coeff = (coeff + data[i]) % 17;
        
        asm volatile("" : : "r"(result) : "memory");
    }
    
    return result;
}

/* Function 5: Software pipelining candidate with high ILP */
int high_ilp_loop(int n, int *restrict A, int *restrict B, int *restrict C) {
    volatile int acc1 = 0, acc2 = 0;
    int i;
    
    A[0] = B[0] = C[0] = 1;
    
    for (i = 1; i < n; i++) {
        /* Independent operations that can be parallelized */
        int t1 = A[i-1] * 7;
        int t2 = B[i-1] + 11;
        int t3 = C[i-1] * 3;
        
        /* Operations with dependencies */
        A[i] = t1 + t2;
        B[i] = t2 * t3;
        C[i] = A[i] + B[i];
        
        /* Multiple accumulators with dependencies */
        acc1 += A[i] * i;
        acc2 += B[i] * (n - i);
        
        /* Cross-iteration mixing */
        A[i] = (A[i] + acc1) % 1024;
        B[i] = (B[i] + acc2) % 1024;
        
        asm volatile("" : : "r"(acc1), "r"(acc2) : "memory");
    }
    
    return acc1 + acc2;
}

/* Main driver that exercises all patterns */
int main(int argc, char *argv[]) {
    const int N = 1000;
    const int M = 100;
    int *a, *b, *c, *x, *y, *arr1, *arr2, *data, *A, *B, *C;
    int result = 0;
    
    /* Allocate and initialize arrays */
    a = (int*)malloc(N * sizeof(int));
    b = (int*)malloc(N * sizeof(int));
    c = (int*)malloc(N * sizeof(int));
    x = (int*)malloc(N * M * sizeof(int));
    y = (int*)malloc(N * M * sizeof(int));
    arr1 = (int*)malloc(N * sizeof(int));
    arr2 = (int*)malloc(N * sizeof(int));
    data = (int*)malloc(N * sizeof(int));
    A = (int*)malloc(N * sizeof(int));
    B = (int*)malloc(N * sizeof(int));
    C = (int*)malloc(N * sizeof(int));
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        data[i] = rand() % 100;
        A[i] = rand() % 100;
        B[i] = rand() % 100;
        C[i] = rand() % 100;
    }
    for (int i = 0; i < N * M; i++) {
        x[i] = rand() % 100;
        y[i] = rand() % 100;
    }
    
    /* Call all test functions with different parameters */
    result += loop_carried_deps(N, a, b, c);
    result += nested_loop_deps(10, M, x, y);
    result += multi_accumulator(N, arr1, arr2);
    result += variable_loop(N, data, 7);
    result += high_ilp_loop(N, A, B, C);
    
    /* Print result to prevent dead code elimination */
    printf("Final checksum: %d\n", result);
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(x); free(y);
    free(arr1); free(arr2);
    free(data);
    free(A); free(B); free(C);
    
    return 0;
}
