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
    /* Contains RAW, WAR, and WAW dependencies */
    for (int i = 0; i < n; i++) {
        /* RAW: Read arr[i] */
        int t = arr[i];
        
        /* WAR: Write to arr[i] after reading it */
        arr[i] = t * 2 + i;
        
        /* WAW: Conditional second write to same location */
        if (i % 3 == 0) {
            arr[i] = arr[i] + 100;  /* Second write to arr[i] */
        }
        
        /* Use t to prevent dead code elimination */
        arr[i] += (t & 1);
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
test_volatile_dep(volatile int *v, int *a, int n) {
    /* Mix volatile and non-volatile accesses */
    for (int i = 1; i < n; i++) {
        int tmp = *v;      /* Volatile read - creates memory barrier */
        a[i] = a[i-1] + tmp;
        *v = a[i];         /* Volatile write - prevents many optimizations */
    }
}

/* Function with pointer aliasing to confuse the optimizer */
static void __attribute__((noinline,noipa))
test_aliasing_dep(int *p, int *q, int n) {
    /* q may alias p, creating additional dependencies */
    for (int i = 1; i < n; i++) {
        p[i] = p[i-1] + q[i];
        q[i-1] = p[i] * 2;  /* WAR: Write to q[i-1] after p read it earlier */
    }
}

int main(int argc, char **argv) {
    /* Use argc to make size non-constant */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;  /* Ensure minimum size */
    
    /* Dynamic allocation prevents constant propagation */
    int *a = (int*)malloc(size * sizeof(int));
    int *b = (int*)malloc(size * sizeof(int));
    int *c = (int*)malloc(size * sizeof(int));
    volatile int v = 42;
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant values */
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
    }
    
    /* Call test functions to create DDG edges */
    test_raw_dep(a, b, size);
    test_mixed_deps(c, size);
    test_distance_2(a, size);
    
    /* Test with volatile */
    test_volatile_dep(&v, b, size);
    
    /* Test with potential aliasing */
    test_aliasing_dep(a, b, size);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += a[i] + b[i] + c[i];
    }
    
    /* Use checksum in output */
    printf("Checksum: %d\n", checksum);
    
    free(a);
    free(b);
    free(c);
    
    return 0;
}
