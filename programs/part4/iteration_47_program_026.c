#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent optimizations that might eliminate dependencies */
#define NOOPT __attribute__((noinline, noipa))

/* Test 1: Simple loop-carried RAW dependency */
NOOPT static void test_raw_dep(int *a, int *b, int n) {
    for (int i = 1; i < n; i++) {
        a[i] = a[i - 1] + b[i];
    }
}

/* Test 2: Mixed dependencies (RAW, WAR, WAW) in one loop */
NOOPT static void test_mixed_deps(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        /* RAW: Read arr[i] */
        int temp = arr[i];
        
        /* WAR: Write to arr[i] after reading it */
        arr[i] = temp * 2 + i;
        
        /* WAW: Conditional second write to same location */
        if (i % 3 == 0) {
            arr[i] = arr[i] + 100;  /* Second write to arr[i] */
        }
        
        /* Use temp to prevent dead code elimination */
        arr[i] += temp % 7;
    }
}

/* Test 3: Loop-carried dependency with distance > 1 */
NOOPT static void test_distance_2(int *a, int n) {
    for (int i = 2; i < n; i++) {
        a[i] = a[i - 2] * 3 + i;
    }
}

/* Test 4: Complex dependencies with multiple arrays */
NOOPT static void test_complex_deps(int *a, int *b, int *c, int n) {
    for (int i = 1; i < n; i++) {
        /* Multiple RAW dependencies */
        int t1 = a[i - 1];
        int t2 = b[i];
        
        /* WAR: Read then write to c[i] */
        int old_c = c[i];
        c[i] = t1 + t2;
        
        /* WAW: Two writes to a[i] */
        a[i] = old_c * 2;
        if (i % 4 == 0) {
            a[i] = a[i] + 50;  /* Second write */
        }
        
        /* Loop-carried with distance 1 */
        b[i] = b[i - 1] + 1;
    }
}

/* Test 5: Volatile to enforce memory dependencies */
NOOPT static void test_volatile_deps(volatile int *a, int *b, int n) {
    for (int i = 1; i < n; i++) {
        /* Volatile read creates strong dependency */
        int v = a[i - 1];
        
        /* Multiple operations to create various edges */
        b[i] = v + i;
        a[i] = b[i] * 2;
        
        /* Conditional WAW */
        if (b[i] > 100) {
            a[i] = a[i] - 50;
        }
    }
}

int main(int argc, char **argv) {
    /* Use argc to make size non-constant */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;
    
    /* Dynamic allocation prevents constant propagation */
    int *a = (int *)malloc(size * sizeof(int));
    int *b = (int *)malloc(size * sizeof(int));
    int *c = (int *)malloc(size * sizeof(int));
    volatile int *v = (volatile int *)malloc(size * sizeof(int));
    
    if (!a || !b || !c || !v) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        v[i] = rand() % 100;
    }
    
    /* Call test functions to create DDG edges */
    test_raw_dep(a, b, size);
    test_mixed_deps(c, size);
    test_distance_2(a, size);
    test_complex_deps(a, b, c, size);
    test_volatile_deps(v, a, size);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += a[i] + b[i] + c[i] + v[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free((void *)v);
    
    return 0;
}
