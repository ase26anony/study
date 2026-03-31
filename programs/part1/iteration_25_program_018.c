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
process_loop(int *restrict arr_a, int *restrict arr_b, int *restrict arr_c, 
             int n, int init_val) {
    int i;
    
    /* Initialize with external value to prevent dead code elimination */
    int temp = init_val;
    
    /* Loop with multiple dependency patterns */
    for (i = 1; i < n; i++) {
        /* FLOW (RAW) dependency: Read arr_a[i-1] after it was written in previous iteration */
        int flow_val = arr_a[i-1] + arr_b[i];  /* RAW: depends on arr_a[i-1] from prev iteration */
        
        /* ANTI (WAR) dependency: Read arr_c[i] then write to it */
        int anti_read = arr_c[i];              /* Read arr_c[i] */
        arr_c[i] = flow_val * 2;               /* WAR: Write to arr_c[i] after reading it */
        
        /* OUTPUT (WAW) dependency: Two writes to arr_a[i] */
        int opaque_val = get_value();          /* Prevent constant propagation */
        arr_a[i] = flow_val + opaque_val;      /* First write to arr_a[i] */
        
        /* Introduce volatile to prevent reordering */
        int volatile_read = volatile_source;
        arr_a[i] = anti_read + volatile_read;  /* WAW: Second write to arr_a[i] (overwrites first) */
        
        /* Another FLOW dependency within same iteration */
        arr_b[i] = arr_a[i] + 1;               /* RAW: depends on arr_a[i] written above */
        
        /* Cross-iteration ANTI dependency */
        int prev_c = arr_c[i-1];               /* Read from previous iteration's arr_c */
        arr_c[i-1] = prev_c + arr_b[i];        /* WAR: Write to arr_c[i-1] after reading it */
    }
}

/* Alternative simpler loop that still creates multiple dependencies */
void __attribute__((noinline))
simple_ddg_loop(int *a, int *b, int n) {
    int i;
    
    /* Initialize to prevent undefined behavior */
    a[0] = b[0] = 1;
    
    for (i = 1; i < n; i++) {
        /* Loop-carried FLOW dependency (distance = 1) */
        a[i] = a[i-1] + b[i];      /* RAW: a[i-1] → a[i] */
        
        /* ANTI dependency within same iteration */
        int temp = b[i];           /* Read b[i] */
        b[i] = a[i] * 2;           /* WAR: b[i] read then written */
        
        /* OUTPUT dependency */
        a[i] = temp + i;           /* WAW: a[i] written again */
        
        /* Memory dependency with variant index */
        if (i > 2) {
            b[i-2] = a[i] + b[i-1]; /* Complex memory dependencies */
        }
    }
}

/* Main function with runtime-determined loop bounds */
int main(int argc, char *argv[]) {
    /* Use command line argument for loop bound to prevent constant folding */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;  /* Ensure minimum size */
    }
    
    /* Allocate arrays with restrict to help alias analysis */
    int *arr_a = (int*)malloc(n * sizeof(int));
    int *arr_b = (int*)malloc(n * sizeof(int));
    int *arr_c = (int*)malloc(n * sizeof(int));
    
    if (!arr_a || !arr_b || !arr_c) return 1;
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        arr_a[i] = i;
        arr_b[i] = i * 2;
        arr_c[i] = i * 3;
    }
    
    /* Call both loop functions to increase coverage chances */
    process_loop(arr_a, arr_b, arr_c, n, argc);
    simple_ddg_loop(arr_a, arr_b, n);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += arr_a[i] + arr_b[i] + arr_c[i];
    }
    
    /* Use checksum in output */
    printf("Result checksum: %d\n", checksum);
    
    free(arr_a);
    free(arr_b);
    free(arr_c);
    
    return 0;
}
