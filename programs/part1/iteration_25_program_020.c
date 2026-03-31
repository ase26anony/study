/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(void) {
    static int counter = 0;
    return ++counter;
}

/* Volatile access functions to prevent optimization */
static volatile int vol_source = 42;
static int volatile_read(void) {
    return vol_source;
}
static void volatile_write(int val) {
    vol_source = val;
}

/* Main processing function with complex loop dependencies */
void __attribute__((noinline)) 
process_loop(int *arr, int *brr, int *crr, int n) {
    int i;
    
    /* Initialize with volatile to prevent dead code elimination */
    int init_val = volatile_read();
    
    /* Loop with multiple dependency types */
    for (i = 1; i < n; i++) {
        /* FLOW (RAW) dependency: crr[i] depends on arr[i] calculation */
        arr[i] = brr[i] + init_val + get_value();
        
        /* Another FLOW dependency with loop-carried (distance=1) */
        brr[i] = arr[i-1] * 2;  /* Cross-iteration dependency */
        
        /* ANTI (WAR) dependency: crr[i] read before arr[i] write */
        int temp = crr[i];      /* Read crr[i] */
        crr[i] = arr[i] + temp; /* Write crr[i] - creates ANTI with next iteration's read */
        
        /* OUTPUT (WAW) dependency: arr[i] written twice */
        arr[i] = crr[i] / 3;    /* Second write to arr[i] */
        
        /* Additional FLOW dependency with memory aliasing */
        if (i % 2 == 0) {
            brr[i] = crr[i] + brr[i/2];  /* Complex index pattern */
        }
        
        /* Volatile write to prevent reordering */
        volatile_write(i);
    }
    
    /* Loop-carried OUTPUT dependency */
    for (i = 2; i < n; i++) {
        arr[i] = arr[i-2] + 1;  /* Distance=2 WAW through arr */
    }
}

/* Another function with different patterns */
void __attribute__((noinline))
process_loop2(float *a, float *b, float *c, int n) {
    int i;
    
    /* Initialize */
    float acc = 0.1f;
    
    /* Reduction-like loop with flow dependencies */
    for (i = 0; i < n; i++) {
        /* Multiple interleaved dependencies */
        float t1 = a[i] + b[i];
        float t2 = t1 * c[i];      /* FLOW: depends on t1 */
        a[i] = t2 + acc;           /* FLOW: depends on t2, OUTPUT: writes a[i] */
        acc = a[i] * 0.5f;         /* FLOW: depends on a[i], loop-carried through acc */
        
        /* ANTI dependency pattern */
        float old_b = b[i];        /* Read b[i] */
        b[i] = t2 + old_b;         /* Write b[i] - ANTI with next iteration's read */
        
        /* Complex addressing for memory disambiguation */
        if (i > 0) {
            c[i] = c[i-1] + b[i];  /* Loop-carried FLOW through c */
        }
    }
}

/* Main driver */
int main(int argc, char **argv) {
    /* Use command line argument for variable loop bound */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;  /* Ensure sufficient iterations */
    }
    
    /* Allocate arrays with volatile to prevent optimization */
    volatile int size = n;
    int *arr = (int*)malloc(size * sizeof(int));
    int *brr = (int*)malloc(size * sizeof(int));
    int *crr = (int*)malloc(size * sizeof(int));
    float *fa = (float*)malloc(size * sizeof(float));
    float *fb = (float*)malloc(size * sizeof(float));
    float *fc = (float*)malloc(size * sizeof(float));
    
    if (!arr || !brr || !crr || !fa || !fb || !fc) {
        return 1;
    }
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < n; i++) {
        arr[i] = i % 7;
        brr[i] = (i * 3) % 11;
        crr[i] = (i + 5) % 13;
        fa[i] = i * 0.1f;
        fb[i] = i * 0.2f;
        fc[i] = i * 0.3f;
    }
    
    /* Call processing functions multiple times */
    for (int iter = 0; iter < 3; iter++) {
        process_loop(arr, brr, crr, n);
        process_loop2(fa, fb, fc, n);
        
        /* Modify inputs slightly */
        volatile_write(iter);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int sum = 0;
    float fsum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += arr[i] + brr[i] + crr[i];
        fsum += fa[i] + fb[i] + fc[i];
    }
    
    /* Use results */
    printf("Checksums: %d, %.2f\n", sum, fsum);
    
    free(arr);
    free(brr);
    free(crr);
    free(fa);
    free(fb);
    free(fc);
    
    return 0;
}
