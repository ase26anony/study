/* test_ddg.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(int x) {
    return x ^ 0x55;
}

/* Volatile read to prevent optimization */
static volatile int volatile_source;

/* Target function with carefully constructed data dependencies */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Complex loop with multiple dependency types */
    for (i = 1; i < n - 1; i++) {
        /* FLOW (RAW) dependency: c[i] depends on a[i] */
        int temp1 = a[i] + get_value(i);
        
        /* ANTI (WAR) dependency: a[i] read before write in next statement */
        int temp2 = b[i] * temp1;
        
        /* OUTPUT (WAW) dependency: a[i] written twice */
        a[i] = temp2 + volatile_source;
        
        /* Cross-iteration FLOW dependency (distance = 1): 
           b[i+1] depends on b[i] from previous iteration */
        b[i+1] = b[i] + c[i-1];
        
        /* Another FLOW dependency with different data type */
        c[i] = temp1 * 2;
        
        /* MEMORY dependency with variant index */
        d[i + a[i] % 4] = d[i-1] + 1;
        
        /* OUTPUT dependency on c */
        c[i] = temp2 / 3;
        
        /* Complex expression to prevent simplification */
        a[i] = (a[i] * 3 + b[i]) ^ c[i];
    }
}

/* Secondary loop with different patterns */
void __attribute__((noinline))
process_loop2(float *restrict fa, float *restrict fb, int n) {
    int i;
    
    for (i = 2; i < n; i++) {
        /* Flow dependency chain */
        float t1 = fa[i] * 1.5f;
        float t2 = t1 + fb[i];
        
        /* Anti-dependency: read fa[i] before writing */
        float old = fa[i];
        
        /* Write to fa creates output dependency if unrolled */
        fa[i] = t2 * old;
        
        /* Cross-iteration flow with distance 2 */
        fb[i] = fb[i-2] + t1;
        
        /* Another write to fa for output dependency */
        fa[i] = fa[i] + 0.5f;
    }
}

int main(int argc, char *argv[]) {
    int n = 1000;
    
    /* Use command line or volatile to prevent constant folding */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;
    } else {
        volatile int vn = 1000;
        n = vn;
    }
    
    /* Allocate arrays with alignment hint */
    int *a = __builtin_assume_aligned(malloc(n * sizeof(int)), 16);
    int *b = __builtin_assume_aligned(malloc(n * sizeof(int)), 16);
    int *c = __builtin_assume_aligned(malloc(n * sizeof(int)), 16);
    int *d = __builtin_assume_aligned(malloc(n * sizeof(int)), 16);
    
    float *fa = __builtin_assume_aligned(malloc(n * sizeof(float)), 16);
    float *fb = __builtin_assume_aligned(malloc(n * sizeof(float)), 16);
    
    if (!a || !b || !c || !d || !fa || !fb) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        a[i] = i * 3 + 1;
        b[i] = i * 5 + 2;
        c[i] = i * 7 + 3;
        d[i] = i * 11 + 5;
        fa[i] = i * 1.3f;
        fb[i] = i * 2.7f;
    }
    
    /* Set volatile source to prevent constant propagation */
    volatile_source = argc;
    
    /* Call the loops multiple times to ensure optimization */
    for (int iter = 0; iter < 10; iter++) {
        process_loop(a, b, c, d, n);
        process_loop2(fa, fb, n);
        
        /* Modify inputs slightly each iteration */
        a[0] += iter;
        fa[0] += iter * 0.5f;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int sum_int = 0;
    float sum_float = 0.0f;
    
    for (int i = 0; i < n; i++) {
        sum_int += a[i] + b[i] + c[i] + d[i];
        sum_float += fa[i] + fb[i];
    }
    
    /* Use the results */
    printf("Checksums: int=%d float=%.2f\n", sum_int, sum_float);
    
    /* Free memory */
    free(a); free(b); free(c); free(d);
    free(fa); free(fb);
    
    return 0;
}
