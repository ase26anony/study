/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(void) {
    static int counter = 0;
    return ++counter;
}

/* Volatile read to prevent dead code elimination */
static volatile int volatile_sink;

/* Target function with carefully constructed loop dependencies */
void __attribute__((noinline)) 
process_loop(int *restrict arr_a, int *restrict arr_b, 
             int *restrict arr_c, int n) {
    int i;
    
    /* Initialize with volatile to prevent constant folding */
    int init_val = get_value();
    arr_a[0] = init_val;
    arr_b[0] = init_val + 1;
    
    /* Main loop with multiple dependency types */
    for (i = 1; i < n; i++) {
        /* 1. FLOW DEPENDENCY (RAW) within iteration */
        int temp = arr_a[i-1] + arr_b[i];      /* Read arr_a[i-1] */
        arr_c[i] = temp * 2;                   /* Write arr_c[i] */
        
        /* 2. ANTI DEPENDENCY (WAR) within iteration */
        int read_before_write = arr_b[i];      /* Read arr_b[i] */
        arr_b[i] = get_value();                /* Write arr_b[i] - WAR with line above */
        
        /* 3. OUTPUT DEPENDENCY (WAW) within iteration */
        arr_a[i] = read_before_write + 1;      /* Write arr_a[i] - first write */
        arr_a[i] = arr_a[i] * 3;               /* Write arr_a[i] again - WAW with line above */
        
        /* 4. FLOW DEPENDENCY with distance > 0 (loop-carried) */
        arr_c[i] = arr_c[i-1] + arr_a[i];      /* Flow from iteration i-1 to i */
        
        /* 5. Complex memory dependency with variant index */
        int idx = i % 10;
        arr_b[idx] = arr_a[idx] + arr_c[i];    /* May create various dependencies */
        
        /* Prevent dead code elimination */
        volatile_sink = arr_a[i] + arr_b[i] + arr_c[i];
    }
    
    /* Cross-iteration anti dependency */
    for (i = 0; i < n-1; i++) {
        int val = arr_a[i];                    /* Read arr_a[i] */
        arr_a[i+1] = val + arr_b[i];           /* Write arr_a[i+1] - creates anti across iterations */
    }
}

/* Another function with different pattern to increase coverage */
void __attribute__((noinline))
process_loop2(float *restrict farr, int *restrict iarr, int n) {
    int i;
    
    /* Initialize */
    farr[0] = 1.0f;
    iarr[0] = 1;
    
    /* Loop with mixed float/int dependencies */
    for (i = 1; i < n; i++) {
        /* Flow dependency with conversion */
        float fval = farr[i-1];                /* Read float */
        iarr[i] = (int)fval + i;               /* Write int */
        
        /* Anti dependency with different types */
        int ival = iarr[i];                    /* Read int */
        farr[i] = (float)ival * 1.5f;          /* Write float - WAR */
        
        /* Output dependency on same array */
        farr[i] = farr[i] + farr[i-1];         /* WAW on farr[i] */
        
        /* Complex index calculation for memory deps */
        int idx = (i * 7) % n;
        if (idx > 0) {
            farr[idx] = farr[idx-1] + iarr[idx];
        }
    }
}

int main(int argc, char **argv) {
    int n = 1000;
    
    /* Use command line argument to prevent constant trip count */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = 100;
        if (n > 10000) n = 10000; /* Reasonable limit */
    }
    
    /* Allocate arrays with restrict to help alias analysis */
    int *arr_a = (int*)malloc(n * sizeof(int));
    int *arr_b = (int*)malloc(n * sizeof(int));
    int *arr_c = (int*)malloc(n * sizeof(int));
    float *farr = (float*)malloc(n * sizeof(float));
    int *iarr = (int*)malloc(n * sizeof(int));
    
    if (!arr_a || !arr_b || !arr_c || !farr || !iarr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < n; i++) {
        arr_a[i] = i;
        arr_b[i] = i * 2;
        arr_c[i] = i * 3;
        farr[i] = (float)i;
        iarr[i] = i;
    }
    
    /* Call functions with dependency-rich loops */
    process_loop(arr_a, arr_b, arr_c, n);
    process_loop2(farr, iarr, n);
    
    /* Compute checksum to prevent dead code elimination */
    int sum = 0;
    float fsum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += arr_a[i] + arr_b[i] + arr_c[i] + iarr[i];
        fsum += farr[i];
    }
    
    /* Use results to prevent optimization */
    printf("Checksum: int=%d, float=%.2f\n", sum, fsum);
    
    /* Cleanup */
    free(arr_a);
    free(arr_b);
    free(arr_c);
    free(farr);
    free(iarr);
    
    return 0;
}
