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
    
    /* Initialize with opaque function to prevent dead code elimination */
    int init_val = get_value();
    
    /* Loop with multiple dependency patterns */
    for (i = 1; i < n; i++) {
        /* 1. FLOW DEPENDENCY (RAW) between statements */
        int temp = a[i-1] + b[i];      /* Read a[i-1] */
        a[i] = temp * 2;               /* Write a[i] - flow dep on temp */
        
        /* 2. ANTI DEPENDENCY (WAR) */
        int read_before_write = c[i];  /* Read c[i] */
        c[i] = read_before_write + volatile_source; /* Write c[i] - anti dep */
        
        /* 3. OUTPUT DEPENDENCY (WAW) on 'd' */
        d[i] = init_val + i;           /* First write to d[i] */
        d[i] = d[i] * 3;               /* Second write to d[i] - output dep */
        
        /* 4. LOOP-CARRIED FLOW DEPENDENCY (distance = 1) */
        b[i] = b[i-1] + a[i];          /* Flow from iteration i-1 to i */
        
        /* 5. Complex memory dependency with variant index */
        int idx = i % 10;
        a[idx] = c[idx] + 1;           /* May create memory deps */
    }
}

/* Another function with different patterns */
void __attribute__((noinline))
process_loop2(float *restrict x, float *restrict y, int m) {
    int j;
    
    /* Loop with floating-point dependencies */
    for (j = 2; j < m; j++) {
        /* Multiple interleaved dependencies */
        float t1 = x[j-1] + y[j];
        float t2 = x[j-2] * t1;
        x[j] = t2 + 1.0f;
        
        /* Anti-dependency chain */
        float old_y = y[j];
        y[j] = old_y * 0.5f;
        y[j] = y[j] + x[j];  /* Output dependency on y[j] */
        
        /* Cross-iteration with distance 2 */
        y[j] = y[j] + x[j-2];
    }
}

/* Main function with runtime-determined loop bounds */
int main(int argc, char *argv[]) {
    /* Use command line or volatile to prevent constant folding */
    volatile int size = 1000;
    if (argc > 1) {
        size = 100;  /* Different size for different runs */
    }
    
    /* Allocate arrays with restrict to help alias analysis */
    int *a = __builtin_malloc(size * sizeof(int));
    int *b = __builtin_malloc(size * sizeof(int));
    int *c = __builtin_malloc(size * sizeof(int));
    int *d = __builtin_malloc(size * sizeof(int));
    
    float *x = __builtin_malloc(size * sizeof(float));
    float *y = __builtin_malloc(size * sizeof(float));
    
    if (!a || !b || !c || !d || !x || !y) {
        return 1;
    }
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < size; i++) {
        a[i] = i;
        b[i] = size - i;
        c[i] = i * 2;
        d[i] = i * 3;
        x[i] = (float)i;
        y[i] = (float)(size - i);
    }
    
    /* Call processing functions multiple times */
    for (int iter = 0; iter < 10; iter++) {
        process_loop(a, b, c, d, size);
        process_loop2(x, y, size);
        
        /* Mix up data to prevent pattern recognition */
        volatile_source = iter;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int sum = 0;
    float fsum = 0.0f;
    for (int i = 0; i < size; i++) {
        sum += a[i] + b[i] + c[i] + d[i];
        fsum += x[i] + y[i];
    }
    
    /* Use results (prevents optimization) */
    if (sum > 0 && fsum > 0.0f) {
        __builtin_printf("Result: %d, %f\n", sum, fsum);
    }
    
    /* Cleanup */
    __builtin_free(a);
    __builtin_free(b);
    __builtin_free(c);
    __builtin_free(d);
    __builtin_free(x);
    __builtin_free(y);
    
    return 0;
}
