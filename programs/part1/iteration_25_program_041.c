/* test_ddg.c - Program to trigger DDG edge creation in GCC */

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(int x) {
    return x * 3 + 1;
}

/* Volatile read to prevent dead code elimination */
static int volatile_read(void) {
    static volatile int v = 7;
    return v;
}

/* Main processing function with complex data dependencies */
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
        /* 1. FLOW (RAW) dependency: a[i] depends on b[i-1] */
        int temp = get_value(b[i-1]);
        
        /* 2. OUTPUT (WAW) dependency: Multiple writes to same location */
        a[i] = temp + c[i];          /* First write to a[i] */
        
        /* 3. ANTI (WAR) dependency: Read a[i] before overwriting it */
        int read_a = a[i] + d[i];    /* Read a[i] */
        
        /* 4. Second write to a[i] - creates OUTPUT dependency with line 1 */
        a[i] = read_a * 2;           /* Second write to a[i] */
        
        /* 5. FLOW dependency with distance 1: b[i] depends on a[i-1] */
        b[i] = a[i-1] + i;
        
        /* 6. Cross-iteration FLOW dependency with distance > 0 */
        c[i] = c[i-1] + b[i];        /* Distance = 1 */
        
        /* 7. Complex memory dependency with variant index */
        int idx = i % 10;
        d[idx] = d[idx] + a[i];      /* Could create memory dependencies */
    }
    
    /* 8. Additional loop with different patterns */
    for (i = 0; i < n - 1; i++) {
        /* Loop-carried FLOW dependency with distance 1 */
        a[i+1] = a[i] + b[i];
        
        /* ANTI dependency within same iteration */
        int old_b = b[i];
        b[i] = c[i] * old_b;         /* WAR: b[i] written after reading old_b */
        
        /* OUTPUT dependency */
        c[i] = i * 2;
        c[i] = c[i] + 1;             /* WAW: Second write to c[i] */
    }
}

/* Another function with pointer aliasing to create memory dependencies */
void __attribute__((noinline))
process_with_aliasing(int *arr, int n) {
    int i;
    int *p = arr;
    int *q = arr + 1;
    
    for (i = 0; i < n - 1; i++) {
        /* Potential memory dependencies due to pointer aliasing */
        *p = *q + i;      /* Write to arr[i] */
        p++;
        q++;
        
        /* FLOW dependency with pointer arithmetic */
        arr[i+1] = arr[i] * 2 - 1;
    }
}

/* Main function with runtime-determined loop bounds */
int main(int argc, char *argv[]) {
    /* Use command line argument to prevent constant trip count */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;  /* Ensure minimum size */
        if (n > 10000) n = 10000; /* Limit for safety */
    }
    
    /* Allocate arrays with restrict to help alias analysis */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    int *d = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d) {
        return 1;
    }
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
        d[i] = i * 4;
    }
    
    /* Call processing functions multiple times */
    process_loop(a, b, c, d, n);
    process_with_aliasing(a, n);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += a[i] + b[i] + c[i] + d[i];
    }
    
    /* Use checksum in output */
    printf("Checksum: %d\n", checksum % 1000);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
