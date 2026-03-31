/* test_ddg.c - Program to trigger DDG edge creation in GCC */

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(int x) {
    return x ^ 0x55;
}

/* Volatile read to prevent optimization */
static volatile int volatile_source = 42;

/* Target function with complex data dependencies */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Loop with multiple dependency types */
    for (i = 1; i < n; i++) {
        /* 1. FLOW dependency (RAW): c[i] depends on a[i] */
        int temp = a[i] + volatile_source;  /* volatile prevents const prop */
        c[i] = temp * 2;
        
        /* 2. ANTI dependency (WAR): a[i] written after being read above */
        /*    and loop-carried FLOW: a[i] depends on a[i-1] */
        a[i] = b[i] + a[i-1];  /* Loop-carried flow dependency */
        
        /* 3. OUTPUT dependency (WAW): d[i] written twice */
        d[i] = c[i] + 1;       /* First write to d[i] */
        
        /* 4. Another FLOW: e depends on d[i] */
        int e = d[i] * 3;
        
        /* Second write to d[i] - creates output dependency with line above */
        d[i] = e + get_value(i);  /* Function call prevents optimization */
        
        /* 5. Memory anti-dependency with array b */
        /*    Read b[i] then modify it in next iteration via b[i-1] assignment */
        if (i < n-1) {
            /* This creates anti-dependency through b array */
            b[i+1] = b[i] + i;  /* b[i] read, b[i+1] written - no direct dep */
        }
    }
}

/* Another function with cross-iteration dependencies */
void __attribute__((noinline))
process_loop2(int *arr, int n) {
    int i;
    
    /* Different pattern: chain of dependencies */
    for (i = 2; i < n; i++) {
        /* Multiple interleaved dependencies */
        arr[i] = arr[i-1] + arr[i-2];  /* Two loop-carried flow deps */
        
        /* Create anti-dependency by reading then modifying */
        int tmp = arr[i] >> 1;
        arr[i-1] = tmp + i;  /* WAR with next iteration's arr[i-1] read */
    }
}

/* Main driver */
int main(int argc, char *argv[]) {
    /* Use command line arg for loop bound to prevent constant folding */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;  /* Ensure minimum size */
    }
    
    /* Allocate arrays with restrict to help alias analysis */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    int *d = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d) return 1;
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        a[i] = i * 3;
        b[i] = i * 5;
        c[i] = i * 7;
        d[i] = i * 11;
    }
    
    /* Call the processing functions multiple times 
       to give optimizer more chances */
    for (int iter = 0; iter < 10; iter++) {
        process_loop(a, b, c, d, n);
        process_loop2(a, n);
    }
    
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
