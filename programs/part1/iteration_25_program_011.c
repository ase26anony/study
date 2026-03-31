/* test_ddg.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(void) {
    static int counter = 0;
    return ++counter;
}

/* Volatile read to prevent optimization */
static volatile int volatile_source = 42;

/* Target function with carefully constructed data dependencies */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Initialize with volatile to prevent dead code elimination */
    int init_val = volatile_source;
    a[0] = init_val;
    b[0] = init_val + 1;
    
    /* Main loop with multiple dependency types */
    for (i = 1; i < n; i++) {
        /* 1. FLOW DEPENDENCY (RAW) with distance 1 */
        /* Read a[i-1] written in previous iteration */
        int temp = a[i-1] + get_value();  /* Loop-carried flow dep */
        
        /* 2. ANTI DEPENDENCY (WAR) within same iteration */
        /* Read b[i] before overwriting it */
        c[i] = b[i] * 2;                  /* Anti dep source */
        b[i] = temp + i;                  /* Anti dep sink */
        
        /* 3. OUTPUT DEPENDENCY (WAW) within same iteration */
        /* Multiple writes to same location */
        d[i] = c[i] + temp;               /* First write to d[i] */
        d[i] = d[i] * 3;                  /* Second write to d[i] - output dep */
        
        /* 4. Another FLOW DEPENDENCY (RAW) within iteration */
        /* Use result immediately */
        a[i] = d[i] / 2;                  /* Flow dep from d[i] assignment */
        
        /* 5. Memory dependency with variant index */
        /* Create complex addressing to prevent simplification */
        int idx = (i * 7) % n;
        if (idx > 0) {
            /* Cross-iteration memory dependency */
            b[idx] = b[idx-1] + a[i];     /* Another loop-carried dep */
        }
        
        /* 6. Additional anti dependency pattern */
        int saved = c[i];                  /* Read c[i] */
        c[i] = b[i] + saved;              /* Write c[i] - anti dep */
    }
    
    /* Final store with volatile to ensure side effect */
    volatile_source = a[n-1];
}

/* Helper to prevent dead code elimination */
static int __attribute__((noinline)) 
compute_checksum(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum ^= arr[i];  /* Use XOR to prevent easy optimization */
    }
    return sum;
}

int main(int argc, char **argv) {
    /* Use command line argument for loop bound to prevent constant folding */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = 1000;
        if (n > 100000) n = 100000;  /* Prevent excessive memory */
    }
    
    /* Allocate arrays with restrict to help alias analysis */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    int *d = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < n; i++) {
        a[i] = i * 3;
        b[i] = i * 5;
        c[i] = i * 7;
        d[i] = i * 11;
    }
    
    /* Call the target function with the complex loop */
    process_loop(a, b, c, d, n);
    
    /* Compute checksum to ensure computations aren't optimized away */
    int checksum = compute_checksum(a, n);
    checksum ^= compute_checksum(b, n);
    checksum ^= compute_checksum(c, n);
    checksum ^= compute_checksum(d, n);
    
    /* Use result to prevent dead code elimination */
    printf("Result checksum: %d\n", checksum);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
