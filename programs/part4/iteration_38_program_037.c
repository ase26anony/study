/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Simple loop with distance-1 dependencies */
void loop_distance1_deps(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int acc = 0;  /* volatile to preserve dependencies */
    
    /* Loop with multiple carried dependencies */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence: a[i] depends on a[i-1] */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Another distance-1 dependence with accumulator */
        acc = acc + a[i] * 3;
        
        /* Cross-iteration dependency with stride 1 */
        b[i] = b[i-1] + a[i] * 2;
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : "r"(acc) : "memory");
    }
    
    /* Use result to prevent dead code elimination */
    a[0] = acc;
}

/* Function 2: Multiple interleaved accumulators */
void loop_multiple_accumulators(int n, int *restrict x, int *restrict y, int *restrict z) {
    volatile int sum1 = 0;
    volatile int sum2 = 0;
    volatile int sum3 = 0;
    
    for (int i = 1; i < n; i++) {
        /* Three separate carried dependencies */
        sum1 = sum1 + x[i] * y[i];
        sum2 = sum2 + y[i] * z[i];
        sum3 = sum3 + z[i] * x[i];
        
        /* Cross-iteration array dependencies */
        x[i] = x[i-1] + sum1;
        y[i] = y[i-1] + sum2;
        z[i] = z[i-1] + sum3;
        
        /* Force dependence preservation */
        asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
    }
    
    /* Combine results */
    x[0] = sum1 + sum2 + sum3;
}

/* Function 3: Nested loop with inner carried dependency */
void loop_nested_carried_dep(int n, int m, int *restrict mat, int *restrict vec) {
    volatile int inner_acc = 0;
    
    /* Outer loop - may be unrolled */
    for (int i = 0; i < n; i++) {
        inner_acc = 0;
        
        /* Inner loop with carried dependency */
        for (int j = 1; j < m; j++) {
            /* Distance-1 dependence in inner loop */
            mat[i*m + j] = mat[i*m + j-1] * vec[j] + inner_acc;
            
            /* Accumulator with carried dependency */
            inner_acc = inner_acc + mat[i*m + j] * 5;
            
            /* Another distance-1 dependence */
            vec[j] = vec[j-1] + mat[i*m + j] * 2;
        }
        
        /* Prevent optimization */
        asm volatile("" : : "r"(inner_acc) : "memory");
    }
}

/* Function 4: Complex loop with unknown trip count (parameter) */
int loop_unknown_trip_count(int n, int *restrict data, int coeff) {
    volatile int result = 0;
    volatile int temp = data[0];
    
    /* Loop with multiple interleaved distance-1 dependencies */
    for (int i = 1; i < n; i++) {
        /* Chain of dependencies */
        int t1 = temp * coeff + result;
        int t2 = data[i] * t1;
        result = result + t2;
        temp = data[i] + t1;
        data[i] = t2 + temp;
        
        /* Force all dependencies to be preserved */
        asm volatile("" : : "r"(t1), "r"(t2), "r"(result), "r"(temp) : "memory");
    }
    
    return result;
}

/* Function 5: Loop with if-conversion opportunities */
void loop_with_conditionals(int n, int *restrict a, int *restrict b, int threshold) {
    volatile int sum_pos = 0;
    volatile int sum_neg = 0;
    
    for (int i = 1; i < n; i++) {
        /* Carried dependency */
        int diff = a[i] - a[i-1];
        
        /* Conditional that creates complex dependence graph */
        if (diff > threshold) {
            sum_pos = sum_pos + diff * b[i];
            a[i] = a[i-1] + b[i];
        } else {
            sum_neg = sum_neg + diff * b[i-1];
            a[i] = a[i-1] - b[i];
        }
        
        /* Another carried dependency */
        b[i] = b[i-1] + (sum_pos - sum_neg);
        
        /* Memory barrier */
        asm volatile("" : : "r"(sum_pos), "r"(sum_neg) : "memory");
    }
    
    a[0] = sum_pos + sum_neg;
}

/* Main driver function */
int main(int argc, char **argv) {
    const int N = 1024;
    const int M = 512;
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    int *x = (int*)malloc(N * sizeof(int));
    int *y = (int*)malloc(N * sizeof(int));
    int *z = (int*)malloc(N * sizeof(int));
    int *mat = (int*)malloc(N * M * sizeof(int));
    int *vec = (int*)malloc(M * sizeof(int));
    int *data = (int*)malloc(N * sizeof(int));
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        x[i] = rand() % 100;
        y[i] = rand() % 100;
        z[i] = rand() % 100;
        data[i] = rand() % 100;
    }
    
    for (int i = 0; i < N * M; i++) {
        mat[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; i++) {
        vec[i] = rand() % 100;
    }
    
    /* Call all test functions to ensure they're compiled */
    loop_distance1_deps(N, a, b, c);
    loop_multiple_accumulators(N, x, y, z);
    loop_nested_carried_dep(10, M, mat, vec);
    int result1 = loop_unknown_trip_count(N, data, 3);
    loop_with_conditionals(N, a, b, 50);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += a[i] + b[i] + c[i] + x[i] + y[i] + z[i] + data[i];
    }
    
    for (int i = 0; i < N * M; i += 97) {
        checksum += mat[i];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Result from unknown trip count: %d\n", result1);
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(x); free(y); free(z);
    free(mat); free(vec);
    free(data);
    
    return 0;
}
