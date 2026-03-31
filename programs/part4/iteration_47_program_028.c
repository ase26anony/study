#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent optimizations that might eliminate dependencies */
#define NO_OPT __attribute__((noinline, noipa))

/* Test 1: Simple RAW (Read-After-Write) dependency */
static void NO_OPT test_raw_dep(int *a, int *b, int n) {
    /* Loop-carried RAW dependency with distance 1 */
    for (int i = 1; i < n; i++) {
        a[i] = a[i-1] + b[i];  /* RAW: read a[i-1], write a[i] */
    }
}

/* Test 2: Mixed dependencies (RAW, WAR, WAW) */
static void NO_OPT test_mixed_deps(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        /* RAW dependency: read arr[i] */
        int temp = arr[i];
        
        /* WAR dependency: write to arr[i] after reading it */
        arr[i] = temp * 2 + i;
        
        /* WAW dependency: conditional second write to same location */
        if (i % 3 == 0) {
            arr[i] = arr[i] + 100;  /* WAW: second write to arr[i] */
        }
        
        /* Use temp to prevent dead code elimination */
        arr[i] += (temp & 1);
    }
}

/* Test 3: Loop-carried dependency with distance > 1 */
static void NO_OPT test_distance_2(int *a, int n) {
    /* Distance 2 RAW dependency */
    for (int i = 2; i < n; i++) {
        a[i] = a[i-2] * 3 - i;  /* RAW with distance 2 */
    }
}

/* Test 4: Complex dependencies with multiple arrays */
static void NO_OPT test_complex_deps(int *a, int *b, int *c, int n) {
    for (int i = 1; i < n; i++) {
        /* Multiple interleaved dependencies */
        int t1 = a[i-1];          /* RAW from previous iteration */
        b[i] = t1 + c[i];         /* Write b[i] */
        int t2 = b[i];            /* RAW from current iteration (b[i]) */
        a[i] = t2 * 2;            /* Write a[i] */
        c[i] = a[i] + b[i-1];     /* WAR on a[i], RAW on b[i-1] */
    }
}

/* Test 5: Volatile to enforce memory dependencies */
static void NO_OPT test_volatile_deps(volatile int *v, int *a, int n) {
    for (int i = 1; i < n; i++) {
        /* Volatile read creates hard memory barrier */
        int val = v[i];
        
        /* RAW dependency through volatile */
        a[i] = a[i-1] + val;
        
        /* Write back to volatile */
        v[i] = a[i] % 256;
    }
}

/* Initialize arrays with non-constant values */
static void init_arrays(int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (i * 7) % 19;
        b[i] = (i * 13) % 23;
        c[i] = (i * 17) % 29;
    }
}

/* Simple checksum to prevent dead code elimination */
static int checksum(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum = (sum * 31 + arr[i]) % 1000000007;
    }
    return sum;
}

int main(int argc, char **argv) {
    /* Use argc to make size non-constant for the optimizer */
    int size = 100 + (argc > 1 ? atoi(argv[1]) % 50 : 0);
    if (size < 10) size = 100;
    
    /* Dynamic allocation prevents constant propagation */
    int *a = (int*)malloc(size * sizeof(int));
    int *b = (int*)malloc(size * sizeof(int));
    int *c = (int*)malloc(size * sizeof(int));
    volatile int *v = (int*)malloc(size * sizeof(int));
    
    if (!a || !b || !c || !v) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with varying data */
    init_arrays(a, b, c, size);
    for (int i = 0; i < size; i++) {
        v[i] = (i * 11) % 31;
    }
    
    /* Call test functions to create various DDG edges */
    test_raw_dep(a, b, size);
    
    /* Reset arrays for next test */
    init_arrays(a, b, c, size);
    test_mixed_deps(a, size);
    
    init_arrays(a, b, c, size);
    test_distance_2(a, size);
    
    init_arrays(a, b, c, size);
    test_complex_deps(a, b, c, size);
    
    init_arrays(a, b, c, size);
    test_volatile_deps(v, a, size);
    
    /* Compute checksums to ensure all computations are used */
    int sum1 = checksum(a, size);
    int sum2 = checksum(b, size);
    int sum3 = checksum(c, size);
    int sum4 = 0;
    for (int i = 0; i < size; i++) {
        sum4 = (sum4 * 37 + v[i]) % 1000000007;
    }
    
    printf("Checksums: %d %d %d %d\n", sum1, sum2, sum3, sum4);
    
    free(a);
    free(b);
    free(c);
    free((void*)v);
    
    return 0;
}
