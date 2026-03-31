/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(int x) {
    return x * 3 + 7;
}

/* Volatile read to prevent optimization */
static volatile int volatile_source = 42;

/* Target function with carefully constructed data dependencies */
void __attribute__((noinline)) process_loop(int *restrict a, int *restrict b, 
                                          int *restrict c, int n) {
    int i;
    
    /* Initialize with volatile to prevent constant folding */
    int init_val = volatile_source;
    
    /* Loop with multiple dependency types */
    for (i = 1; i < n; i++) {
        /* 1. FLOW DEPENDENCY (RAW) within iteration */
        int temp = a[i] + init_val;      /* Read a[i] */
        b[i] = temp * 2;                 /* Write b[i] - depends on temp */
        
        /* 2. ANTI DEPENDENCY (WAR) within iteration */
        int read_before_write = b[i-1];  /* Read b[i-1] */
        b[i-1] = get_value(i);           /* Write b[i-1] - anti-dep on line above */
        
        /* 3. OUTPUT DEPENDENCY (WAW) within iteration */
        c[i] = read_before_write + i;    /* Write c[i] - first write */
        c[i] = c[i] * 3;                 /* Write c[i] again - output dep on line above */
        
        /* 4. LOOP-CARRIED FLOW DEPENDENCY (distance = 1) */
        a[i] = a[i-1] + b[i];            /* Flow from iteration i-1 to i */
        
        /* 5. LOOP-CARRIED ANTI DEPENDENCY */
        int temp2 = c[i-1];              /* Read c[i-1] from previous iteration */
        c[i-1] = temp2 + a[i];           /* Write c[i-1] - anti-dep carried */
        
        /* 6. Additional flow to create more edges */
        int computed = get_value(b[i]);
        a[i] = computed + c[i];
    }
}

/* Another loop with different patterns to increase coverage */
void __attribute__((noinline)) process_loop2(int *restrict x, int *restrict y, 
                                           int *restrict z, int n) {
    int i;
    
    /* Loop with stride to create complex memory dependencies */
    for (i = 2; i < n - 2; i++) {
        /* Multiple interleaved dependencies */
        y[i] = x[i] + x[i-1];            /* Flow from x[i], x[i-1] */
        x[i+1] = y[i] * y[i-1];          /* Flow from y[i], y[i-1], loop-carried */
        z[i] = z[i-2] + x[i];            /* Flow with distance = 2 */
        
        /* Anti-dependency chain */
        int t1 = y[i];
        y[i] = z[i] + 1;
        int t2 = y[i];
        y[i] = t1 + t2;                  /* Multiple anti deps */
    }
}

int main(int argc, char *argv[]) {
    int n = 1000;
    
    /* Use command line argument to prevent constant trip count */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;
    }
    
    /* Allocate arrays with restrict to help alias analysis */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    int *x = (int*)malloc(n * sizeof(int));
    int *y = (int*)malloc(n * sizeof(int));
    int *z = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !x || !y || !z) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
        x[i] = i % 10;
        y[i] = (i + 1) % 10;
        z[i] = (i + 2) % 10;
    }
    
    /* Call loops multiple times to ensure execution */
    for (int iter = 0; iter < 3; iter++) {
        process_loop(a, b, c, n);
        process_loop2(x, y, z, n);
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += a[i] + b[i] + c[i] + x[i] + y[i] + z[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(x); free(y); free(z);
    
    return 0;
}
