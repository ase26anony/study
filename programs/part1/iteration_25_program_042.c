/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(void) {
    static int counter = 0;
    return ++counter;
}

/* Volatile read to prevent optimization */
static volatile int volatile_sink;

/* Target function with complex loop carrying multiple dependency types */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Initialize with opaque values */
    int init_val = get_value();
    a[0] = init_val;
    b[0] = init_val + 1;
    
    /* Main loop with carefully crafted dependencies */
    for (i = 1; i < n; i++) {
        /* 1. FLOW DEPENDENCY (RAW) within iteration */
        int temp = a[i-1] + b[i-1];  /* Read a[i-1], b[i-1] */
        c[i] = temp * 2;             /* Write c[i] */
        
        /* 2. ANTI DEPENDENCY (WAR) within iteration */
        int read_before_write = b[i];  /* Read b[i] */
        b[i] = c[i-1] + read_before_write;  /* Write b[i] */
        
        /* 3. OUTPUT DEPENDENCY (WAW) within iteration */
        a[i] = temp + i;              /* First write to a[i] */
        a[i] = a[i] * 3;              /* Second write to a[i] - WAW */
        
        /* 4. FLOW DEPENDENCY with distance > 0 (loop-carried) */
        d[i] = d[i-1] + a[i];         /* Flow from iteration i-1 to i */
        
        /* 5. Complex memory dependency with variant index */
        int idx = i % 10;
        volatile_sink = c[idx];       /* Volatile read to prevent elimination */
        
        /* 6. Additional anti dependency with memory */
        int temp2 = d[i];             /* Read d[i] */
        d[idx] = temp2 + volatile_sink; /* Write d[idx] - potential WAR */
    }
    
    /* Final store to prevent dead code elimination */
    volatile_sink = a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Helper to initialize arrays */
void init_arrays(int *a, int *b, int *c, int *d, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
        d[i] = i * 4;
    }
}

/* Checksum to ensure computations aren't optimized away */
int compute_checksum(int *a, int *b, int *c, int *d, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += a[i] + b[i] + c[i] + d[i];
    }
    return sum;
}

int main(int argc, char **argv) {
    /* Use command line argument for loop bound to prevent constant folding */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 10;
        if (n > 10000) n = 10000;
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
    
    /* Initialize arrays */
    init_arrays(a, b, c, d, n);
    
    /* Execute the target loop */
    process_loop(a, b, c, d, n);
    
    /* Compute and print checksum to ensure side effects */
    int checksum = compute_checksum(a, b, c, d, n);
    printf("Checksum: %d (n=%d)\n", checksum, n);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    
    return 0;
}
