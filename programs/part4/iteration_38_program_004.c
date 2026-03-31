/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Simple loop with carried dependency and array access */
int loop_carried_dependency(int n, int* restrict a, int* restrict b, int* restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependency */
    
    /* Loop with distance-1 dependence: a[i] depends on a[i-1] */
    for (int i = 1; i < n; i++) {
        a[i] = a[i-1] * b[i] + c[i];
        sum += a[i];
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    return sum;
}

/* Function 2: Multiple accumulators with interleaved dependencies */
int multiple_accumulators(int n, int* restrict x, int* restrict y, int* restrict z) {
    volatile int acc1 = x[0];
    volatile int acc2 = y[0];
    
    /* Two separate carried dependencies */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence on x */
        x[i] = x[i-1] + y[i] * z[i];
        acc1 += x[i];
        
        /* Another distance-1 dependence on y */
        y[i] = y[i-1] - x[i] + z[i];
        acc2 += y[i];
        
        /* Force dependencies to be preserved */
        asm volatile("" : : "r"(acc1), "r"(acc2) : "memory");
    }
    return acc1 + acc2;
}

/* Function 3: Nested loop with inner carried dependency */
int nested_loop_carried(int n, int m, int* restrict mat, int* restrict vec) {
    volatile int total = 0;
    
    for (int i = 0; i < n; i++) {
        int row_start = i * m;
        volatile int row_sum = mat[row_start];
        
        /* Inner loop with carried dependency */
        for (int j = 1; j < m; j++) {
            /* Distance-1 dependence within the row */
            mat[row_start + j] = mat[row_start + j-1] * vec[j] + i;
            row_sum += mat[row_start + j];
            
            /* Prevent optimization */
            asm volatile("" : : "r"(row_sum) : "memory");
        }
        total += row_sum;
    }
    return total;
}

/* Function 4: Complex loop with multiple uses of previous iteration values */
int complex_distance1_uses(int n, int* restrict p, int* restrict q, 
                          int* restrict r, int* restrict s) {
    volatile int result = p[0] + q[0];
    
    /* Multiple distance-1 dependencies in one loop */
    for (int i = 1; i < n; i++) {
        /* p[i] depends on p[i-1] AND q[i-1] */
        p[i] = (p[i-1] * q[i-1]) + r[i];
        
        /* q[i] depends on p[i-1] */
        q[i] = p[i-1] - s[i];
        
        /* r[i] depends on r[i-1] */
        r[i] = r[i-1] + p[i] * q[i];
        
        result += p[i] + q[i] + r[i];
        
        /* Strong memory barrier to ensure all dependencies are visible */
        asm volatile("" : : "r"(p[i]), "r"(q[i]), "r"(r[i]), "r"(result) : "memory");
    }
    return result;
}

/* Function 5: Loop with unknown trip count (prevents unrolling) */
int variable_trip_count(int start, int end, int* restrict data, int* restrict coeff) {
    volatile int acc = 0;
    
    /* Loop count not known at compile time - forces modulo scheduling analysis */
    for (int i = start; i < end; i++) {
        /* Classic accumulator with distance-1 dependence */
        acc = acc + data[i] * coeff[i % 16];
        
        /* Array access with distance-1 dependence */
        if (i > 0) {
            data[i] = data[i-1] + acc;
        }
        
        /* Prevent optimization */
        asm volatile("" : : "r"(acc) : "memory");
    }
    return acc;
}

/* Main driver that calls all test functions */
int main(int argc, char** argv) {
    const int N = 1024;
    const int M = 64;
    
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
    int* r = (int*)malloc(N * sizeof(int));
    int* s = (int*)malloc(N * sizeof(int));
    int* data = (int*)malloc(N * sizeof(int));
    int* coeff = (int*)malloc(16 * sizeof(int));
    
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
        r[i] = rand() % 100;
        s[i] = rand() % 100;
        data[i] = rand() % 100;
    }
    
    for (int i = 0; i < N * M; i++) {
        mat[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; i++) {
        vec[i] = rand() % 100;
    }
    
    for (int i = 0; i < 16; i++) {
        coeff[i] = rand() % 10;
    }
    
    /* Call all test functions to ensure they're compiled */
    int result = 0;
    
    result += loop_carried_dependency(N, a, b, c);
    result += multiple_accumulators(N, x, y, z);
    result += nested_loop_carried(16, M, mat, vec);
    result += complex_distance1_uses(N, p, q, r, s);
    result += variable_trip_count(1, N, data, coeff);
    
    /* Print result to prevent dead code elimination */
    printf("Final checksum: %d\n", result);
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(x); free(y); free(z);
    free(mat); free(vec);
    free(p); free(q); free(r); free(s);
    free(data); free(coeff);
    
    return 0;
}
