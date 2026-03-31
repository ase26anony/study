/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 100

/* Global variables to create loop invariants */
int g_invariant1 = 7;
int g_invariant2 = 13;
int g_invariant3 = 19;

/* Mark the hot function as noinline to prevent inlining */
__attribute__((hot, noinline)) 
int hot_loop_function(int* arr1, int* arr2, int* arr3, int n) {
    int result = 0;
    
    /* Main loop with high register pressure and dependencies */
    for (int i = 1; i < n - 1; i++) {
        /* Declare many temporaries to create register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        int t10, t11, t12, t13, t14, t15, t16, t17, t18, t19;
        
        /* Start dependency chain with array accesses */
        t0 = arr1[i];
        t1 = t0 + arr2[i];           /* Dependency on t0 */
        t2 = t1 * g_invariant1;      /* Use invariant - may create move */
        t3 = t2 - arr3[i];           /* Dependency on t2 */
        t4 = t3 / g_invariant2;      /* Division with invariant - higher latency */
        t5 = t4 + arr1[i-1];         /* Dependency on t4, array with offset */
        
        /* Conditional to create multiple basic blocks */
        if (t5 & 1) {
            /* Path 1: More computations */
            t6 = t5 * arr2[i+1];     /* Dependency on t5, array with offset */
            t7 = t6 - g_invariant3;  /* Use invariant */
            t8 = t7 % 17;            /* Modulo - higher latency operation */
            t9 = t8 + arr3[i];       /* Dependency on t8 */
            t10 = t9 * 3;            /* Continue chain */
        } else {
            /* Path 2: Alternative computations */
            t6 = t5 + arr2[i];       /* Dependency on t5 */
            t7 = t6 * 2;             /* Dependency on t6 */
            t8 = t7 - arr3[i-1];     /* Dependency on t7, array with offset */
            t9 = t8 / 5;             /* Division */
            t10 = t9 + g_invariant1; /* Use invariant */
        }
        
        /* Rejoin and continue dependency chain */
        t11 = t10 * t5;              /* Cross-path dependency */
        t12 = t11 + arr1[i+1];       /* Array with offset */
        t13 = t12 - g_invariant2;    /* Use invariant */
        t14 = t13 % 23;              /* Another modulo - high latency */
        t15 = t14 * t11;             /* Dependency on t14 and t11 */
        t16 = t15 + arr2[i];         /* Memory access */
        t17 = t16 / 7;               /* Division */
        t18 = t17 - arr3[i+1];       /* Array with offset */
        t19 = t18 * g_invariant3;    /* Use invariant */
        
        /* Accumulate result */
        result += t19;
        
        /* Create loop-carried dependency */
        arr1[i] = (t19 + result) & 0xFF;
    }
    
    return result;
}

/* Helper function to generate deterministic pseudo-random values */
void initialize_arrays(int* a, int* b, int* c, int n) {
    int seed = 42;
    for (int i = 0; i < n; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        a[i] = (seed >> 16) & 0xFF;
        b[i] = (seed >> 8) & 0xFF;
        c[i] = seed & 0xFF;
    }
}

int main() {
    int* array1 = malloc(SIZE * sizeof(int));
    int* array2 = malloc(SIZE * sizeof(int));
    int* array3 = malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2 || !array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    int total_result = 0;
    
    /* Call hot function multiple times to ensure it's compiled as hot */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Re-initialize with different patterns each iteration */
        initialize_arrays(array1, array2, array3, SIZE);
        
        /* Modify invariants slightly each call */
        g_invariant1 = (g_invariant1 * 3 + 1) & 0xFF;
        g_invariant2 = (g_invariant2 * 5 + 2) & 0xFF;
        g_invariant3 = (g_invariant3 * 7 + 3) & 0xFF;
        
        total_result += hot_loop_function(array1, array2, array3, SIZE);
    }
    
    printf("Final result: %d\n", total_result);
    
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
