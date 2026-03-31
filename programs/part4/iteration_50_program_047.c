/* test_ddg_coverage.c
 * Designed to trigger DDG edge creation in GCC's instruction scheduler
 * Compile with: gcc -O2 -fmodulo-sched -funroll-loops -c test_ddg_coverage.c
 * Or: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_global_array[1024];
int g_result = 0;

/* Function to create true data dependencies (RAW) */
int test_raw_dep(int *arr, int *brr, int n) {
    int sum = 0;
    /* Loop with flow dependencies across iterations */
    for (int i = 2; i < n; i++) {
        arr[i] = arr[i-1] + brr[i];      /* RAW: arr[i-1] read, then arr[i] written */
        arr[i-1] = arr[i-2] + brr[i-1];  /* Another RAW with distance 1 */
        sum += arr[i];
    }
    return sum;
}

/* Function to create anti and output dependencies (WAR/WAW) */
int test_war_waw_dep(float *fa, float *fb, int n) {
    float temp = 0.0f;
    
    /* Mix of WAR and WAW dependencies */
    for (int i = 0; i < n; i++) {
        float t1 = fa[i] + fb[i];    /* Read fa[i], fb[i] */
        fa[i] = t1 * 2.0f;           /* WAR: fa[i] written after being read above */
        
        /* WAW chain */
        fb[i] = fa[i] * 0.5f;        /* Write fb[i] */
        fb[i] = fb[i] + 1.0f;        /* WAW: fb[i] written again */
        
        temp += fa[i] + fb[i];
    }
    
    /* Force dependency on volatile to prevent dead code elimination */
    temp += g_volatile;
    return (int)temp;
}

/* Function with memory aliasing dependencies */
int test_memory_aliasing(int *arr, int *ptr1, int *ptr2, int n) {
    int sum = 0;
    
    /* Use pointers that may alias */
    int *p = ptr1;
    int *q = ptr2;
    
    /* Compiler can't determine if p and q alias */
    for (int i = 0; i < n; i++) {
        *p = arr[i] + 1;      /* Memory write */
        sum += *q;            /* Memory read - may depend on previous write */
        
        /* Alternate pointers to create complex dependencies */
        if (i % 2 == 0) {
            p = &arr[i];
        } else {
            q = &arr[i];
        }
        
        /* Additional memory operations */
        arr[i] = *p + *q;
    }
    
    return sum;
}

/* Function with control dependencies */
int test_control_dep(int *arr, int *brr, int n) {
    int sum = 0;
    
    /* Loop with internal branching creating control dependencies */
    for (int i = 0; i < n; i++) {
        int val = arr[i];
        
        /* Control-dependent computation */
        if (val > 0) {
            brr[i] = val * 2;
            sum += brr[i] + 1;
        } else if (val < 0) {
            brr[i] = val / 2;
            sum += brr[i] - 1;
        } else {
            brr[i] = g_volatile;  /* Volatile access creates memory dependency */
            sum += brr[i];
        }
        
        /* Another control-dependent operation */
        switch (i % 3) {
            case 0: arr[i] = sum % 100; break;
            case 1: arr[i] = (sum * 2) % 100; break;
            default: arr[i] = (sum / 2) % 100; break;
        }
    }
    
    return sum;
}

/* Function with loop-carried dependencies across multiple iterations */
int test_long_distance_dep(double *da, int n) {
    double sum = 0.0;
    
    /* Loop with distance > 1 dependencies */
    for (int i = 4; i < n; i++) {
        da[i] = da[i-4] * 1.5 + da[i-2] * 0.5;  /* Distance 4 and 2 dependencies */
        sum += da[i];
        
        /* Additional dependency chain */
        if (i > 10) {
            da[i-5] = da[i-10] + sum;  /* Distance 5 dependency */
        }
    }
    
    return (int)sum;
}

/* Function with mixed data types and operations */
int test_mixed_operations(short *sa, int *ia, float *fa, int n) {
    int sum = 0;
    
    /* Mixed type operations creating various edge data types */
    for (int i = 1; i < n; i++) {
        /* Integer operations */
        int temp_int = ia[i] + sa[i];
        ia[i] = temp_int * 2;
        
        /* Floating point operations */
        float temp_float = fa[i] * 0.5f;
        fa[i] = temp_float + (float)ia[i];
        
        /* Type conversions create additional dependencies */
        sa[i] = (short)(fa[i] + temp_float);
        
        sum += ia[i] + sa[i] + (int)fa[i];
    }
    
    return sum;
}

/* Complex nested loop structure */
int test_nested_loops(int **matrix, int rows, int cols) {
    int total = 0;
    
    /* Outer loop with carried dependency */
    for (int i = 1; i < rows; i++) {
        int row_sum = 0;
        
        /* Inner loop with various dependencies */
        for (int j = 1; j < cols; j++) {
            /* Cross-iteration dependencies in both dimensions */
            matrix[i][j] = matrix[i-1][j] + matrix[i][j-1] - matrix[i-1][j-1];
            
            /* Anti-dependency within inner loop */
            int temp = matrix[i][j];
            matrix[i][j] = row_sum + g_volatile;
            row_sum = temp;
        }
        
        total += row_sum;
    }
    
    return total;
}

/* Main function that calls all test cases */
int main(int argc, char **argv) {
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 100) n = 100;
        if (n > 10000) n = 10000;
    }
    
    /* Initialize data arrays */
    int *arr1 = (int*)malloc(n * sizeof(int));
    int *arr2 = (int*)malloc(n * sizeof(int));
    float *arr3 = (float*)malloc(n * sizeof(float));
    double *arr4 = (double*)malloc(n * sizeof(double));
    short *arr5 = (short*)malloc(n * sizeof(short));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < n; i++) {
        arr1[i] = (i * 37) % 100;
        arr2[i] = (i * 53) % 100;
        arr3[i] = (float)(i % 50);
        arr4[i] = (double)(i % 30);
        arr5[i] = (short)(i % 100);
    }
    
    /* Allocate matrix for nested loop test */
    int rows = 50;
    int cols = 50;
    int **matrix = (int**)malloc(rows * sizeof(int*));
    for (int i = 0; i < rows; i++) {
        matrix[i] = (int*)malloc(cols * sizeof(int));
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = (i * cols + j) % 100;
        }
    }
    
    int checksum = 0;
    
    /* Execute all test functions to trigger various DDG edge creations */
    checksum += test_raw_dep(arr1, arr2, n);
    checksum += test_war_waw_dep(arr3, arr3 + n/2, n/2);  /* Overlapping arrays for WAR */
    checksum += test_memory_aliasing(arr1, &arr1[10], &arr1[20], n-20);
    checksum += test_control_dep(arr1, arr2, n);
    checksum += test_long_distance_dep(arr4, n);
    checksum += test_mixed_operations(arr5, arr1, arr3, n);
    checksum += test_nested_loops(matrix, rows, cols);
    
    /* Store to global to prevent optimization */
    g_result = checksum;
    
    /* Print result to ensure code isn't eliminated */
    printf("DDG test checksum: %d\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(arr5);
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
    
    return 0;
}
