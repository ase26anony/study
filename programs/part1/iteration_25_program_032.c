/* test_ddg.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(void) {
    static int counter = 0;
    return ++counter;
}

/* Volatile read to prevent optimization */
static int volatile_read(void) {
    volatile int v = 42;
    return v;
}

/* Target function with complex loop carrying multiple dependency types */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Initialize with opaque values */
    int init_a = get_value();
    int init_b = volatile_read();
    
    a[0] = init_a;
    b[0] = init_b;
    
    /* 
     * Complex loop with multiple dependency patterns:
     * 1. Flow dependencies (RAW)
     * 2. Anti dependencies (WAR)  
     * 3. Output dependencies (WAW)
     * 4. Loop-carried dependencies (distance > 0)
     */
    for (i = 1; i < n; i++) {
        /* FLOW dependency (RAW): c[i] depends on a[i-1] from previous iteration */
        c[i] = a[i-1] + b[i];           /* Statement A */
        
        /* ANTI dependency (WAR): a[i] is read then written in same iteration */
        int temp = a[i];                /* Statement B - read a[i] */
        a[i] = c[i] * 2;                /* Statement C - write a[i] (WAR with B) */
        
        /* OUTPUT dependency (WAW): d[i] written twice */
        d[i] = temp + i;                /* Statement D - first write to d[i] */
        
        /* Another FLOW dependency within same iteration */
        b[i] = a[i] + d[i];             /* Statement E - depends on C and D */
        
        /* OUTPUT dependency (WAW): second write to d[i] */
        d[i] = b[i] - c[i];             /* Statement F - WAW with D */
        
        /* Loop-carried FLOW dependency with distance 2 */
        if (i >= 2) {
            a[i] += c[i-2];             /* Statement G - depends on iteration i-2 */
        }
        
        /* ANTI dependency across iterations */
        if (i < n-1) {
            /* b[i+1] will be read in next iteration before being written here */
            b[i+1] = a[i] + d[i-1];     /* Statement H - creates WAR for next iteration */
        }
    }
    
    /* Final computation to prevent dead code elimination */
    a[n-1] = c[n-1] + d[n-1];
}

/* Helper function to compute checksum */
int compute_checksum(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum ^= arr[i];  /* Use XOR to avoid overflow issues */
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Use command line argument for loop bound to prevent constant folding */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;  /* Ensure minimum size */
        if (n > 10000) n = 10000; /* Limit for safety */
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
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < n; i++) {
        a[i] = i * 3;
        b[i] = i * 5;
        c[i] = i * 7;
        d[i] = i * 11;
    }
    
    /* Execute the target loop */
    process_loop(a, b, c, d, n);
    
    /* Compute and print checksums to ensure computations aren't optimized away */
    int checksum_a = compute_checksum(a, n);
    int checksum_b = compute_checksum(b, n);
    int checksum_c = compute_checksum(c, n);
    int checksum_d = compute_checksum(d, n);
    
    printf("Checksums: a=%d, b=%d, c=%d, d=%d\n", 
           checksum_a, checksum_b, checksum_c, checksum_d);
    
    /* Use results to affect return value */
    int result = checksum_a ^ checksum_b ^ checksum_c ^ checksum_d;
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return result & 0xFF;  /* Return non-zero to indicate execution */
}
