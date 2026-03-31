/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>

/* Function with simple loop-carried dependency */
void loop_carried_dep(int n, int *a, int *b, int *c) {
    volatile int acc = 0;
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence: a[i] depends on a[i-1] */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Another distance-1 dependence with accumulator */
        acc = acc + b[i] * 3;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(acc) : "memory");
    }
}

/* Function with multiple interleaved carried dependencies */
void multiple_deps(int n, int *x, int *y, int *z) {
    volatile int sum1 = 0, sum2 = 0;
    
    for (int i = 1; i < n; i++) {
        /* Two separate distance-1 dependencies */
        x[i] = y[i-1] + z[i];
        y[i] = x[i-1] * 2 - z[i];
        
        /* Two accumulators with carried dependencies */
        sum1 = sum1 + x[i];
        sum2 = sum2 + y[i] * 3;
        
        /* Memory barrier to preserve dependencies */
        asm volatile("" : : "r"(sum1), "r"(sum2) : "memory");
    }
}

/* Function with nested loops - inner loop has carried deps */
void nested_loops(int n, int m, int *mat, int *vec, int *out) {
    for (int i = 0; i < n; i++) {
        volatile int dot = 0;
        
        /* Inner loop with carried dependency */
        for (int j = 1; j < m; j++) {
            /* Distance-1 dependence in inner loop */
            dot = dot + mat[i*m + j] * vec[j-1];
            
            /* Another operation with carried dep */
            mat[i*m + j] = mat[i*m + j-1] + vec[j];
        }
        
        out[i] = dot;
        
        /* Prevent dead code elimination */
        asm volatile("" : : "r"(dot) : "memory");
    }
}

/* Function with complex arithmetic creating many dependencies */
void complex_loop(int n, int *a, int *b, int *c, int *d) {
    volatile int t1 = 0, t2 = 0, t3 = 0;
    
    for (int i = 2; i < n; i++) {
        /* Multiple interleaved distance-1 dependencies */
        t1 = a[i-1] * b[i] + t1;
        t2 = c[i-2] + d[i] * t2;
        t3 = t1 + t2 * t3;
        
        a[i] = t1 + t2;
        b[i] = t3 - a[i-1];
        c[i] = b[i-1] * c[i-1] + d[i];
        
        /* Force dependency preservation */
        asm volatile("" : : "r"(t1), "r"(t2), "r"(t3) : "memory");
    }
}

/* Function with unknown trip count (parameter) */
void variable_loop(int n, int *arr1, int *arr2, int *arr3) {
    if (n <= 1) return;
    
    volatile int carry = arr1[0];
    
    for (int i = 1; i < n; i++) {
        /* Classic distance-1 pattern */
        carry = carry * 3 + arr2[i];
        arr3[i] = arr1[i-1] + carry;
        
        /* Additional operation with dependency */
        arr1[i] = arr3[i-1] * 2 - arr2[i];
        
        /* Memory clobber */
        asm volatile("" : : "r"(carry) : "memory");
    }
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
    int *z = (int*)malloc(N * sizeof(int));
    int *mat = (int*)malloc(N * M * sizeof(int));
    int *vec = (int*)malloc(M * sizeof(int));
    int *out = (int*)malloc(N * sizeof(int));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N; i++) {
        a[i] = i * 3 + 1;
        b[i] = i * 5 - 2;
        c[i] = i * 7 + 3;
        d[i] = i * 11 - 5;
        x[i] = i * 13 + 7;
        y[i] = i * 17 - 11;
        z[i] = i * 19 + 13;
    }
    
    for (int i = 0; i < M; i++) {
        vec[i] = i * 23 - 17;
    }
    
    for (int i = 0; i < N * M; i++) {
        mat[i] = i * 29 + 19;
    }
    
    /* Call all test functions to ensure they're compiled */
    loop_carried_dep(N, a, b, c);
    multiple_deps(N, x, y, z);
    nested_loops(10, M, mat, vec, out);
    complex_loop(N, a, b, c, d);
    
    /* Use command line argument for variable trip count */
    int dynamic_n = (argc > 1) ? atoi(argv[1]) : 500;
    if (dynamic_n < 2) dynamic_n = 500;
    variable_loop(dynamic_n, a, b, c);
    
    /* Compute checksum to prevent dead code elimination */
    volatile int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += a[i] + b[i] + c[i] + x[i] + y[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(x); free(y); free(z);
    free(mat); free(vec); free(out);
    
    return 0;
}
