/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(int x) {
    return x * 3 + 7;
}

/* Volatile read to prevent dead code elimination */
static volatile int volatile_source = 42;

/* Target function with complex loop carrying multiple dependency types */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Initialize with volatile to prevent constant folding */
    int init_val = volatile_source;
    
    /* Complex loop with multiple dependency patterns */
    for (i = 1; i < n; i++) {
        /* 1. FLOW dependency (RAW): a[i] depends on b[i] */
        int temp = b[i] + init_val;
        
        /* 2. FLOW dependency with loop-carried (distance=1): 
           c[i] depends on c[i-1] */
        c[i] = c[i-1] + temp;
        
        /* 3. ANTI dependency (WAR): d[i] is read, then written */
        int old_d = d[i];           /* Read d[i] */
        a[i] = temp * 2;           /* Write a[i] - independent */
        d[i] = old_d + a[i];       /* Write d[i] after reading it */
        
        /* 4. OUTPUT dependency (WAW): a[i] written twice */
        a[i] = get_value(a[i]);    /* Second write to a[i] */
        
        /* 5. Another FLOW dependency with different distance */
        b[i] = a[i-1] + c[i];      /* Depends on a[i-1] (distance=1) */
    }
}

/* Alternative loop with even more complex dependencies */
void __attribute__((noinline))
process_loop2(int *restrict arr1, int *restrict arr2, int n) {
    int i;
    
    /* Initialize with function call to prevent optimization */
    int base = get_value(n);
    
    /* Loop with cross-iteration dependencies */
    for (i = 2; i < n - 1; i++) {
        /* Multiple interleaved dependencies */
        int t1 = arr1[i] + base;      /* Read arr1[i] */
        int t2 = arr2[i-1] * 2;       /* Read arr2[i-1] (distance=1) */
        
        arr1[i+1] = t1 + t2;          /* Write arr1[i+1] (distance=1) */
        
        /* Create anti-dependency chain */
        int old_val = arr2[i];        /* Read arr2[i] */
        arr2[i] = arr1[i] * 3;        /* Write arr2[i] after reading it */
        arr1[i] = old_val + arr2[i];  /* Write arr1[i] */
        
        /* Output dependency */
        arr2[i] = get_value(arr2[i]); /* Second write to arr2[i] */
        
        /* Another flow dependency with distance=2 */
        if (i > 3) {
            arr1[i] = arr1[i-2] + arr2[i]; /* distance=2 */
        }
    }
}

/* Main function with runtime-determined loop bounds */
int main(int argc, char **argv) {
    int n = 1000;
    
    /* Use command line or volatile to prevent constant trip count */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;  /* Ensure loop is large enough */
    } else {
        /* Use volatile to prevent compile-time determination */
        volatile int vn = 1000;
        n = vn;
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
        a[i] = i * 2;
        b[i] = i * 3;
        c[i] = i * 5;
        d[i] = i * 7;
    }
    
    /* Call the loop processing functions multiple times
       to give GCC more optimization opportunities */
    for (int iter = 0; iter < 3; iter++) {
        process_loop(a, b, c, d, n);
        
        /* Use results to prevent dead code elimination */
        volatile int checksum = 0;
        for (int i = 0; i < n; i++) {
            checksum += a[i] + b[i] + c[i] + d[i];
        }
        
        /* Process second loop */
        process_loop2(a, b, n);
        
        /* Another checksum */
        for (int i = 0; i < n; i++) {
            checksum += a[i] * b[i];
        }
    }
    
    /* Final computation that uses results */
    int final_sum = 0;
    for (int i = 0; i < n; i++) {
        final_sum += a[i] + b[i] + c[i] + d[i];
    }
    
    printf("Result: %d (n=%d)\n", final_sum, n);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
