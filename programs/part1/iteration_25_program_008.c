/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */

#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(int x) {
    return x ^ 0x55AA;
}

/* Volatile read to prevent elimination */
static volatile int volatile_source = 42;

/* Target function with carefully constructed data dependencies */
void __attribute__((noinline)) 
process_loop(int *arr, int *brr, int *crr, int n) {
    int i;
    
    /* Loop with multiple dependency types */
    for (i = 1; i < n; i++) {
        /* FLOW dependency (RAW): arr[i] depends on brr[i-1] from previous iteration */
        int temp = brr[i-1] + get_value(i);
        
        /* ANTI dependency (WAR): crr[i] read before arr[i] write in same iteration */
        int anti_temp = crr[i] * 2;
        
        /* OUTPUT dependency (WAW): Two writes to arr[i] */
        arr[i] = temp + anti_temp;
        
        /* This creates WAW with previous arr[i] write */
        arr[i] = arr[i] + get_value(anti_temp);
        
        /* FLOW dependency within iteration: brr[i] depends on arr[i] */
        brr[i] = arr[i] / 3;
        
        /* ANTI dependency: Read arr[i] before writing crr[i] */
        int anti_temp2 = arr[i] + volatile_source;
        
        /* Create loop-carried FLOW dependency with distance 1 */
        crr[i] = crr[i-1] + anti_temp2;
        
        /* Additional FLOW dependency chain */
        arr[i] = arr[i] + brr[i-1];
    }
}

/* Another function with different patterns */
void __attribute__((noinline))
process_loop2(int *a, int *b, int *c, int n) {
    int i;
    
    /* Loop with cross-iteration dependencies */
    for (i = 2; i < n - 1; i++) {
        /* Complex dependency web */
        int t1 = a[i-2] + b[i-1];  /* Distance 2 flow */
        int t2 = c[i] * t1;        /* Within iteration flow */
        
        /* WAR: Read b[i] before overwriting it */
        int t3 = b[i] ^ 0xFF;
        b[i] = t2 + t3;
        
        /* WAW: Multiple writes to c[i] */
        c[i] = t1 * 2;
        c[i] = c[i] + get_value(t3);
        
        /* Loop-carried anti dependency */
        a[i] = b[i+1] - a[i-1];    /* Distance 1 anti */
    }
}

int main(int argc, char **argv) {
    int n = 1000;
    
    /* Use command line or volatile to prevent constant folding */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;
    } else {
        /* Volatile to prevent optimization */
        volatile int vn = 1000;
        n = vn;
    }
    
    /* Allocate arrays with volatile to prevent alignment assumptions */
    volatile int *volatile_arr = (volatile int*)malloc(n * sizeof(int));
    volatile int *volatile_brr = (volatile int*)malloc(n * sizeof(int));
    volatile int *volatile_crr = (volatile int*)malloc(n * sizeof(int));
    
    int *arr = (int*)volatile_arr;
    int *brr = (int*)volatile_brr;
    int *crr = (int*)volatile_crr;
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        arr[i] = i;
        brr[i] = i * 2;
        crr[i] = i * 3;
    }
    
    /* Call the loop processing functions */
    process_loop(arr, brr, crr, n);
    process_loop2(arr, brr, crr, n);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += arr[i] + brr[i] * 2 + crr[i] * 3;
    }
    
    /* Use checksum in output */
    printf("Result checksum: %lld\n", checksum);
    
    free((void*)volatile_arr);
    free((void*)volatile_brr);
    free((void*)volatile_crr);
    
    return 0;
}
