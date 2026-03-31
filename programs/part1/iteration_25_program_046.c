/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(void) {
    static int counter = 0;
    return ++counter;
}

/* Volatile access functions to prevent optimization */
static volatile int vol_var;
static int volatile_read(void) {
    return vol_var;
}
static void volatile_write(int val) {
    vol_var = val;
}

/* Target function with complex loop carrying multiple dependency types */
void __attribute__((noinline)) 
process_loop(int *restrict arr_a, int *restrict arr_b, 
             int *restrict arr_c, int n) {
    int i;
    
    /* Initialize with volatile to prevent dead code elimination */
    int init_val = volatile_read();
    
    /* Loop with carefully crafted dependencies */
    for (i = 1; i < n; i++) {
        /* 1. FLOW dependency (RAW): arr_a[i] depends on arr_a[i-1] from previous iteration */
        /*    This creates edge with distance = 1 */
        int temp = arr_a[i-1] + get_value();
        
        /* 2. ANTI dependency (WAR): arr_b[i] read before write in same iteration */
        /*    arr_b[i] is read here... */
        int read_b = arr_b[i] + temp;
        
        /*    ...and written here, creating anti-dependency */
        arr_b[i] = read_b * 2;
        
        /* 3. OUTPUT dependency (WAW): arr_c written twice in same iteration */
        /*    First write to arr_c[i] */
        arr_c[i] = arr_a[i] + read_b;
        
        /*    Second write to arr_c[i] - output dependency with first write */
        arr_c[i] = arr_c[i] * 3 + get_value();
        
        /* 4. Another FLOW dependency within same iteration */
        /*    arr_a[i] calculation depends on temp */
        arr_a[i] = temp + arr_c[i];
        
        /* 5. Memory dependency with variant index (potential aliasing) */
        /*    This creates additional edges in DDG */
        if (i % 2 == 0) {
            arr_a[i/2] = arr_b[i] + 1;
        }
    }
}

/* Another loop with different patterns to increase coverage */
void __attribute__((noinline))
process_loop2(float *restrict farr, int *restrict iarr, int n) {
    int i;
    float acc = 0.0f;
    
    /* Loop with floating point operations and dependencies */
    for (i = 0; i < n; i++) {
        /* Flow dependency with distance 1 */
        float prev = (i > 0) ? farr[i-1] : 1.0f;
        
        /* Anti-dependency: read iarr[i] before modifying it */
        int ival = iarr[i];
        
        /* Complex calculation with multiple dependencies */
        farr[i] = prev * ival + acc;
        
        /* Output dependency: iarr written twice */
        iarr[i] = ival + 1;
        iarr[i] = iarr[i] * 2;
        
        /* Flow dependency within iteration */
        acc = farr[i] * 0.5f;
        
        /* Volatile operation to prevent reordering */
        volatile_write(i);
    }
}

/* Main function with runtime-determined loop bounds */
int main(int argc, char *argv[]) {
    /* Use command line argument for loop bound to prevent constant folding */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;  /* Ensure non-trivial loop */
    }
    
    /* Allocate arrays with volatile to prevent optimization */
    volatile int alloc_size = n;
    int *arr_a = (int*)malloc(alloc_size * sizeof(int));
    int *arr_b = (int*)malloc(alloc_size * sizeof(int));
    int *arr_c = (int*)malloc(alloc_size * sizeof(int));
    float *farr = (float*)malloc(alloc_size * sizeof(float));
    int *iarr = (int*)malloc(alloc_size * sizeof(int));
    
    if (!arr_a || !arr_b || !arr_c || !farr || !iarr) {
        return 1;
    }
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < n; i++) {
        arr_a[i] = i * 3;
        arr_b[i] = i * 5;
        arr_c[i] = i * 7;
        farr[i] = (float)i * 0.1f;
        iarr[i] = i * 2;
    }
    
    /* Call the loops multiple times to ensure execution */
    for (int iter = 0; iter < 3; iter++) {
        process_loop(arr_a, arr_b, arr_c, n);
        process_loop2(farr, iarr, n);
        
        /* Modify inputs slightly between iterations */
        volatile_write(iter);
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += arr_a[i] + arr_b[i] + arr_c[i] + (int)farr[i] + iarr[i];
    }
    
    /* Use checksum in output */
    printf("Checksum: %llu\n", checksum);
    
    free(arr_a);
    free(arr_b);
    free(arr_c);
    free(farr);
    free(iarr);
    
    return 0;
}
