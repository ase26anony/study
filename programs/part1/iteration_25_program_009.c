/* test_ddg.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent optimization */
static int __attribute__((noinline)) get_value(int idx) {
    return idx * 3 + 7;
}

/* Volatile read to prevent dead code elimination */
static volatile int volatile_source = 42;

/* Target function with complex loop carrying all dependency types */
void __attribute__((noinline)) 
process_loop(int *arr, int *brr, int *crr, int n) {
    int i;
    
    /* Initialize some values */
    int temp = volatile_source;
    int acc = 0;
    
    /* 
     * Loop designed to create multiple DDG edge types:
     * 1. Flow dependencies (RAW)
     * 2. Anti dependencies (WAR)  
     * 3. Output dependencies (WAW)
     * 4. Loop-carried dependencies (distance > 0)
     */
    for (i = 1; i < n; i++) {
        /* Statement 1: Flow dependency source (write to arr[i]) */
        arr[i] = brr[i] + temp;          /* RAW source for statement 2 */
        
        /* Statement 2: Flow dependency consumer (read arr[i]) */
        int val1 = arr[i] * 2;           /* RAW from statement 1 */
        
        /* Statement 3: Anti dependency (WAR) */
        int val2 = crr[i];               /* Read before write in statement 4 */
        crr[i] = val1 + val2;            /* WAR: statement 4 writes crr[i] */
        
        /* Statement 4: Output dependency (WAW) and loop-carried flow */
        crr[i] = crr[i-1] + get_value(i); /* WAW with statement 3, 
                                           * Loop-carried flow from crr[i-1] */
        
        /* Statement 5: Multiple dependencies */
        brr[i] = arr[i-1] + crr[i];      /* Loop-carried flow from arr[i-1],
                                           * Flow from statement 4's crr[i] */
        
        /* Statement 6: Create anti dependency with statement 7 */
        int temp2 = arr[i];              /* Read arr[i] */
        arr[i] = brr[i] * 3;             /* WAR: overwrites arr[i] read above */
        
        /* Statement 7: Complex dependency web */
        acc += arr[i] + temp2;           /* Flow from statement 6,
                                           * Anti from statement 6's temp2 */
    }
    
    /* Use result to prevent dead code elimination */
    arr[0] = acc;
}

/* Another loop with different patterns to increase coverage */
void __attribute__((noinline))
process_loop2(int *a, int *b, int *c, int n) {
    int i;
    
    /* Loop with cross-iteration dependencies */
    for (i = 2; i < n; i++) {
        /* Chain of dependencies across iterations */
        a[i] = a[i-1] + b[i-2];          /* Distance 1 and 2 dependencies */
        b[i] = a[i] * c[i];              /* Flow within iteration */
        c[i] = b[i-1] + volatile_source; /* Distance 1 dependency */
        
        /* Create output dependency */
        int t = b[i];
        b[i] = t + i;                    /* WAW with previous b[i] write */
        
        /* Memory dependency with variant index */
        a[(i * 7) % n] = b[i] + c[i-1];  /* Complex memory dependency */
    }
}

int main(int argc, char **argv) {
    int n = 1000;
    
    /* Use command line argument to prevent constant propagation */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;  /* Ensure minimum size */
    }
    
    /* Allocate arrays with volatile size to prevent optimization */
    volatile int size_volatile = n;
    int size = size_volatile;
    
    int *arr = (int*)malloc(size * sizeof(int));
    int *brr = (int*)malloc(size * sizeof(int)); 
    int *crr = (int*)malloc(size * sizeof(int));
    
    if (!arr || !brr || !crr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < size; i++) {
        arr[i] = i;
        brr[i] = i * 2;
        crr[i] = i * 3;
    }
    
    /* Call the target functions multiple times */
    process_loop(arr, brr, crr, size);
    process_loop2(arr, brr, crr, size);
    
    /* Compute checksum to ensure computations aren't optimized away */
    int checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += arr[i] + brr[i] + crr[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(arr);
    free(brr);
    free(crr);
    
    return 0;
}
