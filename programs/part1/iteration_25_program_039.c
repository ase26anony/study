/* test_ddg.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(void) {
    static int counter = 0;
    return ++counter;
}

/* Volatile read to prevent optimization */
static volatile int volatile_source = 42;

/* Target function with complex loop carrying multiple dependency types */
__attribute__((noinline))
void process_loop(int *arr, int *brr, int *crr, int n) {
    int i;
    
    /* Initialize with volatile to prevent dead code elimination */
    int init_val = volatile_source;
    
    /* Loop with carefully crafted dependencies */
    for (i = 1; i < n; i++) {
        /* 1. FLOW DEPENDENCY (RAW) with distance 1 */
        /* Read arr[i-1] from previous iteration, write to brr[i] */
        brr[i] = arr[i-1] + init_val;
        
        /* 2. ANTI DEPENDENCY (WAR) within same iteration */
        /* Read arr[i], then write to it later */
        int temp = arr[i] + get_value();
        
        /* 3. OUTPUT DEPENDENCY (WAW) within same iteration */
        /* Multiple writes to crr[i] */
        crr[i] = temp * 2;
        crr[i] = crr[i] + brr[i];  // WAW on crr[i]
        
        /* 4. FLOW DEPENDENCY with distance 0 (within iteration) */
        /* Use temp to create RAW within iteration */
        arr[i] = temp + crr[i];
        
        /* 5. ANTI DEPENDENCY with distance 1 */
        /* Read brr[i-1] (written in previous iteration), then modify it */
        int prev_val = brr[i-1];
        brr[i-1] = prev_val + 1;  // WAR on brr[i-1] across iterations
    }
}

/* Another function with different pattern to increase coverage */
__attribute__((noinline))
void process_loop2(int *a, int *b, int *c, int n) {
    int i;
    
    /* Loop with cross-iteration dependencies */
    for (i = 2; i < n - 1; i++) {
        /* Complex chain of dependencies */
        int t1 = a[i-2] + b[i-1];  // Flow from i-2 and i-1
        
        /* Output dependency chain */
        c[i] = t1 * 3;
        c[i] = c[i] + a[i];  // WAW on c[i]
        
        /* Anti dependency */
        int t2 = b[i];
        b[i] = t1 + t2;  // WAR on b[i]
        
        /* Flow to next iteration */
        a[i] = c[i-1] + t2;  // Flow from i-1
        
        /* Another output dependency */
        b[i+1] = a[i] * 2;
        b[i+1] = b[i+1] - 1;  // WAW on b[i+1]
    }
}

int main(int argc, char **argv) {
    int n = 1000;
    
    /* Use command line argument to prevent constant propagation */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;
    }
    
    /* Allocate arrays with volatile to prevent optimization */
    volatile int size = n;
    int *arr = (int*)malloc(size * sizeof(int));
    int *brr = (int*)malloc(size * sizeof(int));
    int *crr = (int*)malloc(size * sizeof(int));
    
    if (!arr || !brr || !crr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        arr[i] = i;
        brr[i] = i * 2;
        crr[i] = i * 3;
    }
    
    /* Call the loop processing functions multiple times */
    for (int iter = 0; iter < 10; iter++) {
        process_loop(arr, brr, crr, n);
        process_loop2(arr, brr, crr, n);
        
        /* Modify inputs slightly to prevent complete optimization */
        arr[0] += iter;
        brr[0] += iter;
    }
    
    /* Compute checksum to ensure computations aren't optimized away */
    int checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += arr[i] + brr[i] + crr[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(arr);
    free(brr);
    free(crr);
    
    return 0;
}
