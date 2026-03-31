/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(int x) {
    return x * 3 + 1;
}

/* Volatile read to prevent dead code elimination */
static volatile int volatile_source = 7;

/* Target function with carefully constructed data dependencies */
void __attribute__((noinline)) process_loop(int *arr, int *brr, int *crr, int n) {
    int i;
    
    /* Loop with multiple dependency types */
    for (i = 1; i < n; i++) {
        /* 1. FLOW DEPENDENCY (RAW) between statements */
        int temp = arr[i] + brr[i];          /* Statement A: write temp */
        crr[i] = temp * 2;                   /* Statement B: read temp */
        
        /* 2. ANTI DEPENDENCY (WAR) */
        int old_val = arr[i];                /* Statement C: read arr[i] */
        arr[i] = get_value(brr[i]);          /* Statement D: write arr[i] */
        
        /* 3. OUTPUT DEPENDENCY (WAW) on crr */
        int volatile_read = volatile_source; /* Volatile to prevent elimination */
        crr[i] = old_val + volatile_read;    /* Statement E: write crr[i] again */
        
        /* 4. LOOP-CARRIED FLOW DEPENDENCY (distance > 0) */
        brr[i] = brr[i-1] + crr[i];          /* Statement F: read brr[i-1] from prev iteration */
        
        /* 5. Additional flow dependency chain */
        arr[i] = arr[i] + temp;              /* Statement G: read temp again */
    }
}

/* Another loop with different patterns */
void __attribute__((noinline)) process_loop2(int *a, int *b, int *c, int n) {
    int i;
    
    /* Loop with array accesses creating memory dependencies */
    for (i = 2; i < n - 1; i++) {
        /* Complex dependency web */
        int t1 = a[i] + b[i-1];      /* Flow from b[i-1] (loop-carried) */
        int t2 = c[i+1] - t1;        /* Flow from t1, anti from c[i+1]? */
        
        b[i] = t2 * 3;               /* Output on b[i], flow from t2 */
        c[i] = a[i-2] + b[i];        /* Flow from a[i-2] (distance=2), flow from b[i] */
        
        a[i] = t1 + t2;              /* Output on a[i], flow from t1 & t2 */
        
        /* Introduce control flow to prevent if-conversion */
        if (b[i] > 100) {
            c[i] = c[i] / 2;
        }
    }
}

int main(int argc, char **argv) {
    int n = 1000;
    
    /* Use command line argument to prevent constant trip count */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;  /* Ensure minimum size */
    }
    
    /* Allocate arrays with volatile to prevent prefetching optimizations */
    volatile int *arr = (int*)malloc(n * sizeof(int));
    volatile int *brr = (int*)malloc(n * sizeof(int));
    volatile int *crr = (int*)malloc(n * sizeof(int));
    
    if (!arr || !brr || !crr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        arr[i] = i * 2;
        brr[i] = i + 1;
        crr[i] = i * 3;
    }
    
    /* Call the target functions multiple times */
    process_loop((int*)arr, (int*)brr, (int*)crr, n);
    process_loop2((int*)arr, (int*)brr, (int*)crr, n);
    
    /* Compute checksum to prevent dead code elimination */
    long long sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i] + brr[i] + crr[i];
    }
    
    printf("Result checksum: %lld\n", sum);
    
    free((void*)arr);
    free((void*)brr);
    free((void*)crr);
    
    return 0;
}
