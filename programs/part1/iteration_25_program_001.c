/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */

/* Opaque function to prevent optimization */
static int __attribute__((noinline)) get_value(void) {
    static int counter = 0;
    return ++counter;
}

/* Volatile read to prevent dead code elimination */
static volatile int volatile_sink;

/* Function with complex loop carrying multiple dependency types */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Initialize with opaque function to prevent constant propagation */
    int init_val = get_value();
    a[0] = init_val;
    b[0] = init_val + 1;
    
    /* Main loop with carefully crafted dependencies */
    for (i = 1; i < n; i++) {
        /* 1. FLOW DEPENDENCY (RAW) with distance 1 - loop-carried */
        /* Read a[i-1] from previous iteration, write to a[i] */
        int temp = a[i-1] + b[i];      /* RAW on a[i-1] from previous iteration */
        a[i] = temp * 2;               /* Becomes source for next iteration's RAW */
        
        /* 2. ANTI DEPENDENCY (WAR) within same iteration */
        /* Read then write to same location through different pointers */
        int read_before_write = b[i];  /* Read b[i] */
        b[i] = c[i] + get_value();     /* Write b[i] - WAR with previous read */
        volatile_sink = read_before_write; /* Use value to prevent elimination */
        
        /* 3. OUTPUT DEPENDENCY (WAW) on array 'c' */
        /* Two writes to c[i] in same iteration */
        c[i] = a[i] + b[i];            /* First write to c[i] */
        int opaque_val = get_value();  /* Opaque call prevents reordering */
        c[i] = c[i] * opaque_val;      /* Second write to c[i] - WAW */
        
        /* 4. FLOW DEPENDENCY with distance 0 (within iteration) */
        /* Chain of operations on d array */
        d[i] = a[i] + 1;               /* RAW on a[i] from earlier in iteration */
        d[i] = d[i] + c[i];            /* RAW on d[i] from previous statement */
        
        /* 5. Additional loop-carried dependency with distance > 1 */
        /* Skip some iterations to potentially create different distance edges */
        if (i >= 3) {
            /* Flow dependency with distance 3 */
            c[i] = c[i] + d[i-3];      /* RAW on d[i-3] from iteration i-3 */
        }
        
        /* 6. Memory dependency with variant index */
        /* Create potential memory aliasing that's hard to analyze */
        int idx = (i * 7) % n;         /* Non-linear index computation */
        if (idx > 0 && idx < n) {
            /* Anti-dependency through memory */
            int val = a[idx];          /* Read a at computed index */
            a[i % n] = val + i;        /* Write to different a element */
            volatile_sink = val;       /* Prevent elimination */
        }
    }
    
    /* Final store to prevent entire loop from being eliminated */
    volatile_sink = a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Helper to initialize arrays */
void init_arrays(int *a, int *b, int *c, int *d, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
        d[i] = i * 4;
    }
}

/* Main function with runtime-determined loop bound */
int main(int argc, char **argv) {
    /* Use command line argument for loop bound to prevent constant folding */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 10;
        if (n > 10000) n = 10000;
    }
    
    /* Allocate arrays with restrict to help alias analysis */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    int *d = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d) {
        return 1;
    }
    
    /* Initialize arrays */
    init_arrays(a, b, c, d, n);
    
    /* Call the function with the complex loop */
    process_loop(a, b, c, d, n);
    
    /* Compute checksum to ensure computations aren't eliminated */
    int checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += a[i] + b[i] + c[i] + d[i];
    }
    
    /* Output checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
