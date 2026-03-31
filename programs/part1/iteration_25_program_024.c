/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(void) {
    static int counter = 0;
    return ++counter;
}

/* Volatile read to prevent optimization */
static volatile int volatile_source = 42;

/* Function with complex loop containing multiple dependency types */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Initialize with volatile read to prevent dead code elimination */
    int init_val = volatile_source;
    
    /* Loop with multiple data dependencies */
    for (i = 1; i < n; i++) {
        /* 1. FLOW DEPENDENCY (RAW) between statements */
        int temp = a[i-1] + init_val;      /* Read a[i-1] */
        b[i] = temp * 2;                   /* Write b[i] */
        c[i] = b[i] + get_value();         /* Read b[i] (flow from previous stmt) */
        
        /* 2. ANTI DEPENDENCY (WAR) */
        int read_before_write = d[i];      /* Read d[i] */
        d[i] = read_before_write + c[i];   /* Write d[i] (anti: read before write) */
        
        /* 3. OUTPUT DEPENDENCY (WAW) on 'a' array */
        a[i] = temp + d[i];                /* Write a[i] - first write */
        a[i] = a[i] * 3;                   /* Write a[i] again (output dependency) */
        
        /* 4. LOOP-CARRIED DEPENDENCY (distance > 0) */
        /* Flow from a[i] (current iteration) to a[i+1] (next iteration) */
        /* This creates edge with distance = 1 */
    }
    
    /* Cross-iteration dependency setup */
    a[0] = init_val;
    for (i = 1; i < n; i++) {
        /* True loop-carried flow dependency with distance 1 */
        a[i] = a[i-1] + b[i] + c[i];
        
        /* Another loop-carried anti dependency */
        int old_val = d[i];
        d[i-1] = old_val * 2;
    }
}

/* Another loop with different patterns to ensure edge variety */
void __attribute__((noinline))
process_loop2(int *restrict x, int *restrict y, int n) {
    int i;
    
    /* Initialize */
    x[0] = volatile_source;
    y[0] = x[0] + 1;
    
    /* Loop with mixed dependencies */
    for (i = 1; i < n - 1; i++) {
        /* Multiple interleaved dependencies */
        int t1 = x[i-1];           /* Flow from previous iteration (distance=1) */
        int t2 = y[i];             /* Anti: read before write below */
        x[i] = t1 + t2;            /* Write x[i] */
        y[i] = x[i] * 2;           /* Write y[i], read x[i] (flow) */
        
        /* Create output dependency */
        int t3 = x[i] + y[i];
        x[i] = t3;                 /* Second write to x[i] (output) */
        
        /* Another flow dependency chain */
        y[i+1] = x[i] + y[i-1];    /* Flow from x[i] and y[i-1] */
    }
}

/* Main function with runtime-determined loop bounds */
int main(int argc, char **argv) {
    /* Use command line argument for loop bound to prevent constant folding */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;  /* Ensure minimum size */
        if (n > 10000) n = 10000; /* Limit for safety */
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
    
    /* Call loops multiple times to ensure execution */
    for (int iter = 0; iter < 10; iter++) {
        process_loop(a, b, c, d, n);
        process_loop2(x, y, n);
        
        /* Mix up data to prevent pattern recognition */
        volatile_source = iter;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += a[i] + b[i] + c[i] + d[i] + x[i] + y[i];
    }
    
    /* Use checksum in output */
    printf("Result checksum: %d\n", checksum % 1000);
    
    free(a); free(b); free(c); free(d); free(x); free(y);
    return 0;
}
