/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(int x) {
    return x * 3 + 7;
}

/* Function with complex loop dependencies */
void __attribute__((noinline)) 
process_array(int *restrict a, int *restrict b, int *restrict c, int n) {
    int i;
    
    /* Initialize with some values */
    for (i = 0; i < n; i++) {
        a[i] = i;
        b[i] = i * 2;
    }
    
    /* Main loop with multiple dependency types */
    for (i = 1; i < n - 1; i++) {
        /* 1. FLOW dependency (RAW): a[i] depends on previous computation */
        int temp = a[i] + b[i];          /* Statement S1 */
        
        /* 2. ANTI dependency (WAR): c[i] read before a[i] write */
        int old_val = c[i];              /* Statement S2 - read c[i] */
        
        /* 3. OUTPUT dependency (WAW): a[i] written twice */
        a[i] = temp * 2;                 /* Statement S3 - first write to a[i] */
        
        /* 4. Another FLOW dependency using temp */
        c[i] = old_val + temp;           /* Statement S4 - write c[i] */
        
        /* 5. Loop-carried FLOW dependency with distance 1 */
        a[i] = a[i - 1] + c[i];          /* Statement S5 - second write to a[i] (WAW with S3) */
        
        /* 6. Loop-carried ANTI dependency with distance 1 */
        b[i + 1] = b[i] + get_value(a[i]); /* Statement S6 - WAR on b[i] from next iteration */
    }
    
    /* Final computation with cross-iteration dependency */
    for (i = 2; i < n; i++) {
        /* Complex dependency chain */
        c[i] = a[i - 1] + a[i - 2] + b[i];  /* Multiple loop-carried dependencies */
    }
}

/* Alternative: Loop with memory dependencies through pointers */
void __attribute__((noinline))
process_with_pointers(int *arr, int n) {
    int *p = arr;
    int *q = arr + 1;
    int *r = arr + 2;
    
    for (int i = 0; i < n - 2; i++) {
        /* Create all dependency types through pointer aliasing */
        int x = *p;          /* Read *p - will create dependencies */
        *q = x + i;          /* Write *q - FLOW from x */
        int y = *q;          /* Read *q - ANTI with previous write */
        *p = y * 2;          /* Write *p - OUTPUT if p==q, otherwise FLOW */
        *r = *p + *q;        /* Multiple reads */
        
        p++; q++; r++;       /* Pointer movement */
    }
}

/* Use volatile to prevent dead code elimination */
static volatile int global_seed = 42;

int main(int argc, char **argv) {
    /* Use command line argument for non-constant loop bound */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;  /* Ensure sufficient iterations */
    }
    
    /* Add some randomness to prevent compile-time computation */
    n += (global_seed % 10);
    
    /* Allocate arrays */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c) return 1;
    
    /* Initialize with non-constant values */
    for (int i = 0; i < n; i++) {
        a[i] = i + global_seed;
        b[i] = i * 3 - global_seed;
        c[i] = 0;
    }
    
    /* Call the function with dependencies */
    process_array(a, b, c, n);
    
    /* Also call pointer version */
    process_with_pointers(a, n);
    
    /* Compute checksum to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    /* Use the result */
    printf("Result checksum: %d\n", sum % 1000);
    
    free(a);
    free(b);
    free(c);
    
    return 0;
}
