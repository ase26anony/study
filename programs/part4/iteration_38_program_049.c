/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Simple loop with distance-1 dependence and array access */
int loop_distance1(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependence */
    
    /* Loop with carried dependency: a[i] depends on a[i-1] */
    for (int i = 1; i < n; i++) {
        a[i] = a[i-1] * b[i] + c[i];
        sum += a[i];
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    return sum;
}

/* Function 2: Multiple accumulators with interleaved dependencies */
int loop_multiple_deps(int n, int *restrict x, int *restrict y, 
                       int *restrict z, int *restrict w) {
    volatile int acc1 = x[0];
    volatile int acc2 = y[0];
    
    /* Two separate distance-1 dependencies */
    for (int i = 1; i < n; i++) {
        /* First dependency chain */
        x[i] = x[i-1] * 3 + z[i];
        acc1 += x[i];
        
        /* Second dependency chain with different distance */
        y[i] = y[i-1] + w[i] * 2;
        acc2 ^= y[i];  /* XOR to create non-linear dependence */
        
        /* Force dependence between the two chains */
        asm volatile("" : : "r"(acc1), "r"(acc2) : "memory");
    }
    return acc1 + acc2;
}

/* Function 3: Nested loop with inner loop having carried dependency */
int loop_nested(int n, int m, int *restrict mat, int *restrict vec) {
    volatile int total = 0;
    
    /* Outer loop partially unrolled */
    for (int i = 0; i < n; i += 2) {
        int inner_acc = vec[0];
        
        /* Inner loop with distance-1 dependence */
        for (int j = 1; j < m; j++) {
            /* Carried dependency across inner loop iterations */
            vec[j] = vec[j-1] * mat[i * m + j] + 7;
            inner_acc += vec[j];
            
            /* Additional operation to create more ILP */
            mat[i * m + j] = (mat[i * m + j] * 3) / 2;
        }
        
        total += inner_acc;
        asm volatile("" : : "r"(total) : "memory");
    }
    return total;
}

/* Function 4: Complex loop with varying dependencies */
int loop_complex(int n, int *restrict p, int *restrict q, 
                 int *restrict r, int coeff) {
    volatile int sum1 = p[0];
    volatile int sum2 = q[0];
    
    /* Mixed dependencies: some distance-1, some distance-0 */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence on p */
        p[i] = p[i-1] * coeff + r[i];
        sum1 += p[i];
        
        /* Distance-1 dependence on q with different pattern */
        q[i] = (q[i-1] << 1) | (r[i] & 1);
        sum2 ^= q[i];
        
        /* Independent computation to increase ILP */
        r[i] = r[i] * 2 - 1;
        
        /* Complex expression with multiple uses */
        int temp = (p[i] + q[i]) * (r[i] - coeff);
        sum1 += temp & 0xFF;
        
        asm volatile("" : : "r"(sum1), "r"(sum2) : "memory");
    }
    return sum1 * 3 + sum2;
}

/* Function 5: Loop with unknown trip count (prevents unrolling) */
int loop_variable_bound(int *restrict a, int *restrict b, 
                        int start, int end, int step) {
    volatile int result = 0;
    
    /* Loop with variable bounds - harder to optimize away */
    for (int i = start; i < end; i += step) {
        if (i > 0) {
            /* Create distance-1 dependence when possible */
            a[i] = a[i-step] * 2 + b[i];
        } else {
            a[i] = b[i] * 3;
        }
        result += a[i];
        
        /* Insert memory barrier every 8 iterations */
        if ((i & 7) == 0) {
            asm volatile("" : : "r"(result) : "memory");
        }
    }
    return result;
}

/* Main driver that calls all test functions */
int main(int argc, char **argv) {
    const int N = 1024;
    const int M = 512;
    
    /* Allocate and initialize test arrays */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    int *x = (int*)malloc(N * sizeof(int));
    int *y = (int*)malloc(N * sizeof(int));
    int *z = (int*)malloc(N * sizeof(int));
    int *w = (int*)malloc(N * sizeof(int));
    int *mat = (int*)malloc(N * M * sizeof(int));
    int *vec = (int*)malloc(M * sizeof(int));
    int *p = (int*)malloc(N * sizeof(int));
    int *q = (int*)malloc(N * sizeof(int));
    int *r = (int*)malloc(N * sizeof(int));
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        x[i] = rand() % 100;
        y[i] = rand() % 100;
        z[i] = rand() % 100;
        w[i] = rand() % 100;
        p[i] = rand() % 100;
        q[i] = rand() % 100;
        r[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; i++) {
        vec[i] = rand() % 100;
    }
    
    for (int i = 0; i < N * M; i++) {
        mat[i] = rand() % 100;
    }
    
    /* Call all test functions to ensure they're compiled */
    int result = 0;
    
    result += loop_distance1(N, a, b, c);
    printf("loop_distance1 result: %d\n", result);
    
    result += loop_multiple_deps(N, x, y, z, w);
    printf("loop_multiple_deps result: %d\n", result);
    
    result += loop_nested(16, M, mat, vec);
    printf("loop_nested result: %d\n", result);
    
    result += loop_complex(N, p, q, r, 5);
    printf("loop_complex result: %d\n", result);
    
    result += loop_variable_bound(a, b, 1, N, 2);
    printf("loop_variable_bound result: %d\n", result);
    
    /* Free allocated memory */
    free(a); free(b); free(c);
    free(x); free(y); free(z); free(w);
    free(mat); free(vec);
    free(p); free(q); free(r);
    
    return result != 0 ? 0 : 1;  /* Non-zero return to indicate success */
}
