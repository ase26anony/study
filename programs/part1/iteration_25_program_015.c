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

/* Target function with carefully constructed data dependencies */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Initialize with volatile to prevent dead code elimination */
    int init_val = volatile_read();
    
    /* Pre-loop setup to create initial values */
    a[0] = init_val;
    b[0] = init_val + 1;
    
    /* Main loop with multiple dependency types */
    for (i = 1; i < n; i++) {
        /* 1. FLOW DEPENDENCY (RAW) with distance 1 - loop-carried */
        /* Read a[i-1], write to a[i] */
        int temp = a[i-1] + get_value();  /* Opaque call prevents const folding */
        a[i] = temp + b[i];               /* Flow dep from a[i-1] to a[i] */
        
        /* 2. ANTI DEPENDENCY (WAR) within same iteration */
        /* Read b[i], then write to b[i] */
        int b_read = b[i];                /* Read b[i] */
        b[i] = c[i] * 2;                  /* Write b[i] - anti-dep with above read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) within same iteration */
        /* Two writes to c[i] */
        c[i] = a[i] + b_read;             /* First write to c[i] */
        c[i] = c[i] * 3;                  /* Second write to c[i] - output dep */
        
        /* 4. FLOW DEPENDENCY (RAW) within iteration */
        /* Use c[i] after it's written */
        d[i] = c[i] + a[i];               /* Flow dep from c[i] write above */
        
        /* 5. ANTI DEPENDENCY (WAR) with loop-carried */
        /* Read d[i-1], then write to d[i-1] in next iteration (simulated) */
        if (i < n-1) {
            int d_read = d[i-1];          /* Read d[i-1] */
            /* This creates anti-dep with d[i-1] write in next iteration */
            /* The dependency will be analyzed by DDG */
        }
        
        /* 6. Complex memory dependency with variant index */
        /* Creates flow dependency with non-unit distance */
        int idx = i % 10;
        if (idx > 0) {
            a[idx] = a[idx-1] + 1;        /* Flow dep with distance 1 */
        }
    }
    
    /* Post-loop to ensure computations aren't dead */
    a[n-1] = a[n-1] + volatile_read();
}

/* Another loop with different patterns to increase coverage */
void __attribute__((noinline))
process_loop2(int *restrict x, int *restrict y, int n) {
    int i;
    
    /* Initialize */
    x[0] = volatile_read();
    y[0] = x[0] + 1;
    
    /* Loop with cross-iteration dependencies */
    for (i = 1; i < n; i++) {
        /* Multiple interleaved dependencies */
        int t1 = x[i-1];                  /* Flow dep from prev iteration */
        int t2 = y[i];                    /* Anti dep potential */
        
        x[i] = t1 + t2;                   /* Flow from t1, anti from t2 */
        y[i] = x[i] * 2;                  /* Flow from x[i] */
        
        /* Create output dependency */
        int tmp = x[i];
        x[i] = tmp + y[i-1];              /* Output dep on x[i], flow from y[i-1] */
        
        /* Memory dependency with computed index */
        int j = i & 3;  /* 0-3 */
        if (j > 0) {
            y[j] = x[j-1] + y[j];         /* Flow and anti dependencies */
        }
    }
}

int main(int argc, char **argv) {
    int n = 1000;
    
    /* Use command line argument to prevent constant propagation */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = 1000;
        if (n > 10000) n = 10000;  /* Limit for safety */
    }
    
    /* Allocate arrays with restrict to help alias analysis */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    int *d = (int*)malloc(n * sizeof(int));
    int *x = (int*)malloc(n * sizeof(int));
    int *y = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d || !x || !y) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
        d[i] = i * 4;
        x[i] = i * 5;
        y[i] = i * 6;
    }
    
    /* Call the processing functions */
    process_loop(a, b, c, d, n);
    process_loop2(x, y, n);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += a[i] + b[i] + c[i] + d[i] + x[i] + y[i];
        checksum &= 0xFFF;  /* Prevent overflow */
    }
    
    /* Use checksum in output */
    printf("Result checksum: %d (n=%d)\n", checksum, n);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(x); free(y);
    
    return 0;
}
