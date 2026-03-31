/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */

#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(int seed) {
    return seed * 1103515245 + 12345;
}

/* Volatile read to prevent dead code elimination */
static volatile int volatile_source = 42;

/* Target function with complex loop carrying all dependency types */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Initialize with volatile to prevent constant folding */
    int init = volatile_source;
    
    /* Loop with multiple dependency patterns */
    for (i = 1; i < n; i++) {
        /* 1. FLOW DEPENDENCY (RAW) within iteration */
        int temp = a[i-1] + get_value(i);  /* Read a[i-1] */
        b[i] = temp * 2;                   /* Write b[i] */
        c[i] = b[i] + init;                /* Read b[i] just written -> Flow dep */
        
        /* 2. ANTI DEPENDENCY (WAR) */
        int read_before = d[i];            /* Read d[i] */
        a[i] = read_before + c[i];         /* Write a[i] */
        d[i] = a[i] * 3;                   /* Write d[i] after read -> Anti dep */
        
        /* 3. OUTPUT DEPENDENCY (WAW) */
        int tmp = get_value(i) % 100;
        b[i] = tmp + 5;                    /* Write b[i] again -> Output dep */
        
        /* 4. LOOP-CARRIED FLOW DEPENDENCY (distance = 1) */
        c[i] = c[i-1] + b[i];              /* Read c[i-1] from prev iteration */
        
        /* 5. Additional memory dependencies with variant indices */
        int idx = i % 10;
        a[idx] = b[idx] + c[idx];          /* May create cross-iteration deps */
    }
}

/* Another function with different patterns */
void __attribute__((noinline))
process_loop2(int *restrict arr1, int *restrict arr2, int n) {
    int i;
    
    /* Loop with write-after-write and write-after-read patterns */
    for (i = 2; i < n - 1; i++) {
        /* Chain of dependencies */
        int x = arr1[i-2];
        int y = arr2[i-1];
        
        /* Multiple writes to same location */
        arr1[i] = x + y;                   /* Write arr1[i] */
        arr1[i] = arr1[i] * 2;             /* Write arr1[i] again -> Output dep */
        
        /* Read after write in next statement */
        int z = arr1[i];                    /* Read arr1[i] */
        arr2[i+1] = z + i;                  /* Write arr2[i+1] */
        
        /* Anti-dependency pattern */
        int old = arr2[i];                  /* Read arr2[i] */
        arr1[i-1] = old * 3;                /* Write arr1[i-1] */
        arr2[i] = arr1[i-1] + 1;           /* Write arr2[i] after read -> Anti dep */
    }
}

int main(int argc, char **argv) {
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
    int *d = (int*)malloc(n * sizeof(int));
    int *arr1 = (int*)malloc(n * sizeof(int));
    int *arr2 = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d || !arr1 || !arr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        a[i] = i * 3 + 1;
        b[i] = i * 5 + 2;
        c[i] = i * 7 + 3;
        d[i] = i * 11 + 5;
        arr1[i] = i * 13 + 7;
        arr2[i] = i * 17 + 11;
    }
    
    /* Call both loop processing functions */
    process_loop(a, b, c, d, n);
    process_loop2(arr1, arr2, n);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += a[i] + b[i] + c[i] + d[i] + arr1[i] + arr2[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(arr1); free(arr2);
    
    return 0;
}
