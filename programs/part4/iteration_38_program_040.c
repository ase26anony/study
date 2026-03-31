/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Simple loop with distance-1 dependence and array access */
int loop_distance1_dep(int n, int* restrict a, int* restrict b, int* restrict c) {
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

/* Function 2: Multiple interleaved carried dependencies */
int loop_multiple_deps(int n, int* restrict x, int* restrict y, int* restrict z) {
    volatile int acc1 = x[0];
    volatile int acc2 = y[0];
    
    /* Two separate accumulators with distance-1 dependencies */
    for (int i = 1; i < n; i++) {
        /* First dependency chain */
        acc1 = acc1 * 3 + x[i];
        
        /* Second dependency chain with different operation */
        acc2 = acc2 + y[i-1] * z[i];
        
        /* Cross-dependency between chains */
        x[i] = acc1 + acc2;
        y[i] = acc1 - acc2;
        
        /* Force dependence preservation */
        asm volatile("" : : "r"(acc1), "r"(acc2) : "memory");
    }
    return acc1 + acc2;
}

/* Function 3: Nested loop with inner carried dependency */
int loop_nested_deps(int n, int m, int* restrict mat, int* restrict vec) {
    volatile int total = 0;
    
    /* Outer loop - unrolled by compiler */
    for (int i = 0; i < n; i += 2) {
        int row_sum = 0;
        
        /* Inner loop with carried dependency */
        for (int j = 1; j < m; j++) {
            /* Distance-1 dependence: uses previous iteration's value */
            row_sum = row_sum + mat[i * m + j-1] * vec[j];
            mat[i * m + j] = row_sum;
            
            /* Another distance-1 dependence */
            if (j > 1) {
                mat[i * m + j] += mat[i * m + j-2] / 2;
            }
        }
        total += row_sum;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(row_sum) : "memory");
    }
    return total;
}

/* Function 4: Complex recurrence with multiple operations */
int loop_complex_recurrence(int n, int* restrict p, int* restrict q) {
    volatile int a = p[0];
    volatile int b = q[0];
    volatile int c = p[0] + q[0];
    
    /* Multiple inter-dependent recurrence relations */
    for (int i = 1; i < n; i++) {
        /* Three separate but interdependent chains */
        int new_a = a * 2 + p[i];
        int new_b = b + a * q[i];      /* Uses 'a' from previous iteration */
        int new_c = c + new_a - new_b; /* Uses newly computed values */
        
        /* Update with memory barrier */
        a = new_a;
        b = new_b;
        c = new_c;
        
        /* Store results creating more dependencies */
        p[i] = a + b;
        q[i] = b - c;
        
        /* Force all dependencies to be preserved */
        asm volatile("" : : "r"(a), "r"(b), "r"(c) : "memory");
    }
    return a + b + c;
}

/* Function 5: Loop with runtime-dependent trip count */
int loop_variable_trip(int start, int end, int* restrict data) {
    volatile int result = data[start];
    
    /* Loop count not known at compile time */
    for (int i = start + 1; i < end; i++) {
        /* Strong distance-1 dependence */
        result = result * 7 + data[i] * 3;
        data[i] = result;
        
        /* Additional operation with its own dependence */
        if (i > start + 1) {
            data[i] += data[i-2] / 4;
        }
        
        /* Memory clobber to preserve all dependencies */
        asm volatile("" : : "r"(result) : "memory");
    }
    return result;
}

/* Main driver that calls all test functions */
int main(int argc, char** argv) {
    const int N = 1000;
    const int M = 100;
    
    /* Allocate and initialize test arrays */
    int* a = (int*)malloc(N * sizeof(int));
    int* b = (int*)malloc(N * sizeof(int));
    int* c = (int*)malloc(N * sizeof(int));
    int* x = (int*)malloc(N * sizeof(int));
    int* y = (int*)malloc(N * sizeof(int));
    int* z = (int*)malloc(N * sizeof(int));
    int* mat = (int*)malloc(N * M * sizeof(int));
    int* vec = (int*)malloc(M * sizeof(int));
    int* p = (int*)malloc(N * sizeof(int));
    int* q = (int*)malloc(N * sizeof(int));
    int* data = (int*)malloc(N * sizeof(int));
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        x[i] = rand() % 100;
        y[i] = rand() % 100;
        z[i] = rand() % 100;
        p[i] = rand() % 100;
        q[i] = rand() % 100;
        data[i] = rand() % 100;
    }
    for (int i = 0; i < M; i++) {
        vec[i] = rand() % 100;
    }
    for (int i = 0; i < N * M; i++) {
        mat[i] = rand() % 100;
    }
    
    /* Call all test functions to ensure they're compiled */
    int result = 0;
    
    result += loop_distance1_dep(N, a, b, c);
    printf("Test 1 result: %d\n", result);
    
    result += loop_multiple_deps(N, x, y, z);
    printf("Test 2 result: %d\n", result);
    
    result += loop_nested_deps(10, M, mat, vec);
    printf("Test 3 result: %d\n", result);
    
    result += loop_complex_recurrence(N, p, q);
    printf("Test 4 result: %d\n", result);
    
    result += loop_variable_trip(0, N, data);
    printf("Test 5 result: %d\n", result);
    
    /* Final checksum to prevent dead code elimination */
    printf("Final checksum: %d\n", result);
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(x); free(y); free(z);
    free(mat); free(vec);
    free(p); free(q);
    free(data);
    
    return 0;
}
