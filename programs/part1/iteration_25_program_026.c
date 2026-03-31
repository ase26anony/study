/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(int x) {
    return x * 3 + 1;
}

/* Volatile access functions to prevent optimization */
static volatile int vol_source;
static int volatile_read(void) {
    return vol_source;
}
static void volatile_write(int val) {
    vol_source = val;
}

/* Main processing function with complex loop dependencies */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Initialize with volatile to prevent dead code elimination */
    int init_val = volatile_read();
    
    /* Loop with multiple dependency types */
    for (i = 1; i < n; i++) {
        /* 1. FLOW (RAW) dependency within iteration */
        int temp = a[i] + init_val;      /* Read a[i] */
        b[i] = temp * 2;                 /* Write b[i] */
        
        /* 2. FLOW (RAW) dependency with distance=1 (loop-carried) */
        c[i] = c[i-1] + b[i];            /* Read c[i-1], Write c[i] */
        
        /* 3. ANTI (WAR) dependency */
        int old_d = d[i];                /* Read d[i] */
        d[i] = get_value(i);             /* Write d[i] - anti-dep on line above */
        
        /* 4. OUTPUT (WAW) dependency */
        a[i] = old_d * 3;                /* Write a[i] - output dep on next iteration's read */
        
        /* 5. Another FLOW dependency with potential memory aliasing */
        /* This creates conservative dependencies */
        if (i % 2 == 0) {
            b[i] = a[i-1] + 1;           /* Flow dep with distance=1 */
        }
        
        /* 6. Complex expression to prevent vectorization */
        volatile_write(i);               /* Volatile write creates memory barrier */
    }
    
    /* Final store to prevent dead code elimination */
    a[0] = b[n-1] + c[n-1];
}

/* Secondary function with different dependency pattern */
void __attribute__((noinline))
process_loop2(int *restrict x, int *restrict y, int n) {
    int i;
    
    /* Loop with output dependencies and anti-dependencies */
    for (i = 0; i < n - 1; i++) {
        /* Output dependency chain */
        x[i] = y[i] + 1;                 /* Write x[i] */
        x[i] = x[i] * 2;                 /* Write x[i] again - output dep */
        
        /* Anti-dependency with pointer aliasing possibility */
        int tmp = x[i+1];                /* Read x[i+1] */
        y[i] = tmp + i;                  /* Write y[i] */
        x[i+1] = y[i] * 3;               /* Write x[i+1] - anti-dep on line above */
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
    
    /* Allocate arrays with restrict to help alias analysis */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    int *d = (int*)malloc(n * sizeof(int));
    int *x = (int*)malloc(n * sizeof(int));
    int *y = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d || !x || !y) {
        return 1;
    }
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < n; i++) {
        a[i] = i * 2;
        b[i] = i * 3;
        c[i] = i * 5;
        d[i] = i * 7;
        x[i] = i;
        y[i] = i * 11;
    }
    
    /* Set volatile source to prevent optimization */
    vol_source = argc;
    
    /* Call processing functions multiple times to ensure optimization */
    for (int iter = 0; iter < 3; iter++) {
        process_loop(a, b, c, d, n);
        process_loop2(x, y, n);
        
        /* Modify inputs slightly to prevent complete optimization */
        a[0] += iter;
        x[0] += iter;
    }
    
    /* Compute checksum to ensure computations aren't optimized away */
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += a[i] + b[i] + c[i] + d[i] + x[i] + y[i];
    }
    
    /* Use checksum in output */
    printf("Result checksum: %d\n", sum % 1000);
    
    free(a); free(b); free(c); free(d); free(x); free(y);
    return 0;
}
