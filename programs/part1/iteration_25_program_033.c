/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */

#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(void) {
    static int counter = 0;
    return ++counter;
}

/* Volatile read to prevent optimization */
static int volatile_read(void) {
    volatile int v = 42;
    return v;
}

/* Target function with complex loop carrying multiple dependency types */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Initialize with volatile to prevent dead code elimination */
    int init_val = volatile_read();
    
    /* Loop with carefully crafted dependencies */
    for (i = 1; i < n; i++) {
        /* 1. FLOW DEPENDENCY (RAW) with distance 1 - loop-carried */
        /* Read a[i-1] from previous iteration, write to a[i] */
        a[i] = a[i-1] + b[i] + init_val;
        
        /* 2. ANTI DEPENDENCY (WAR) within same iteration */
        /* Read from c[i], then write to it */
        int temp = c[i] + get_value();
        c[i] = temp * 2;
        
        /* 3. OUTPUT DEPENDENCY (WAW) within same iteration */
        /* Two writes to d[i] with computation in between */
        d[i] = b[i] * 3;
        /* Some computation that compiler can't eliminate */
        int intermediate = get_value() % 7;
        d[i] = d[i] + intermediate;
        
        /* 4. Another FLOW DEPENDENCY (RAW) within same iteration */
        /* Use a[i] just computed, write to b[i+1] for next iteration */
        if (i < n-1) {
            b[i+1] = a[i] - c[i];
        }
        
        /* 5. Memory dependency with variant index */
        /* Create potential aliasing to force conservative analysis */
        int idx = i % 5;
        a[idx] = b[idx] + c[i % 3];
    }
}

/* Secondary loop with different patterns to increase coverage */
void __attribute__((noinline))
process_loop2(int *restrict x, int *restrict y, int n) {
    int i;
    
    /* Loop with cross-iteration dependencies of distance > 1 */
    for (i = 2; i < n; i++) {
        /* Flow dependency with distance 2 */
        x[i] = x[i-2] * y[i-1] + volatile_read();
        
        /* Anti dependency spanning iterations */
        int tmp = y[i-1];
        y[i] = tmp + x[i] / 2;
        
        /* Output dependency with computation */
        x[i-1] = tmp * 3;
        x[i-1] = x[i-1] + (i % 10);
    }
}

int main(int argc, char **argv) {
    int n = 1000;
    
    /* Use command line argument to prevent constant propagation */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;  /* Ensure minimum size */
    }
    
    /* Allocate arrays with volatile to prevent optimization */
    volatile int size = n;
    int *a = (int*)malloc(size * sizeof(int));
    int *b = (int*)malloc(size * sizeof(int));
    int *c = (int*)malloc(size * sizeof(int));
    int *d = (int*)malloc(size * sizeof(int));
    int *x = (int*)malloc(size * sizeof(int));
    int *y = (int*)malloc(size * sizeof(int));
    
    if (!a || !b || !c || !d || !x || !y) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < n; i++) {
        a[i] = i % 100;
        b[i] = (i * 3) % 100;
        c[i] = (i * 5) % 100;
        d[i] = (i * 7) % 100;
        x[i] = (i * 11) % 100;
        y[i] = (i * 13) % 100;
    }
    
    /* Call the loops multiple times to ensure execution */
    for (int iter = 0; iter < 3; iter++) {
        process_loop(a, b, c, d, n);
        process_loop2(x, y, n);
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += a[i] + b[i] + c[i] + d[i] + x[i] + y[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    free(a); free(b); free(c); free(d); free(x); free(y);
    return 0;
}
