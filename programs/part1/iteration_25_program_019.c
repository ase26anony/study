/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(void) {
    static int counter = 0;
    return ++counter;
}

/* Volatile access functions to prevent optimization */
static volatile int vol_source;
static void __attribute__((noinline)) volatile_write(int val) {
    vol_source = val;
}
static int __attribute__((noinline)) volatile_read(void) {
    return vol_source;
}

/* Main processing function with complex loop dependencies */
void __attribute__((noinline)) 
process_loop(int *arr_a, int *arr_b, int *arr_c, int n) {
    int i;
    
    /* Initialize with volatile to prevent constant folding */
    int base = volatile_read();
    
    /* Complex loop with multiple dependency types */
    for (i = 1; i < n; i++) {
        /* 1. FLOW DEPENDENCY (RAW) within iteration */
        int temp = arr_a[i-1] + base;      /* Read arr_a[i-1] */
        arr_c[i] = temp * 2;               /* Write arr_c[i] */
        
        /* 2. ANTI DEPENDENCY (WAR) within iteration */
        int old_val = arr_b[i];            /* Read arr_b[i] */
        arr_b[i] = get_value();            /* Write arr_b[i] - anti-dep on line above */
        
        /* 3. OUTPUT DEPENDENCY (WAW) within iteration */
        arr_a[i] = old_val + 1;            /* Write arr_a[i] - first write */
        arr_a[i] = arr_a[i] * 3;           /* Write arr_a[i] again - output dep */
        
        /* 4. LOOP-CARRIED FLOW DEPENDENCY (distance=1) */
        arr_c[i] = arr_c[i-1] + arr_b[i];  /* Flow from iteration i-1 to i */
        
        /* 5. Additional anti-dependency with memory */
        int tmp2 = arr_a[i];               /* Read arr_a[i] */
        arr_a[i] = tmp2 + arr_c[i];        /* Write arr_a[i] - anti-dep */
        
        /* 6. Complex expression to prevent simplification */
        arr_b[i] = (arr_b[i] & 0xFF) | ((arr_a[i] << 8) & 0xFF00);
    }
    
    /* Final store with volatile to ensure side effect */
    volatile_write(arr_a[n-1]);
}

/* Another function with different pattern to increase coverage */
void __attribute__((noinline))
process_loop2(int *arr_x, int *arr_y, int n) {
    int i;
    
    /* Loop with stride access pattern */
    for (i = 2; i < n; i++) {
        /* Multiple interleaved dependencies */
        int t1 = arr_x[i-2];               /* Read with distance=2 */
        int t2 = arr_y[i-1];               /* Read with distance=1 */
        
        /* Create output dependency chain */
        arr_x[i] = t1 + t2;                /* Write arr_x[i] */
        arr_x[i] = arr_x[i] * 7;           /* Output dependency */
        
        /* Anti-dependency chain */
        int t3 = arr_y[i];                 /* Read arr_y[i] */
        arr_y[i] = t3 ^ arr_x[i];          /* Write arr_y[i] - anti-dep */
        
        /* Another flow dependency */
        arr_y[i+1] = arr_x[i] + 1;         /* Flow to next iteration */
    }
}

/* Main driver with runtime-determined loop bounds */
int main(int argc, char *argv[]) {
    /* Use command line argument for loop bound to prevent constant folding */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;  /* Ensure minimum size */
        if (n > 10000) n = 10000; /* Limit for safety */
    }
    
    /* Dynamically allocate arrays to force memory dependencies */
    int *arr1 = (int*)malloc(n * sizeof(int));
    int *arr2 = (int*)malloc(n * sizeof(int));
    int *arr3 = (int*)malloc(n * sizeof(int));
    
    if (!arr1 || !arr2 || !arr3) return 1;
    
    /* Initialize with pattern */
    for (int i = 0; i < n; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
    }
    
    /* Call processing functions multiple times */
    for (int iter = 0; iter < 10; iter++) {
        process_loop(arr1, arr2, arr3, n);
        process_loop2(arr1, arr3, n);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i];
    }
    
    /* Use checksum in output */
    printf("Checksum: %d\n", checksum);
    
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
