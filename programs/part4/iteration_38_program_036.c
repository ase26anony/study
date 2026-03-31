/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Simple loop with distance-1 dependence and array access */
int loop_distance1_dep(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependence */
    int i;
    
    /* Core pattern: a[i] depends on a[i-1] (distance-1) */
    for (i = 1; i < n; i++) {
        a[i] = a[i-1] * b[i] + c[i];  /* Distance-1 carried dependence */
        sum += a[i];                   /* Accumulator with distance-1 */
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    return sum;
}

/* Function 2: Multiple interleaved carried dependencies */
int loop_multiple_deps(int n, int *restrict x, int *restrict y, 
                       int *restrict z, int *restrict w) {
    volatile int acc1 = 0, acc2 = 0;
    int i;
    
    /* Two separate distance-1 dependencies */
    for (i = 1; i < n; i++) {
        x[i] = x[i-1] + y[i];      /* First distance-1 dep */
        acc1 += x[i] * 3;
        
        z[i] = z[i-1] * 2 - w[i];  /* Second distance-1 dep */
        acc2 ^= z[i];
        
        /* Complex operation mixing both */
        y[i] = (acc1 + acc2) % 100;
        
        asm volatile("" : : "r"(acc1), "r"(acc2) : "memory");
    }
    return acc1 + acc2;
}

/* Function 3: Nested loop with inner carried dependency */
int loop_nested_deps(int n, int m, int *restrict mat, int *restrict vec) {
    volatile int total = 0;
    int i, j;
    
    /* Outer loop partially unrolled */
    for (i = 0; i < n; i += 2) {
        int row_sum = 0;
        
        /* Inner loop with carried dependence */
        for (j = 1; j < m; j++) {
            mat[i*m + j] = mat[i*m + j-1] + vec[j];  /* Distance-1 */
            row_sum += mat[i*m + j] * (i + j);
            
            /* Additional operation with temporal locality */
            if (j > 1) {
                mat[i*m + j] += mat[i*m + j-2] / 4;  /* Distance-2 */
            }
        }
        
        total += row_sum;
        asm volatile("" : : "r"(total) : "memory");
    }
    return total;
}

/* Function 4: Loop with unknown trip count and complex pattern */
int loop_unknown_trip(int n, int *restrict arr1, int *restrict arr2, 
                      int *restrict arr3, int coeff) {
    volatile int result = 0;
    int i;
    
    /* Mixed distance dependencies */
    for (i = 2; i < n; i++) {
        /* Distance-1 on arr1 */
        arr1[i] = arr1[i-1] * coeff + arr2[i];
        
        /* Distance-2 on arr3 */
        arr3[i] = arr3[i-2] + arr1[i] - arr2[i-1];
        
        /* Accumulator with multiple uses */
        result = result * 2 + arr1[i] + arr3[i];
        
        /* Conditional to add complexity */
        if (i % 3 == 0) {
            result -= arr2[i-1];
        }
        
        asm volatile("" : : "r"(result) : "memory");
    }
    return result;
}

/* Function 5: Software pipelining candidate with high ILP */
int loop_high_ilp(int n, int *restrict a, int *restrict b, 
                  int *restrict c, int *restrict d) {
    volatile int s1 = 0, s2 = 0, s3 = 0;
    int i;
    
    /* Multiple independent chains with distance-1 deps */
    for (i = 1; i < n; i++) {
        /* Chain 1: a[i] depends on a[i-1] */
        a[i] = a[i-1] * 3 + b[i];
        s1 += a[i];
        
        /* Chain 2: c[i] depends on c[i-1] */
        c[i] = c[i-1] + d[i] * 2;
        s2 ^= c[i];
        
        /* Chain 3: cross-chain dependency */
        b[i] = a[i] + c[i-1];  /* Distance-1 from c */
        s3 = s3 * 5 + b[i];
        
        /* Final reduction with all chains */
        d[i] = s1 + s2 + s3;
        
        asm volatile("" : : "r"(s1), "r"(s2), "r"(s3) : "memory");
    }
    return s1 + s2 + s3;
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int n, int *a, int *b, int *c, int *d) {
    for (int i = 0; i < n; i++) {
        a[i] = (i * 13) % 97;
        b[i] = (i * 17) % 101;
        c[i] = (i * 19) % 103;
        d[i] = (i * 23) % 107;
    }
}

int main(int argc, char *argv[]) {
    const int N = 1000;
    const int M = 100;
    
    /* Allocate and initialize arrays */
    int *a = malloc(N * sizeof(int));
    int *b = malloc(N * sizeof(int));
    int *c = malloc(N * sizeof(int));
    int *d = malloc(N * sizeof(int));
    int *mat = malloc(N * M * sizeof(int));
    int *vec = malloc(M * sizeof(int));
    
    if (!a || !b || !c || !d || !mat || !vec) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with deterministic values */
    init_arrays(N, a, b, c, d);
    for (int i = 0; i < M; i++) vec[i] = i * 7;
    for (int i = 0; i < N * M; i++) mat[i] = i % 53;
    
    int result = 0;
    
    /* Call all loop functions to ensure they're compiled */
    result += loop_distance1_dep(N, a, b, c, d);
    result += loop_multiple_deps(N, a, b, c, d);
    result += loop_nested_deps(10, M, mat, vec);
    result += loop_unknown_trip(N, a, b, c, 3);
    result += loop_high_ilp(N, a, b, c, d);
    
    /* Use result to prevent dead code elimination */
    printf("Final checksum: %d\n", result);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(mat); free(vec);
    
    return 0;
}
