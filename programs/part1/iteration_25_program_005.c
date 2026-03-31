/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(int x) {
    return x ^ 0x55AA;
}

/* Volatile read to prevent dead code elimination */
static volatile int volatile_source = 42;

/* Target function with loop containing multiple dependency types */
void __attribute__((noinline)) 
process_loop(int *arr, int *brr, int *crr, int n) {
    int i;
    
    /* Initialize with volatile to prevent constant folding */
    int base = volatile_source;
    
    /* Loop with carefully crafted dependencies */
    for (i = 1; i < n; i++) {
        /* FLOW (RAW) dependency: crr[i] depends on arr[i] */
        int temp = arr[i] + base;
        
        /* OUTPUT (WAW) dependency: arr[i] written twice */
        arr[i] = temp * 2;
        
        /* ANTI (WAR) dependency: brr[i] read before being written */
        int anti_temp = brr[i] + crr[i-1];
        
        /* FLOW with distance=1: Loop-carried dependency */
        crr[i] = crr[i-1] + anti_temp;
        
        /* OUTPUT dependency on arr[i] from earlier */
        arr[i] = get_value(arr[i]);
        
        /* ANTI dependency: using arr[i] before overwriting */
        brr[i] = arr[i] + brr[i-1];
        
        /* Another FLOW dependency */
        arr[i] = brr[i] * 3;
    }
}

/* Alternative function with even more explicit dependencies */
void __attribute__((noinline))
complex_dependencies(int *a, int *b, int *c, int n) {
    int i;
    
    /* Initialize to prevent elimination */
    a[0] = volatile_source;
    b[0] = volatile_source + 1;
    c[0] = volatile_source + 2;
    
    /* Loop with cross-iteration dependencies */
    for (i = 1; i < n - 1; i++) {
        /* Chain of FLOW dependencies within iteration */
        int t1 = a[i] + b[i-1];      /* distance=1 flow */
        int t2 = t1 * c[i];          /* flow within iteration */
        
        /* OUTPUT dependency - multiple writes to same location */
        c[i] = t2 + i;
        
        /* ANTI dependency - read c[i] before overwriting */
        int t3 = b[i] + c[i];
        
        /* Another OUTPUT to c[i] */
        c[i] = t3 * 2;
        
        /* FLOW with distance=1 to next iteration */
        a[i+1] = a[i] + t3;          /* distance=1 flow */
        
        /* ANTI on b[i] */
        int t4 = b[i] * 3;
        b[i] = t4 + a[i-1];          /* distance=1 anti? Actually flow from a[i-1] */
        
        /* Another FLOW within iteration */
        c[i] = b[i] + c[i-1];        /* distance=1 flow */
    }
}

/* Simple checksum to prevent dead code elimination */
int compute_checksum(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum ^= arr[i];
    }
    return sum;
}

int main(int argc, char **argv) {
    int n = 1000;
    
    /* Use command line or volatile to prevent constant trip count */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;
    } else {
        /* Volatile to prevent compile-time determination */
        volatile int vn = 1000;
        n = vn;
    }
    
    /* Allocate arrays with volatile to prevent optimizations */
    int *arr1 = (int*)malloc(n * sizeof(int));
    int *arr2 = (int*)malloc(n * sizeof(int));
    int *arr3 = (int*)malloc(n * sizeof(int));
    
    if (!arr1 || !arr2 || !arr3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 5 + 2;
        arr3[i] = i * 7 + 3;
    }
    
    /* Call the function with dependencies */
    process_loop(arr1, arr2, arr3, n);
    
    /* Call second function for more edge cases */
    complex_dependencies(arr1, arr2, arr3, n);
    
    /* Compute checksum to ensure computations aren't eliminated */
    int sum1 = compute_checksum(arr1, n);
    int sum2 = compute_checksum(arr2, n);
    int sum3 = compute_checksum(arr3, n);
    
    printf("Checksums: %d, %d, %d\n", sum1, sum2, sum3);
    
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
