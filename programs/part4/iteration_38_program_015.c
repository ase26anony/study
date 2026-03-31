/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Simple loop with carried dependency and array access */
int func1(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependency */
    int i;
    
    /* Loop with distance-1 dependence: a[i] depends on a[i-1] */
    for (i = 1; i < n; i++) {
        a[i] = a[i-1] * b[i] + c[i];
        sum += a[i];
        /* Memory barrier to prevent optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    return sum;
}

/* Function 2: Multiple accumulators with interleaved dependencies */
int func2(int n, int *restrict x, int *restrict y, int *restrict z) {
    volatile int acc1 = x[0];
    volatile int acc2 = y[0];
    int i;
    
    /* Two separate distance-1 dependencies */
    for (i = 1; i < n; i++) {
        /* First dependency chain */
        x[i] = x[i-1] + y[i] * 3;
        acc1 += x[i];
        
        /* Second dependency chain with different distance */
        y[i] = y[i-1] - z[i];
        acc2 ^= y[i];  /* XOR to create non-linear dependency */
        
        /* Force dependency preservation */
        asm volatile("" : : "r"(acc1), "r"(acc2) : "memory");
    }
    return acc1 + acc2;
}

/* Function 3: Nested loop with inner carried dependency */
int func3(int n, int m, int *restrict mat, int *restrict vec) {
    volatile int total = 0;
    int i, j;
    
    /* Outer loop partially unrolled */
    for (i = 0; i < n; i += 2) {
        int row_sum = 0;
        /* Inner loop with carried dependency */
        for (j = 1; j < m; j++) {
            mat[i*m + j] = mat[i*m + j-1] * vec[j] + i;
            row_sum += mat[i*m + j];
        }
        total += row_sum;
        asm volatile("" : : "r"(total) : "memory");
    }
    return total;
}

/* Function 4: Complex loop with multiple uses of previous iteration values */
int func4(int n, int *restrict p, int *restrict q, int *restrict r) {
    volatile int s1 = p[0];
    volatile int s2 = q[0];
    int i;
    
    /* Multiple distance-1 uses in same iteration */
    for (i = 1; i < n; i++) {
        /* p[i] depends on p[i-1] AND q[i-1] */
        p[i] = (p[i-1] * 2 + q[i-1]) / 3;
        
        /* q[i] depends on previous p[i-1] */
        q[i] = p[i-1] + r[i] * 7;
        
        /* r[i] has self-dependency */
        r[i] = r[i-1] ^ (i * 11);
        
        s1 += p[i];
        s2 += q[i] - r[i];
        
        /* Critical: Force all dependencies to be preserved */
        asm volatile("" : : "r"(s1), "r"(s2), "r"(r[i]) : "memory");
    }
    return s1 * 3 + s2;
}

/* Function 5: Loop with variable trip count (unknown at compile time) */
int func5(int start, int end, int step, int *restrict data) {
    volatile int accum = 0;
    int i;
    
    /* Loop with non-constant bounds creates scheduling challenge */
    for (i = start + step; i < end; i += step) {
        /* Strong carried dependency */
        data[i] = data[i - step] * data[i] + i;
        accum += data[i] * (i % 7);
        
        /* Prevent optimization */
        if (accum & 1) {
            asm volatile("" : : "r"(accum) : "memory");
        }
    }
    return accum;
}

/* Main driver that calls all functions */
int main(int argc, char **argv) {
    const int N = 1000;
    const int M = 100;
    int i, result = 0;
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    int *x = (int*)malloc(N * sizeof(int));
    int *y = (int*)malloc(N * sizeof(int));
    int *z = (int*)malloc(N * sizeof(int));
    int *mat = (int*)malloc(N * M * sizeof(int));
    int *vec = (int*)malloc(M * sizeof(int));
    int *p = (int*)malloc(N * sizeof(int));
    int *q = (int*)malloc(N * sizeof(int));
    int *r = (int*)malloc(N * sizeof(int));
    int *data = (int*)malloc(N * sizeof(int));
    
    /* Initialize with pseudo-random but deterministic values */
    srand(42);
    for (i = 0; i < N; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        x[i] = rand() % 100;
        y[i] = rand() % 100;
        z[i] = rand() % 100;
        p[i] = rand() % 100;
        q[i] = rand() % 100;
        r[i] = rand() % 100;
        data[i] = rand() % 100;
    }
    for (i = 0; i < M; i++) {
        vec[i] = rand() % 100;
    }
    for (i = 0; i < N * M; i++) {
        mat[i] = rand() % 100;
    }
    
    /* Call all functions to ensure they're compiled */
    result += func1(N, a, b, c);
    result += func2(N, x, y, z);
    result += func3(10, M, mat, vec);
    result += func4(N, p, q, r);
    result += func5(1, N, 2, data);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(x); free(y); free(z);
    free(mat); free(vec);
    free(p); free(q); free(r);
    free(data);
    
    return result != 0 ? 0 : 1;
}
