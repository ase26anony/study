/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(int x) {
    return x * 3 + 7;
}

/* Volatile read to prevent optimization */
static volatile int volatile_source = 42;

/* Target function with complex loop carrying multiple dependency types */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Initialize with volatile to prevent dead code elimination */
    int init_val = volatile_source;
    
    /* Loop with multiple dependency patterns */
    for (i = 1; i < n; i++) {
        /* 1. FLOW DEPENDENCY (RAW) within iteration */
        int temp = a[i-1] + b[i];      /* Read a[i-1] */
        a[i] = temp * 2;               /* Write a[i] - flow dep from previous stmt */
        
        /* 2. ANTI DEPENDENCY (WAR) */
        int read_before_write = c[i];  /* Read c[i] */
        c[i] = get_value(i);           /* Write c[i] - anti dep from previous read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) on 'd' */
        d[i] = read_before_write + init_val;  /* First write to d[i] */
        d[i] = d[i] * 3 - i;                  /* Second write to d[i] - output dep */
        
        /* 4. LOOP-CARRIED FLOW DEPENDENCY (distance > 0) */
        b[i] = b[i-1] + a[i];          /* Flow from iteration i-1 to i */
        
        /* 5. Complex expression to prevent simplification */
        init_val = (init_val + i) & 0xFF;
    }
    
    /* Cross-iteration anti dependency */
    for (i = 0; i < n-1; i++) {
        /* Read after write across iterations */
        int val = a[i+1];              /* Read a[i+1] written in previous iteration */
        b[i] = val + c[i];             /* Anti dependency across iterations */
    }
}

/* Another function with different pattern to increase coverage */
void __attribute__((noinline))
process_loop2(float *restrict x, float *restrict y, int n) {
    int i;
    
    /* Initialize with volatile */
    float acc = (float)volatile_source;
    
    /* Loop with floating point operations and dependencies */
    for (i = 1; i < n; i++) {
        /* Flow dependency chain */
        float t1 = x[i-1] * 1.5f;
        float t2 = t1 + y[i];
        x[i] = t2 * 0.8f;
        
        /* Anti dependency */
        float old_y = y[i];
        y[i] = acc + (float)i;
        acc = old_y * 0.9f;  /* Use after read */
        
        /* Output dependency */
        float tmp = x[i] + y[i];
        tmp = tmp * tmp;  /* WAW on tmp */
        
        /* Loop-carried dependency */
        y[i] = y[i-1] * 0.7f + tmp;
    }
}

/* Main function with runtime-determined loop bounds */
int main(int argc, char *argv[]) {
    /* Use command line or volatile to prevent constant folding */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;  /* Ensure non-trivial loop */
    }
    
    /* Allocate arrays with restrict to help alias analysis */
    int *a = __builtin_malloc(n * sizeof(int));
    int *b = __builtin_malloc(n * sizeof(int));
    int *c = __builtin_malloc(n * sizeof(int));
    int *d = __builtin_malloc(n * sizeof(int));
    float *x = __builtin_malloc(n * sizeof(float));
    float *y = __builtin_malloc(n * sizeof(float));
    
    if (!a || !b || !c || !d || !x || !y) return 1;
    
    /* Initialize with pattern */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
        d[i] = i * 4;
        x[i] = (float)i * 0.5f;
        y[i] = (float)i * 0.25f;
    }
    
    /* Call processing functions multiple times */
    for (int iter = 0; iter < 10; iter++) {
        process_loop(a, b, c, d, n);
        process_loop2(x, y, n);
        
        /* Modify inputs slightly to prevent complete optimization */
        a[0] += iter;
        x[0] += (float)iter;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int sum_int = 0;
    float sum_float = 0.0f;
    for (int i = 0; i < n; i++) {
        sum_int += a[i] + b[i] + c[i] + d[i];
        sum_float += x[i] + y[i];
    }
    
    /* Use results (prevents optimization) */
    volatile int result = sum_int + (int)sum_float;
    
    __builtin_free(a);
    __builtin_free(b);
    __builtin_free(c);
    __builtin_free(d);
    __builtin_free(x);
    __builtin_free(y);
    
    return result != 0 ? 0 : 1;
}
