/* ddg_test.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Complex dependency patterns to create various DDG edges */
void process_data(int *a, int *b, int *c, int n) {
    int i, j;
    int reg1, reg2, reg3;
    
    /* Loop with multiple dependency types */
    for (i = 1; i < n; i++) {
        /* FLOW (RAW) dependency - memory to register */
        reg1 = a[i-1] + b[i];      /* Read a[i-1] */
        
        /* ANTI (WAR) dependency */
        reg2 = c[i];               /* Read c[i] */
        c[i] = reg1 * 2;           /* Write c[i] - anti-dep with previous read */
        
        /* OUTPUT (WAW) dependency */
        a[i] = reg1 + reg2;        /* Write a[i] - first write */
        
        /* Control flow to create basic block boundaries */
        if (i % 3 == 0) {
            /* Another FLOW dependency inside conditional */
            reg3 = a[i] * 3;       /* Read a[i] just written */
            b[i] = reg3 + i;       /* Write b[i] */
            
            /* Another OUTPUT dependency */
            a[i] = reg3 / 2;       /* Second write to a[i] - output dep */
        } else if (i % 3 == 1) {
            /* Different path with anti dependency */
            int temp = b[i-1];     /* Read b[i-1] */
            b[i-1] = a[i] + 5;     /* Write b[i-1] - anti-dep */
            a[i] = temp * 2;       /* Modify a[i] */
        } else {
            /* Path with flow dependency across iterations */
            a[i] = a[i-1] + b[i-2] + 1;  /* Loop-carried dep distance=1,2 */
        }
        
        /* Additional scalar operations for register dependencies */
        reg1 = reg2 * reg1;
        reg2 = reg1 + i;
        reg3 = reg2 - reg1;
    }
}

/* Nested loops with different dependency patterns */
void nested_processing(int *arr1, int *arr2, int n, int m) {
    int i, j;
    
    for (i = 0; i < n; i++) {
        int acc = arr1[i];
        
        for (j = 1; j < m; j++) {
            /* Loop-carried flow dependency in inner loop */
            acc = acc + arr2[j] + i;
            
            /* Anti dependency in inner loop */
            int temp = arr2[j-1];
            arr2[j-1] = acc * j;
            acc = temp + acc;
            
            /* Output dependency */
            arr1[i] = acc;
            arr1[i] = arr1[i] * 2;  /* Second write - output dep */
        }
        
        /* Cross-iteration dependency in outer loop */
        if (i > 0) {
            arr2[i] = arr2[i-1] + arr1[i];
        }
    }
}

/* Function with pointer aliasing to create memory dependencies */
void pointer_aliasing_test(int *p, int *q, int n) {
    int i;
    
    /* Assume possible aliasing */
    for (i = 1; i < n; i++) {
        /* Complex memory dependencies */
        p[i] = q[i-1] + p[i-1];    /* Flow deps with distance 1 */
        q[i] = p[i] * 2;           /* Immediate flow dep */
        
        /* Create anti dependency */
        int tmp = p[i/2];
        p[i/2] = q[i] + 3;         /* Anti dep */
        q[i] = tmp - 1;
        
        /* Output dependency on different indices */
        if (i % 4 == 0) {
            p[i/4] = q[i] * 2;
            p[i/4] = p[i/4] + 1;   /* Output dep on p[i/4] */
        }
    }
}

int main() {
    int *array1, *array2, *array3;
    int result = 0;
    int i;
    
    /* Allocate and initialize arrays */
    array1 = (int*)malloc(N * sizeof(int));
    array2 = (int*)malloc(N * sizeof(int));
    array3 = (int*)malloc(N * sizeof(int));
    
    if (!array1 || !array2 || !array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (i = 0; i < N; i++) {
        array1[i] = i;
        array2[i] = N - i;
        array3[i] = (i * 3) % 7;
    }
    
    /* Execute functions with complex dependencies */
    process_data(array1, array2, array3, N);
    nested_processing(array1, array2, N/2, M);
    pointer_aliasing_test(array3, array1, N);
    
    /* Compute a result to prevent dead code elimination */
    for (i = 0; i < N; i++) {
        result += array1[i] + array2[i] * 2 - array3[i];
    }
    
    /* Use result to avoid optimization */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
