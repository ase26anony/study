/* test_ddg.c - Program to trigger DDG edge creation in GCC */

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

/* Target function with complex data dependencies */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Initialize with volatile to prevent dead code elimination */
    int init = volatile_read();
    a[0] = init;
    b[0] = init + 1;
    
    /* Main loop with multiple dependency types */
    for (i = 1; i < n; i++) {
        /* FLOW dependency (RAW): a[i] depends on a[i-1] - loop-carried, distance=1 */
        a[i] = a[i-1] + b[i];           /* Statement 1: Flow dep from prev iteration */
        
        /* ANTI dependency (WAR): c[i] reads a[i] before it's overwritten */
        int temp = a[i] + get_value();  /* Statement 2: Read a[i] */
        c[i] = temp * 2;                /* Statement 3: Use temp */
        
        /* OUTPUT dependency (WAW): a[i] written twice */
        a[i] = c[i-1] + d[i];           /* Statement 4: Overwrite a[i] - Output dep with Stmt 1 */
        
        /* Another FLOW dependency with different distance */
        b[i] = b[i-2] + a[i];           /* Statement 5: Flow dep distance=2 */
        
        /* Complex memory dependencies with variant indices */
        int idx = i % 10;
        d[idx] = c[idx] + get_value();  /* Statement 6: Memory dep through arrays */
        
        /* ANTI dependency through memory */
        int val = d[i % 5];             /* Statement 7: Read d[] */
        c[i] = val + a[i];              /* Statement 8: Write c[i] - Anti with Stmt 6 */
    }
}

/* Another loop with different patterns to increase coverage */
void __attribute__((noinline))
process_loop2(float *restrict x, float *restrict y, 
              float *restrict z, int m) {
    int j;
    float acc = 0.0f;
    
    /* Loop with reduction and dependencies */
    for (j = 0; j < m; j++) {
        /* Flow dependency through accumulator - loop-carried */
        acc = acc + x[j] * y[j];        /* Statement A: Flow dep distance=1 */
        
        /* Anti dependency: read z[j], then modify it */
        float old_z = z[j];             /* Statement B: Read z[j] */
        z[j] = acc + old_z;             /* Statement C: Write z[j] - Anti with future reads */
        
        /* Output dependency: multiple writes to same location */
        if (j % 3 == 0) {
            z[j] = old_z * 2.0f;        /* Statement D: Another write to z[j] - Output with Stmt C */
        }
        
        /* Cross-iteration memory dependency */
        x[(j + 1) % m] = z[j] + 0.5f;   /* Statement E: Flow to next iteration */
    }
}

/* Main function with runtime-determined bounds */
int main(int argc, char *argv[]) {
    /* Use command line or volatile to prevent constant folding */
    volatile int size = 1000;
    if (argc > 1) {
        size = 100;  /* Different size for different runs */
    }
    
    int n = size + (argc > 1 ? 0 : volatile_read() % 50);
    
    /* Allocate arrays with restrict to help alias analysis */
    int *a = __builtin_malloc(n * sizeof(int));
    int *b = __builtin_malloc(n * sizeof(int));
    int *c = __builtin_malloc(n * sizeof(int));
    int *d = __builtin_malloc(n * sizeof(int));
    
    float *x = __builtin_malloc(n * sizeof(float));
    float *y = __builtin_malloc(n * sizeof(float));
    float *z = __builtin_malloc(n * sizeof(float));
    
    /* Initialize arrays to prevent undefined behavior */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
        d[i] = i * 4;
        x[i] = i * 0.1f;
        y[i] = i * 0.2f;
        z[i] = i * 0.3f;
    }
    
    /* Call loops multiple times to ensure execution */
    for (int iter = 0; iter < 3; iter++) {
        process_loop(a, b, c, d, n);
        process_loop2(x, y, z, n);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int sum = 0;
    float fsum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += a[i] + b[i] + c[i] + d[i];
        fsum += x[i] + y[i] + z[i];
    }
    
    /* Use results to prevent optimization */
    if (sum > 0 && fsum > 0) {
        __builtin_printf("Result: %d, %f\n", sum % 1000, fsum);
    }
    
    __builtin_free(a);
    __builtin_free(b);
    __builtin_free(c);
    __builtin_free(d);
    __builtin_free(x);
    __builtin_free(y);
    __builtin_free(z);
    
    return 0;
}
