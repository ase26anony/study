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
process_loop(int *arr, int *brr, int *crr, int n) {
    int i;
    
    /* Initialize with volatile to prevent dead code elimination */
    int init_val = volatile_source;
    if (init_val == 0) init_val = 1;
    
    /* Loop with multiple dependency types */
    for (i = 1; i < n; i++) {
        /* FLOW dependency (RAW): arr[i] depends on brr[i] */
        int temp = brr[i] + init_val;
        
        /* OUTPUT dependency (WAW): Two writes to arr[i] */
        arr[i] = temp * 2;           /* First write */
        
        /* ANTI dependency (WAR): crr[i] read before arr[i] write */
        int read_crr = crr[i] + 7;
        
        /* Second write to arr[i] - creates OUTPUT dependency with line above */
        arr[i] = arr[i] + read_crr;  /* Second write - WAW with previous */
        
        /* FLOW dependency with distance > 0 (loop-carried) */
        brr[i] = brr[i-1] + arr[i];  /* RAW from previous iteration */
        
        /* ANTI dependency (WAR): Using arr[i] then modifying it */
        int copy = arr[i];
        arr[i] = get_value(i);       /* WAR dependency */
        
        /* Another FLOW dependency */
        crr[i] = copy + arr[i-1];    /* RAW from arr[i-1] */
        
        /* Complex memory dependency with variant index */
        int idx = i % 8;
        arr[idx] = brr[idx] + crr[i % 4];
    }
}

/* Another loop with cross-iteration dependencies */
void __attribute__((noinline))
process_loop2(int *a, int *b, int n) {
    int i;
    
    /* Loop-carried flow dependency (distance = 1) */
    for (i = 1; i < n; i++) {
        a[i] = a[i-1] + b[i];        /* Strong loop-carried dependency */
    }
    
    /* Output dependencies in separate loop */
    for (i = 0; i < n; i++) {
        int t = b[i];
        b[i] = t + 1;                /* Anti dependency potential */
        b[i] = b[i] * 2;             /* Output dependency */
    }
}

int main(int argc, char **argv) {
    int n = 1000;
    
    /* Use command line argument to prevent constant propagation */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = 100;
        if (n > 10000) n = 10000;    /* Reasonable limit */
    }
    
    /* Allocate arrays with volatile to prevent optimization */
    volatile int v_size = n;
    int size = v_size;
    
    int *arr = (int*)malloc(size * sizeof(int));
    int *brr = (int*)malloc(size * sizeof(int));
    int *crr = (int*)malloc(size * sizeof(int));
    
    if (!arr || !brr || !crr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-zero values */
    for (int i = 0; i < size; i++) {
        arr[i] = i + 1;
        brr[i] = i * 2;
        crr[i] = i * 3;
    }
    
    /* Call the loop processing functions */
    process_loop(arr, brr, crr, size);
    process_loop2(arr, brr, size);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += arr[i] + brr[i] + crr[i];
    }
    
    /* Use checksum in output */
    printf("Result checksum: %d\n", checksum);
    
    free(arr);
    free(brr);
    free(crr);
    
    return 0;
}
