/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(void) {
    static int counter = 0;
    return ++counter;
}

/* Volatile access to prevent optimization */
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
    
    /* Main loop with various dependencies */
    for (i = 1; i < n; i++) {
        /* 1. FLOW dependency (RAW): a[i] depends on a[i-1] - loop-carried, distance=1 */
        int temp = a[i-1] + b[i];      /* Read a[i-1] */
        a[i] = temp * 2;               /* Write a[i] - flow dep from previous iteration */
        
        /* 2. ANTI dependency (WAR): between c[i] and a[i] */
        c[i] = a[i] + 3;               /* Read a[i] */
        a[i] = get_value();            /* Write a[i] - anti dep with previous read */
        
        /* 3. OUTPUT dependency (WAW): on array 'd' */
        d[i] = b[i] * 4;               /* Write d[i] */
        d[i] = c[i] + d[i];            /* Write d[i] again - output dep */
        
        /* 4. Another FLOW dependency with different distance */
        if (i > 2) {
            b[i] = b[i-2] + a[i];      /* Loop-carried flow, distance=2 */
        }
        
        /* 5. Memory dependency with variant index to prevent simplification */
        int idx = i % 10;
        c[idx] = a[i] + b[idx];        /* Creates cross-iteration memory deps */
        
        /* Volatile write to prevent dead code elimination */
        volatile_sink = a[i] + b[i] + c[i] + d[i];
    }
    
    /* Final store with dependency */
    a[n-1] = b[n-1] + c[n-1];
}

/* Another function with different pattern to ensure edge variety */
void __attribute__((noinline))
process_loop2(float *restrict x, float *restrict y, int n) {
    int i;
    
    /* Initialize */
    x[0] = 1.0f;
    y[0] = 2.0f;
    
    for (i = 1; i < n; i++) {
        /* Multiple interleaved dependencies */
        float t1 = x[i-1] * y[i-1];    /* Flow, distance=1 */
        float t2 = t1 + x[i];          /* Flow within iteration */
        
        x[i] = t2 * 0.5f;              /* Write x[i] */
        y[i] = x[i] + y[i-1];          /* Flow from x[i], flow from y[i-1] (distance=1) */
        
        /* Anti dependency pattern */
        float read_x = x[i];           /* Read x[i] */
        x[i] = read_x * y[i];          /* Write x[i] - anti dep */
        
        /* Output dependency */
        y[i] = t1 + t2;                /* Write y[i] */
        y[i] = y[i] * 1.1f;            /* Write y[i] again - output dep */
    }
}

/* Main function with runtime-determined loop bounds */
int main(int argc, char *argv[]) {
    /* Use command line or volatile to prevent constant folding */
    volatile int size = 1000;
    if (argc > 1) {
        size = 100;  /* Different size for different runs */
    }
    
    int n = size;
    
    /* Allocate arrays with enough size for dependencies */
    int *a = __builtin_alloca(n * sizeof(int));
    int *b = __builtin_alloca(n * sizeof(int));
    int *c = __builtin_alloca(n * sizeof(int));
    int *d = __builtin_alloca(n * sizeof(int));
    
    float *x = __builtin_alloca(n * sizeof(float));
    float *y = __builtin_alloca(n * sizeof(float));
    
    /* Initialize with non-constant values */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
        d[i] = i * 4;
        x[i] = i * 0.5f;
        y[i] = i * 0.7f;
    }
    
    /* Call processing functions multiple times */
    for (int iter = 0; iter < 10; iter++) {
        process_loop(a, b, c, d, n);
        process_loop2(x, y, n);
    }
    
    /* Compute checksum to prevent optimization */
    int sum = 0;
    float fsum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += a[i] + b[i] + c[i] + d[i];
        fsum += x[i] + y[i];
    }
    
    /* Use results */
    volatile_sink = sum + (int)fsum;
    
    return sum > 0 ? 0 : 1;
}
