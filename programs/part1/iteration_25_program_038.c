/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(void) {
    static int counter = 0;
    return ++counter;
}

/* Volatile read to prevent optimization */
static volatile int volatile_sink;

/* Function with complex loop containing all dependency types */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Initialize with opaque function to prevent dead code elimination */
    int init = get_value();
    a[0] = init;
    b[0] = init + 1;
    
    /* Main loop with multiple dependency patterns */
    for (i = 1; i < n; i++) {
        /* 1. FLOW DEPENDENCY (RAW) with distance 1 - loop-carried */
        /* Read a[i-1] from previous iteration, write to a[i] */
        int temp = a[i-1] + b[i];  /* RAW on a[i-1] from previous iteration */
        a[i] = temp;               /* WAW with potential later write */
        
        /* 2. ANTI DEPENDENCY (WAR) within same iteration */
        /* Read b[i], then write to b[i] */
        int b_read = b[i];         /* Read b[i] */
        b[i] = c[i] * 2;           /* WAR: Write to b[i] after reading it */
        
        /* 3. OUTPUT DEPENDENCY (WAW) within same iteration */
        /* Multiple writes to same location */
        c[i] = temp + b_read;      /* First write to c[i] */
        c[i] = c[i] * 3;           /* WAW: Second write to c[i] */
        
        /* 4. FLOW DEPENDENCY (RAW) within same iteration */
        /* Chain of operations */
        d[i] = c[i] + a[i];        /* RAW on c[i] and a[i] */
        
        /* 5. Complex memory dependency with variant index */
        /* Create cross-iteration memory dependency */
        if (i > 2) {
            /* Flow dependency with distance 2 */
            a[i] += d[i-2];        /* RAW on d[i-2] from iteration i-2 */
        }
        
        /* 6. Anti dependency with distance 1 */
        if (i < n-1) {
            /* Read c[i+1] early, will be written in next iteration */
            volatile_sink = c[i+1]; /* Anti dependency to next iteration's write */
        }
    }
    
    /* Final volatile write to prevent dead code elimination */
    volatile_sink = a[n-1] + b[n-1];
}

/* Another loop with different pattern to increase coverage */
void __attribute__((noinline))
process_loop2(int *restrict x, int *restrict y, int n) {
    int i;
    
    /* Initialize */
    x[0] = get_value();
    y[0] = get_value();
    
    /* Loop with output and anti dependencies */
    for (i = 1; i < n; i++) {
        /* Output dependency chain */
        x[i] = y[i-1] + i;         /* Flow from y[i-1] */
        x[i] = x[i] * 2;           /* WAW on x[i] */
        x[i] = x[i] - 1;           /* Another WAW on x[i] */
        
        /* Anti dependency */
        int y_temp = y[i];         /* Read y[i] */
        y[i] = x[i] + y_temp;      /* WAR: Write y[i] after reading it */
        
        /* Flow dependency with multiple consumers */
        int shared = x[i] + y[i];  /* RAW on x[i] and y[i] */
        x[i-1] = shared;           /* Flow to next iteration's read of x[i-1] */
        y[i] = shared / 2;         /* Another use of shared */
    }
}

/* Main function with runtime-determined loop bounds */
int main(int argc, char *argv[]) {
    /* Use command line argument for loop bound to prevent constant folding */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 10;
        if (n > 10000) n = 10000;
    }
    
    /* Allocate arrays with dynamic size */
    int *a = (int *)malloc(n * sizeof(int));
    int *b = (int *)malloc(n * sizeof(int));
    int *c = (int *)malloc(n * sizeof(int));
    int *d = (int *)malloc(n * sizeof(int));
    int *x = (int *)malloc(n * sizeof(int));
    int *y = (int *)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d || !x || !y) {
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
    
    /* Call loops multiple times to ensure optimization */
    for (int iter = 0; iter < 10; iter++) {
        process_loop(a, b, c, d, n);
        process_loop2(x, y, n);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += a[i] + b[i] + c[i] + d[i] + x[i] + y[i];
    }
    
    /* Use result */
    printf("Checksum: %d\n", sum % 1000);
    
    free(a); free(b); free(c); free(d); free(x); free(y);
    return 0;
}
