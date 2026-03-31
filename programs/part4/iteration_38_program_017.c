/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>

/* Function with simple loop-carried dependency */
void loop_carried_dep(int n, int *a, int *b, int *c) {
    volatile int acc = 0;  /* volatile prevents optimization */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence: a[i] depends on a[i-1] */
        a[i] = a[i-1] * b[i] + c[i];
        /* Another distance-1 dependence with accumulator */
        acc = acc + b[i] * 3;
        /* Memory barrier to preserve dependencies */
        asm volatile("" : : "r"(acc) : "memory");
    }
    /* Use result to prevent dead code elimination */
    a[0] = acc;
}

/* Function with multiple interleaved carried dependencies */
void multiple_deps(int n, int *x, int *y, int *z) {
    volatile int sum1 = 0, sum2 = 0;
    
    for (int i = 1; i < n; i++) {
        /* Two separate distance-1 dependencies */
        x[i] = y[i-1] + z[i];      /* Distance-1 from y[i-1] */
        sum1 = sum1 + x[i] * 2;    /* Distance-1 accumulator */
        sum2 = sum2 * 3 + z[i];    /* Another distance-1 accumulator */
        
        /* Cross-iteration dependency with computation */
        y[i] = (x[i] + x[i-1]) / 2;
        
        /* Memory clobber to prevent reordering */
        asm volatile("" : : "r"(sum1), "r"(sum2) : "memory");
    }
    
    /* Use results */
    x[0] = sum1 + sum2;
}

/* Function with nested loops - inner loop has dependencies */
void nested_loops(int n, int m, int *mat, int *vec) {
    volatile int total = 0;
    
    for (int i = 0; i < n; i++) {
        int row_start = i * m;
        int prev_row_start = (i-1) * m;
        
        for (int j = 1; j < m; j++) {
            /* Multiple dependencies: from previous element and previous row */
            int val = mat[row_start + j-1] * 2;
            if (i > 0) {
                val += mat[prev_row_start + j] * 3;  /* Distance in outer loop */
            }
            mat[row_start + j] = val + vec[j];
            
            /* Accumulator with distance-1 in inner loop */
            total = total + mat[row_start + j];
            
            /* Dependency chain within inner loop */
            vec[j] = vec[j-1] + 1;
        }
        
        /* Prevent optimization */
        asm volatile("" : : "r"(total) : "memory");
    }
    
    mat[0] = total;
}

/* Function with unknown trip count (parameter) to force modulo scheduling */
void variable_trip_count(int n, int *data, int *coeff) {
    volatile int result = 0;
    
    /* Complex loop with multiple distance-1 dependencies */
    for (int i = 2; i < n; i++) {
        /* Three different distance-1 patterns */
        int temp = data[i-2] * coeff[i-1] + data[i-1];  /* Uses i-2 and i-1 */
        data[i] = temp * 7 - coeff[i];
        
        /* Accumulator pattern - classic distance-1 */
        result = result + data[i] * coeff[i];
        
        /* Another dependency chain */
        coeff[i] = coeff[i-1] + i;
        
        /* Force dependency preservation */
        asm volatile("" : : "r"(result), "r"(temp) : "memory");
    }
    
    data[0] = result;
}

/* Function with unrolled outer loop containing dependent inner loop */
void unrolled_with_deps(int n, int *arr1, int *arr2) {
    volatile int sum = 0;
    
    /* Outer loop with small unroll factor */
    for (int i = 0; i < n; i += 4) {
        /* Process 4 elements with carried dependencies */
        for (int j = 0; j < 4 && (i + j) < n; j++) {
            int idx = i + j;
            if (idx > 0) {
                /* Distance-1 dependency within the unrolled block */
                arr1[idx] = arr1[idx-1] + arr2[idx] * (j + 1);
            }
            sum += arr1[idx];
            
            /* Create anti-dependencies */
            arr2[idx] = arr2[idx] ^ sum;
        }
        
        /* Memory barrier between unrolled iterations */
        asm volatile("" : : "r"(sum) : "memory");
    }
    
    arr1[0] = sum;
}

/* Main driver that calls all test functions */
int main(int argc, char **argv) {
    const int N = 1000;
    const int M = 100;
    
    /* Allocate and initialize test arrays */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    int *x = (int*)malloc(N * sizeof(int));
    int *y = (int*)malloc(N * sizeof(int));
    int *z = (int*)malloc(N * sizeof(int));
    int *mat = (int*)malloc(N * M * sizeof(int));
    int *vec = (int*)malloc(M * sizeof(int));
    int *data = (int*)malloc(N * sizeof(int));
    int *coeff = (int*)malloc(N * sizeof(int));
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(N * sizeof(int));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N; i++) {
        a[i] = i * 3;
        b[i] = i * 5 + 1;
        c[i] = i * 7 + 2;
        x[i] = i * 11;
        y[i] = i * 13;
        z[i] = i * 17;
        data[i] = i * 19;
        coeff[i] = i * 23 + 3;
        arr1[i] = i * 29;
        arr2[i] = i * 31 + 7;
    }
    
    for (int i = 0; i < M; i++) {
        vec[i] = i * 37;
    }
    
    for (int i = 0; i < N * M; i++) {
        mat[i] = i * 41;
    }
    
    /* Call all test functions to ensure they're compiled */
    loop_carried_dep(N, a, b, c);
    multiple_deps(N, x, y, z);
    nested_loops(10, M, mat, vec);
    variable_trip_count(N, data, coeff);
    unrolled_with_deps(N, arr1, arr2);
    
    /* Compute checksum to use results and prevent elimination */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum ^= a[i] ^ x[i] ^ data[i] ^ arr1[i];
        if (i < M) checksum ^= vec[i];
    }
    
    /* Use checksum so compiler can't optimize everything away */
    printf("Result checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(x); free(y); free(z);
    free(mat); free(vec);
    free(data); free(coeff);
    free(arr1); free(arr2);
    
    return checksum != 0 ? 0 : 1;
}
