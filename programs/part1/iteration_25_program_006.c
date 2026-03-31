/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(void) {
    static int counter = 0;
    return ++counter;
}

/* Volatile access functions */
static int volatile_read(void) {
    volatile int v = 42;
    return v;
}

static void volatile_write(int *p) {
    volatile int *vp = (volatile int *)p;
    *vp = *vp + 1;
}

/* Main processing function with carefully constructed loop */
void process(int *arr, int *brr, int *crr, int n) {
    int i;
    
    /* Initialize with volatile to prevent dead code elimination */
    int init = volatile_read();
    
    /* Loop with multiple dependency types */
    for (i = 1; i < n; i++) {
        /* FLOW dependency (RAW): crr[i] depends on arr[i-1] from previous iteration */
        int temp = arr[i-1] + get_value();  /* Loop-carried flow dependency */
        
        /* ANTI dependency (WAR): brr[i] read before arr[i] write */
        int anti_temp = brr[i] * 2;         /* Read brr[i] */
        arr[i] = temp + anti_temp;          /* Write arr[i] - creates WAR with next iteration's brr[i] read */
        
        /* OUTPUT dependency (WAW): Multiple writes to crr[i] */
        crr[i] = temp * 3;                  /* First write to crr[i] */
        
        /* Another OUTPUT dependency */
        if (anti_temp > 100) {
            crr[i] = anti_temp / 2;         /* Second write to crr[i] - WAW */
        }
        
        /* FLOW dependency within same iteration */
        brr[i] = crr[i] + init;             /* RAW: depends on crr[i] write above */
        
        /* Memory dependency with variant index */
        int idx = i % 10;
        arr[idx] = brr[i] - arr[i];         /* Complex memory dependency pattern */
    }
}

/* Another function with different patterns */
void process2(float *a, float *b, float *c, int n) {
    int i;
    
    /* Loop with cross-iteration dependencies */
    for (i = 2; i < n; i++) {
        /* Strong loop-carried flow dependency chain */
        float t1 = a[i-1] * b[i-2];         /* Distance = 1 and 2 dependencies */
        float t2 = c[i] + t1;
        
        /* Anti-dependency pattern */
        float old_a = a[i];                 /* Read a[i] */
        a[i] = t2 * old_a;                  /* Write a[i] - WAR */
        
        /* Output dependency */
        b[i] = t1 + t2;
        b[i] = old_a - b[i];                /* WAW on b[i] */
        
        /* Another flow dependency */
        c[i] = a[i] * b[i] + c[i-1];        /* Loop-carried on c[] */
    }
}

/* Main function to drive execution */
int main(int argc, char *argv[]) {
    /* Use command line argument to prevent constant trip count */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;  /* Ensure minimum size */
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
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        arr[i] = i;
        brr[i] = i * 2;
        crr[i] = i * 3;
        fa[i] = i * 1.5f;
        fb[i] = i * 2.5f;
        fc[i] = i * 0.5f;
    }
    
    /* Call processing functions multiple times */
    for (int iter = 0; iter < 10; iter++) {
        process(arr, brr, crr, n);
        process2(fa, fb, fc, n);
        
        /* Mix in some volatile operations to prevent optimization */
        volatile_write(&arr[iter % n]);
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
