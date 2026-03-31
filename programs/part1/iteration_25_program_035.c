/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(int x) {
    return x * 3 + 7;
}

/* Volatile read to prevent dead code elimination */
static volatile int volatile_source = 42;

/* Target function with carefully constructed data dependencies */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Initialize with volatile to prevent constant folding */
    int init_val = volatile_source;
    
    /* Loop with multiple dependency types */
    for (i = 1; i < n; i++) {
        /* 1. FLOW DEPENDENCY (RAW) with distance 1 (loop-carried) */
        /* Read a[i-1] from previous iteration, write to a[i] */
        a[i] = a[i-1] + b[i] + init_val;
        
        /* 2. ANTI DEPENDENCY (WAR) within same iteration */
        /* Read c[i] then immediately overwrite it */
        int temp = c[i] + get_value(i);
        c[i] = temp * 2;
        
        /* 3. OUTPUT DEPENDENCY (WAW) within same iteration */
        /* Multiple writes to d[i] */
        d[i] = b[i] * 3;
        d[i] = d[i] + a[i];  // Overwrites previous value
        
        /* 4. Another FLOW DEPENDENCY with anti in between */
        /* Read from a[i] (written above), write to b[i+1] for next iteration */
        if (i < n-1) {
            b[i+1] = a[i] - d[i];
        }
        
        /* 5. Complex memory dependency with variant index */
        /* Creates flow dependency with distance 1 */
        int idx = i % 10;
        c[idx] = c[idx] + 1;
    }
}

/* Secondary loop with different patterns */
void __attribute__((noinline))
process_loop2(float *restrict x, float *restrict y, int n) {
    int i;
    
    /* Initialize with volatile */
    float init = (float)volatile_source / 10.0f;
    
    /* Loop with floating-point dependencies */
    for (i = 2; i < n; i++) {
        /* Loop-carried flow dependency with distance 2 */
        x[i] = x[i-2] * y[i] + init;
        
        /* Anti dependency chain */
        float t1 = y[i];
        y[i] = x[i] * 0.5f;
        
        /* Output dependency */
        x[i-1] = t1 + 1.0f;
        x[i-1] = x[i-1] * 2.0f;  // Overwrite
        
        /* Another flow dependency */
        if (i < n-1) {
            y[i+1] = x[i] + y[i-1];
        }
    }
}

int main(int argc, char **argv) {
    int n = 1000;
    
    /* Use command line argument to prevent constant trip count */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;  /* Ensure minimum size */
    }
    
    /* Allocate arrays with volatile to prevent optimizations */
    volatile int size = n;
    int *a = (int*)malloc(size * sizeof(int));
    int *b = (int*)malloc(size * sizeof(int));
    int *c = (int*)malloc(size * sizeof(int));
    int *d = (int*)malloc(size * sizeof(int));
    
    float *x = (float*)malloc(size * sizeof(float));
    float *y = (float*)malloc(size * sizeof(float));
    
    if (!a || !b || !c || !d || !x || !y) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < n; i++) {
        a[i] = i * 2;
        b[i] = i * 3;
        c[i] = i * 5;
        d[i] = i * 7;
        x[i] = (float)i * 1.5f;
        y[i] = (float)i * 2.5f;
    }
    
    /* Call the loops multiple times to ensure execution */
    for (int iter = 0; iter < 3; iter++) {
        process_loop(a, b, c, d, n);
        process_loop2(x, y, n);
        
        /* Modify volatile to prevent loop invariant code motion */
        volatile_source = iter * 100;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int sum_int = 0;
    float sum_float = 0.0f;
    
    for (int i = 0; i < n; i++) {
        sum_int += a[i] + b[i] + c[i] + d[i];
        sum_float += x[i] + y[i];
    }
    
    printf("Checksum: int=%d, float=%.2f\n", sum_int, sum_float);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(x); free(y);
    
    return 0;
}
