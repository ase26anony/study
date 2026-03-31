/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(int x) {
    return x ^ 0x55AA;
}

/* Volatile access to prevent dead code elimination */
static volatile int volatile_sink;

/* Target function with complex loop carrying multiple dependency types */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Initialize with volatile to prevent constant folding */
    int init_val = 0;
    volatile_sink = 1;
    init_val = volatile_sink;
    
    /* Complex loop with multiple dependency patterns */
    for (i = 1; i < n; i++) {
        /* FLOW (RAW) dependency within iteration */
        int temp = a[i-1] + get_value(i);      /* Statement 1: read a[i-1] */
        b[i] = temp * 2;                       /* Statement 2: use temp */
        
        /* ANTI (WAR) dependency */
        int read_before = b[i-1];              /* Statement 3: read b[i-1] */
        b[i-1] = c[i] + read_before;           /* Statement 4: write b[i-1] */
        
        /* OUTPUT (WAW) dependency on 'c' */
        c[i] = temp + read_before;             /* Statement 5: write c[i] */
        c[i] = c[i] * 3 - get_value(i);        /* Statement 6: write c[i] again */
        
        /* Loop-carried FLOW dependency (distance = 1) */
        d[i] = d[i-1] + a[i] + b[i];           /* Statement 7: read d[i-1] */
        
        /* Another loop-carried dependency with distance > 1 */
        if (i >= 3) {
            a[i] = a[i-2] + c[i-3];            /* Statement 8: distance = 2,3 */
        }
        
        /* Memory dependency with variant index */
        int idx = i % 5;
        c[idx] = b[i] + d[idx];                /* Statement 9: complex memory dep */
    }
}

/* Wrapper to ensure loop isn't optimized away */
int main(int argc, char **argv) {
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;  /* Ensure sufficient iterations */
    }
    
    /* Allocate with volatile size to prevent static analysis */
    volatile int size_volatile = n;
    int array_size = size_volatile;
    
    int *a = (int*)malloc(array_size * sizeof(int));
    int *b = (int*)malloc(array_size * sizeof(int));
    int *c = (int*)malloc(array_size * sizeof(int));
    int *d = (int*)malloc(array_size * sizeof(int));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with non-constant pattern */
    for (int i = 0; i < array_size; i++) {
        a[i] = i * 3 + 1;
        b[i] = i * 5 - 2;
        c[i] = i * 7 + 3;
        d[i] = i * 11 - 5;
    }
    
    /* Process the loop - this should trigger DDG construction */
    process_loop(a, b, c, d, array_size);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < array_size; i++) {
        checksum += a[i] + b[i] + c[i] + d[i];
        checksum = (checksum << 3) | (checksum >> 61);  /* Simple mixing */
    }
    
    /* Use the result */
    printf("Result checksum: %lld\n", checksum);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
