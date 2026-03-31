/* test_ddg.c - Program to trigger GCC's DDG edge creation */
#include <stdio.h>
#include <stdlib.h>

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

/* Target function with complex loop carrying multiple dependency types */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Initialize with volatile to prevent dead code elimination */
    int init_val = volatile_read();
    
    /* Loop with carefully crafted dependencies */
    for (i = 1; i < n; i++) {
        /* 1. FLOW DEPENDENCY (RAW) within iteration */
        int temp = a[i-1] + init_val;      /* Read a[i-1] */
        b[i] = temp * 2;                   /* Write b[i] */
        
        /* 2. FLOW DEPENDENCY (RAW) cross-iteration with distance=1 */
        a[i] = b[i-1] + get_value();       /* Read b[i-1], Write a[i] */
        
        /* 3. ANTI DEPENDENCY (WAR) */
        int read_c = c[i];                  /* Read c[i] */
        c[i] = temp + read_c;               /* Write c[i] after read */
        
        /* 4. OUTPUT DEPENDENCY (WAW) on 'd' */
        d[i] = read_c * 3;                  /* Write d[i] */
        d[i] = d[i] + a[i];                 /* Overwrite d[i] - WAW */
        
        /* 5. Additional FLOW dependency chain */
        b[i] = b[i] + c[i-1];               /* Read c[i-1], Update b[i] */
    }
    
    /* Prevent tail elimination */
    a[0] = volatile_read();
}

/* Alternate loop with memory aliasing to create more complex dependencies */
void __attribute__((noinline))
process_loop2(int *arr, int n) {
    int i;
    
    /* Loop with pointer-based dependencies */
    for (i = 2; i < n - 1; i++) {
        /* Multiple interleaved dependencies */
        int *p1 = &arr[i];
        int *p2 = &arr[i-1];
        int *p3 = &arr[i-2];
        
        /* Flow (RAW) with distance 1 */
        int val1 = *p2 + get_value();
        *p1 = val1;
        
        /* Flow (RAW) with distance 2 */
        int val2 = *p3 + val1;
        arr[i+1] = val2;
        
        /* Anti (WAR) */
        int read_val = arr[i];
        arr[i] = read_val * 2;
        
        /* Output (WAW) */
        arr[i-1] = val2;
        arr[i-1] = arr[i-1] + 1;
    }
}

int main(int argc, char **argv) {
    int n = 1000;
    
    /* Use command line or default to prevent constant folding */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;  /* Ensure sufficient iterations */
    }
    
    /* Allocate arrays with restrict to help alias analysis */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    int *d = (int*)malloc(n * sizeof(int));
    int *arr = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d || !arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
        d[i] = i * 4;
        arr[i] = i * 5;
    }
    
    /* Call both loop functions to increase coverage chances */
    process_loop(a, b, c, d, n);
    process_loop2(arr, n);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long sum = 0;
    for (int i = 0; i < n; i++) {
        sum += a[i] + b[i] + c[i] + d[i] + arr[i];
    }
    
    printf("Checksum: %llu (n=%d)\n", sum, n);
    
    free(a);
    free(b);
    free(c);
    free(d);
    free(arr);
    
    return 0;
}
