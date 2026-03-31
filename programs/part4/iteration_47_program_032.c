#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and interprocedural analysis */
static void __attribute__((noinline,noipa)) 
test_raw_dep(int *a, int *b, int n) {
    /* Loop-carried RAW dependency with distance 1 */
    for (int i = 1; i < n; i++) {
        a[i] = a[i-1] + b[i];
    }
}

static void __attribute__((noinline,noipa))
test_mixed_deps(int *arr, int n) {
    /* Mixed dependencies within same iteration */
    for (int i = 0; i < n; i++) {
        /* RAW: Read arr[i] */
        int temp = arr[i];
        
        /* WAR: Write to arr[i] after reading it */
        arr[i] = temp * 2 + i;
        
        /* WAW: Conditional second write to same location */
        if (i % 3 == 0) {
            arr[i] = arr[i] + 100;  /* Second write to arr[i] */
        }
    }
}

static void __attribute__((noinline,noipa))
test_distance_2(int *a, int n) {
    /* Loop-carried RAW dependency with distance 2 */
    for (int i = 2; i < n; i++) {
        a[i] = a[i-2] * 3;
    }
}

/* Additional test with volatile to prevent optimization */
static void __attribute__((noinline,noipa))
test_volatile_deps(volatile int *v, int *a, int n) {
    /* Volatile creates memory barrier preventing dependency elimination */
    for (int i = 1; i < n; i++) {
        a[i] = *v + a[i-1];
        *v = a[i];  /* WAR through volatile */
    }
}

/* Test with pointer aliasing to confuse the optimizer */
static void __attribute__((noinline,noipa))
test_aliasing_deps(int *p, int *q, int n) {
    /* q may alias p, creating additional dependencies */
    for (int i = 1; i < n; i++) {
        p[i] = p[i-1] + q[i];  /* RAW with possible aliasing */
        q[i-1] = p[i] + i;     /* WAR with possible aliasing */
    }
}

int main(int argc, char **argv) {
    /* Use argc to make size non-constant */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;  /* Ensure reasonable size */
    
    /* Dynamic allocation prevents constant propagation */
    int *array1 = (int*)malloc(size * sizeof(int));
    int *array2 = (int*)malloc(size * sizeof(int));
    int *array3 = (int*)malloc(size * sizeof(int));
    int *array4 = (int*)malloc(size * sizeof(int));
    int *array5 = (int*)malloc(size * sizeof(int));
    
    if (!array1 || !array2 || !array3 || !array4 || !array5) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant values */
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        array1[i] = rand() % 100;
        array2[i] = rand() % 100;
        array3[i] = rand() % 100;
        array4[i] = rand() % 100;
        array5[i] = rand() % 100;
    }
    
    /* Call test functions to create various DDG edges */
    test_raw_dep(array1, array2, size);
    test_mixed_deps(array3, size);
    test_distance_2(array4, size);
    
    /* Create volatile variable for memory barrier */
    volatile int volatile_var = 42;
    test_volatile_deps(&volatile_var, array5, size);
    
    /* Test with potential pointer aliasing */
    test_aliasing_deps(array1, array2, size/2);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += array1[i] + array2[i] + array3[i] + array4[i] + array5[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    free(array5);
    
    return 0;
}
