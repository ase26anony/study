/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(void) {
    static int counter = 0;
    return ++counter;
}

/* Volatile read to prevent optimization */
static volatile int volatile_sink;

/* Function with complex loop carrying multiple dependency types */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Initialize with opaque values */
    int init_val = get_value();
    a[0] = init_val;
    b[0] = init_val + 1;
    
    /* Main loop with carefully crafted dependencies */
    for (i = 1; i < n; i++) {
        /* 1. FLOW dependency (RAW) within iteration */
        int temp = a[i-1] + b[i];      /* Read a[i-1] */
        c[i] = temp * 2;               /* Write c[i] */
        
        /* 2. ANTI dependency (WAR) within iteration */
        int read_before_write = b[i];  /* Read b[i] */
        b[i] = read_before_write + c[i-1]; /* Write b[i] - anti-dep on line above */
        
        /* 3. OUTPUT dependency (WAW) within iteration */
        a[i] = temp + i;               /* First write to a[i] */
        a[i] = a[i] + read_before_write; /* Second write to a[i] - output dep */
        
        /* 4. FLOW dependency with distance > 0 (loop-carried) */
        d[i] = d[i-1] + a[i];          /* Flow dep from iteration i-1 to i */
        
        /* 5. Additional ANTI dependency with memory aliasing */
        volatile_sink = c[i];          /* Read c[i] - prevents dead code elimination */
        c[i] = volatile_sink + 1;      /* Write c[i] - anti-dep on previous line */
    }
}

/* Another loop with cross-iteration anti and output dependencies */
void __attribute__((noinline))
process_loop2(int *restrict x, int *restrict y, int n) {
    int i;
    
    /* Initialize */
    x[0] = get_value();
    y[0] = get_value();
    
    for (i = 1; i < n; i++) {
        /* Complex chain of dependencies */
        int t1 = x[i-1] + y[i-1];      /* Flow from i-1 */
        int t2 = t1 * i;               /* Flow within iteration */
        
        /* Create anti dependency chain */
        int old_x = x[i];              /* Read x[i] before write */
        x[i] = t2 + old_x;             /* Write x[i] - anti-dep */
        
        /* Output dependency with flow */
        y[i] = old_x * 2;              /* First write to y[i] */
        y[i] = y[i] + x[i-1];          /* Second write to y[i] - output dep + flow */
        
        /* Cross-iteration output dependency */
        if (i % 3 == 0) {
            x[i-1] = y[i] + 1;         /* Write to x[i-1] - output across iterations */
        }
    }
}

/* Main function with runtime-determined loop bounds */
int main(int argc, char *argv[]) {
    /* Use command line or volatile to prevent constant folding */
    volatile int size = 1000;
    if (argc > 1) {
        size = 100;  /* Different size to vary optimization decisions */
    }
    
    int n = size;
    
    /* Allocate arrays with restrict to help alias analysis */
    int *a = __builtin_malloc(n * sizeof(int));
    int *b = __builtin_malloc(n * sizeof(int));
    int *c = __builtin_malloc(n * sizeof(int));
    int *d = __builtin_malloc(n * sizeof(int));
    int *x = __builtin_malloc(n * sizeof(int));
    int *y = __builtin_malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d || !x || !y) {
        return 1;
    }
    
    /* Call both loops to increase coverage chances */
    process_loop(a, b, c, d, n);
    process_loop2(x, y, n);
    
    /* Compute checksum to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += a[i] + b[i] + c[i] + d[i] + x[i] + y[i];
    }
    
    /* Use result */
    volatile_sink = sum;
    
    __builtin_free(a);
    __builtin_free(b);
    __builtin_free(c);
    __builtin_free(d);
    __builtin_free(x);
    __builtin_free(y);
    
    return 0;
}
