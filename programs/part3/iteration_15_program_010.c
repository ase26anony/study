/* test_ddg.c - Program to trigger DDG edge creation in GCC's modulo scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant propagation and loop unrolling */
volatile int N = 1024;

/* Function containing the loop with carried dependencies */
__attribute__((noinline))
int compute_loop(int* restrict a, int* restrict b, 
                 int* restrict c, int* restrict d, int n) {
    int i;
    
    /* Loop with multiple carried dependencies for DDG construction */
    for (i = 1; i < n; ++i) {
        /* True (flow) dependence: a[i] depends on a[i-1] from previous iteration */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Anti-dependence: b[i] is written, but b[i-1] might be read in next iteration
           if we had: c[i] = b[i-1] + ... (we'll add this later) */
        
        /* Output dependence: multiple writes to same array in same iteration */
        a[i] = a[i] + d[i];  /* Second write to a[i] creates output dependence */
        
        /* Another true dependence: b[i] depends on a[i] computed above */
        b[i] = a[i] - d[i];
        
        /* Anti-dependence example: read b[i-1] after writing b[i] in previous iteration */
        if (i < n-1) {
            c[i+1] = b[i] << 2;  /* Creates anti-dependence on b[i] for next iteration */
        }
    }
    
    /* Simple checksum to prevent dead code elimination */
    return a[n-1] + b[n-1];
}

/* Another loop with different dependency patterns */
__attribute__((noinline))
int compute_loop2(int* restrict x, int* restrict y, 
                  int* restrict z, int n) {
    int i;
    
    /* Different loop with register-to-register dependencies */
    int acc = x[0];
    for (i = 1; i < n; ++i) {
        /* Chain of dependencies within same iteration */
        int t1 = acc * y[i];
        int t2 = t1 + z[i];
        int t3 = t2 - x[i];
        acc = t3 + acc;  /* Loop-carried dependence through acc */
        
        x[i] = acc;
        
        /* Create anti-dependence through y array */
        y[i-1] = z[i] + 1;
    }
    
    return acc + x[n-1];
}

int main(void) {
    int i;
    
    /* Allocate and initialize arrays with deterministic values */
    int* a = (int*)malloc(N * sizeof(int));
    int* b = (int*)malloc(N * sizeof(int));
    int* c = (int*)malloc(N * sizeof(int));
    int* d = (int*)malloc(N * sizeof(int));
    
    int* x = (int*)malloc(N * sizeof(int));
    int* y = (int*)malloc(N * sizeof(int));
    int* z = (int*)malloc(N * sizeof(int));
    
    if (!a || !b || !c || !d || !x || !y || !z) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic values */
    for (i = 0; i < N; ++i) {
        a[i] = (i * 37) % 1001;
        b[i] = (i * 53) % 997;
        c[i] = (i * 71) % 991;
        d[i] = (i * 29) % 983;
        
        x[i] = (i * 43) % 977;
        y[i] = (i * 61) % 971;
        z[i] = (i * 19) % 967;
    }
    
    /* Call the loop functions multiple times to give compiler more chances */
    int sum1 = 0, sum2 = 0;
    for (int iter = 0; iter < 3; ++iter) {
        sum1 += compute_loop(a, b, c, d, N);
        sum2 += compute_loop2(x, y, z, N);
        
        /* Modify inputs slightly for next iteration */
        a[0] += 1;
        x[0] += 1;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Checksum 1: %d\n", sum1);
    printf("Checksum 2: %d\n", sum2);
    printf("Total: %d\n", sum1 + sum2);
    
    /* Free allocated memory */
    free(a); free(b); free(c); free(d);
    free(x); free(y); free(z);
    
    return 0;
}
