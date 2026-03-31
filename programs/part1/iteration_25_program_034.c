/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(int x) {
    return x * 3 + 7;
}

/* Volatile read to prevent optimization */
static volatile int volatile_source = 42;

/* Target function with complex loop dependencies */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Initialize with volatile to prevent constant folding */
    int init_val = volatile_source;
    
    /* Complex loop with multiple dependency types */
    for (i = 1; i < n - 1; i++) {
        /* FLOW (RAW) dependency: a[i] depends on previous iteration's a[i-1] */
        int temp = a[i - 1] + get_value(i);  // Read a[i-1]
        a[i] = temp * 2;                     // Write a[i] - creates flow dep with distance=1
        
        /* ANTI (WAR) dependency: b[i] written after being read */
        int read_b = b[i] + init_val;        // Read b[i]
        b[i] = read_b * 3;                   // Write b[i] - creates anti dep within iteration
        
        /* OUTPUT (WAW) dependency: c written twice */
        c[i] = temp + read_b;                // First write to c[i]
        c[i] = c[i] * 2;                     // Second write to c[i] - creates output dep
        
        /* Another FLOW dependency with different distance */
        d[i + 1] = d[i] * 2 + a[i];          // Flow dep from d[i] to d[i+1], distance=1
        
        /* Memory dependency with variant index */
        int idx = i % 10;
        b[idx] = a[idx] + 1;                 // Potential flow/anti deps through memory
        
        /* Cross-iteration anti dependency */
        a[i - 1] = b[i] + c[i];              // Write a[i-1] after it was read earlier
                                             // Creates anti dep with distance=1
    }
    
    /* Final store to prevent dead code elimination */
    if (n > 0) {
        a[n - 1] = b[0] + c[0];
    }
}

/* Helper to create runtime-variant loop bounds */
static int get_iteration_count(int base) {
    /* Use volatile to prevent compile-time computation */
    volatile int adjust = 5;
    return base + adjust;
}

int main(int argc, char **argv) {
    /* Use command line arg to make loop bound non-constant */
    int n = 1000;
    if (argc > 1) {
        n = get_iteration_count(atoi(argv[1]));
    }
    
    /* Allocate arrays with restrict to help alias analysis */
    int *a = __builtin_malloc(sizeof(int) * (n + 10));
    int *b = __builtin_malloc(sizeof(int) * (n + 10));
    int *c = __builtin_malloc(sizeof(int) * (n + 10));
    int *d = __builtin_malloc(sizeof(int) * (n + 10));
    
    if (!a || !b || !c || !d) return 1;
    
    /* Initialize with pattern to create meaningful dependencies */
    for (int i = 0; i < n + 10; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
        d[i] = i * 4;
    }
    
    /* Call the target function */
    process_loop(a, b, c, d, n);
    
    /* Compute checksum to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += a[i] + b[i] + c[i] + d[i];
    }
    
    /* Use result */
    printf("Result checksum: %d\n", sum);
    
    __builtin_free(a);
    __builtin_free(b);
    __builtin_free(c);
    __builtin_free(d);
    
    return 0;
}
