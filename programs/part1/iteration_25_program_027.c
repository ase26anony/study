/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(void) {
    static int counter = 0;
    return ++counter;
}

/* Volatile read to prevent dead code elimination */
static volatile int volatile_sink;

/* Main processing function with carefully constructed dependencies */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Loop with multiple dependency types */
    for (i = 1; i < n; i++) {
        /* 1. FLOW DEPENDENCY (RAW) with distance 1 - loop-carried */
        /* Read a[i-1] from previous iteration, write to a[i] */
        int temp = a[i-1] + get_value();  /* Prevents constant folding */
        a[i] = temp;
        
        /* 2. ANTI DEPENDENCY (WAR) within same iteration */
        /* Read b[i], then write to b[i] */
        int read_b = b[i];
        b[i] = read_b * 2 + get_value();
        
        /* 3. OUTPUT DEPENDENCY (WAW) within same iteration */
        /* Two writes to c[i] */
        c[i] = read_b + temp;
        c[i] = c[i] * 3;  /* Overwrites previous value - WAW */
        
        /* 4. FLOW DEPENDENCY (RAW) within same iteration */
        /* Read from c[i] (just written), write to d[i] */
        d[i] = c[i] + a[i];
        
        /* 5. ANTI DEPENDENCY (WAR) with array aliasing possibility */
        /* Force compiler to consider potential aliasing */
        int idx = i & 3;  /* Non-linear index */
        int val = d[idx];
        a[idx] = val + i;  /* WAR if idx == i in some iterations */
        
        /* 6. Complex memory dependency with variant indices */
        /* Creates cross-iteration dependencies with varying distances */
        int j = (i * 7) % n;
        if (j > 0) {
            b[j] = b[j-1] + a[i];  /* Flow dependency with variable distance */
        }
    }
    
    /* Prevent tail elimination */
    volatile_sink = a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Another loop with different patterns to increase coverage */
void __attribute__((noinline))
process_loop2(float *restrict x, float *restrict y, int m) {
    int i;
    
    /* Loop with floating-point operations and dependencies */
    for (i = 2; i < m; i++) {
        /* Flow dependency chain */
        float t1 = x[i-1] * 0.5f;
        float t2 = t1 + y[i-2];
        x[i] = t2 * t2;
        
        /* Anti dependency */
        float old_y = y[i];
        y[i] = old_y * x[i] + 1.0f;
        
        /* Output dependency */
        float accum = x[i] + y[i];
        accum = accum * accum;  /* WAW */
        x[i] = accum;
        
        /* Memory dependency with pointer chasing */
        if (i % 4 == 0) {
            y[i/2] = x[i] * 2.0f;
        }
    }
    
    volatile_sink = (int)(x[m-1] + y[m-1]);
}

/* Main function with runtime-determined bounds */
int main(int argc, char *argv[]) {
    /* Use command line or volatile to prevent constant folding */
    int size = 1000;
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size < 10) size = 1000;
    }
    
    /* Allocate arrays with restrict to help alias analysis */
    int *a = (int*)malloc(size * sizeof(int));
    int *b = (int*)malloc(size * sizeof(int));
    int *c = (int*)malloc(size * sizeof(int));
    int *d = (int*)malloc(size * sizeof(int));
    
    float *x = (float*)malloc(size * sizeof(float));
    float *y = (float*)malloc(size * sizeof(float));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < size; i++) {
        a[i] = i * 3;
        b[i] = i * 5;
        c[i] = i * 7;
        d[i] = i * 11;
        x[i] = (float)i * 0.1f;
        y[i] = (float)i * 0.3f;
    }
    
    /* Call processing functions multiple times */
    for (int iter = 0; iter < 10; iter++) {
        process_loop(a, b, c, d, size);
        process_loop2(x, y, size);
        
        /* Modify inputs slightly each iteration */
        a[0] += iter;
        x[0] += (float)iter;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int sum_int = 0;
    float sum_float = 0.0f;
    for (int i = 0; i < size; i++) {
        sum_int += a[i] + b[i] + c[i] + d[i];
        sum_float += x[i] + y[i];
    }
    
    volatile_sink = sum_int + (int)sum_float;
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(x); free(y);
    
    return 0;
}
