/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */

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

/* Target function with carefully constructed data dependencies */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Initialize with volatile to prevent constant folding */
    int init = volatile_read();
    a[0] = init;
    b[0] = init + 1;
    
    /* Main loop with multiple dependency types */
    for (i = 1; i < n; i++) {
        /* 1. FLOW DEPENDENCY (RAW) within same iteration */
        int temp = a[i-1] + get_value();  /* Read a[i-1] */
        b[i] = temp * 2;                  /* Write b[i] */
        
        /* 2. ANTI DEPENDENCY (WAR) */
        int read_before_write = b[i];     /* Read b[i] */
        a[i] = read_before_write + c[i];  /* Write a[i] - WAR on b[i] */
        
        /* 3. OUTPUT DEPENDENCY (WAW) on 'temp' variable */
        temp = d[i] * 3;                  /* Re-write temp - WAW */
        
        /* 4. FLOW DEPENDENCY with distance > 0 (loop-carried) */
        c[i] = c[i-1] + temp;             /* Flow with distance=1 */
        
        /* 5. Complex memory dependency pattern */
        d[i] = (a[i] + b[i-1]) * c[i];    /* Multiple dependencies */
        
        /* 6. Another anti dependency pattern */
        int x = d[i];                     /* Read d[i] */
        d[i] = x + i;                     /* Write d[i] - WAR on d[i] */
    }
    
    /* Final output dependency to prevent dead code elimination */
    a[n-1] = b[n-1] + c[n-1];
}

/* Alternative simpler but effective version */
void __attribute__((noinline))
simple_ddg_loop(int *arr, int n) {
    int i;
    
    /* Create all three dependency types in minimal form */
    for (i = 1; i < n; i++) {
        /* FLOW (RAW): arr[i] depends on arr[i-1] */
        int flow_val = arr[i-1] + volatile_read();
        
        /* ANTI (WAR): Read then write same location */
        int anti_read = arr[i];
        arr[i] = flow_val + i;            /* WAR on arr[i] */
        
        /* OUTPUT (WAW): Multiple writes to same variable */
        int output_var = anti_read * 2;
        output_var = output_var + 1;      /* WAW on output_var */
        
        /* Loop-carried flow with distance */
        arr[i] = arr[i] + output_var + arr[i-1]; /* RAW on arr[i-1] */
    }
}

/* Main function with runtime-determined loop bound */
int main(int argc, char *argv[]) {
    /* Use command line argument for loop bound to prevent constant folding */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;  /* Ensure sufficient iterations */
    }
    
    /* Allocate arrays with restrict to help alias analysis */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    int *d = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d) return 1;
    
    /* Initialize with non-constant values */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
        d[i] = i * 4;
    }
    
    /* Call the loop processing functions */
    process_loop(a, b, c, d, n);
    simple_ddg_loop(a, n);
    
    /* Compute checksum to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += a[i] + b[i] + c[i] + d[i];
    }
    
    /* Use the result */
    printf("Result checksum: %d\n", sum);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
