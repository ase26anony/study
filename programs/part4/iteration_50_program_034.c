/* test_ddg_edges.c
 * Program to trigger DDG edge creation in GCC's instruction scheduler
 * Compile with: gcc -O2 -funroll-loops -fmodulo-sched -c test_ddg_edges.c
 * Or: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -ftree-vectorize -c test_ddg_edges.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_array[1024];
int g_result = 0;

/* Function prototypes */
int test_raw_dep(int *a, int *b, int n);
int test_war_waw_dep(int *a, int *b, int *c, int n);
int test_memory_aliasing(int *arr1, int *arr2, int *ptr1, int *ptr2, int n);
int test_control_dep(int *data, int *mask, int n);
int test_mixed_deps(float *fa, int *ia, double *da, int n);
int test_nested_loop_deps(int *a, int *b, int n, int m);

/* 1. Test True Data Dependencies (RAW/flow dependencies) */
int test_raw_dep(int *a, int *b, int n) {
    int sum = 0;
    
    /* Multiple RAW dependencies with different distances */
    for (int i = 2; i < n; i++) {
        /* Flow dependency with distance 1 */
        a[i] = a[i-1] + b[i] + g_volatile;
        
        /* Flow dependency with distance 2 */
        b[i] = a[i-2] * 3 - b[i-1];
        
        /* Longer dependency chain */
        int temp = a[i-1] + b[i-2];
        a[i] = temp * 2 + a[i-3];
        
        sum += a[i] + b[i];
    }
    
    /* Another loop with floating point RAW dependencies */
    float fa[256];
    for (int i = 1; i < n && i < 256; i++) {
        fa[i] = fa[i-1] * 1.5f + i;
        sum += (int)fa[i];
    }
    
    return sum;
}

/* 2. Test Anti (WAR) and Output (WAW) Dependencies */
int test_war_waw_dep(int *a, int *b, int *c, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Anti-dependency (WAR): read after write */
        int temp = a[i] + g_volatile;  /* Read a[i] */
        a[i] = b[i] * 2;               /* Write a[i] - creates WAR with above */
        
        /* Output dependency (WAW): write after write */
        c[i] = temp * 3;
        c[i] = a[i] + c[i-1];          /* Second write to c[i] - creates WAW */
        
        /* Complex WAR/WAW mixture */
        b[i] = a[i] + c[i];            /* Read a[i], c[i] */
        a[i] = b[i] * 2;               /* Write a[i] - WAR with b[i] calculation */
        b[i] = a[i-1] + 1;             /* Write b[i] - WAW with previous b[i] write */
        
        sum += a[i] + b[i] + c[i];
    }
    
    return sum;
}

/* 3. Test Memory Aliasing Dependencies */
int test_memory_aliasing(int *arr1, int *arr2, int *ptr1, int *ptr2, int n) {
    int sum = 0;
    
    /* Force potential aliasing */
    ptr1 = arr1 + 1;
    ptr2 = arr2 - 1;
    
    for (int i = 1; i < n - 1; i++) {
        /* Memory operations that may alias */
        arr1[i] = arr2[i] * 2 + g_volatile;
        arr2[i] = ptr1[i-1] + ptr2[i+1];
        
        /* Pointer accesses with unknown relationship */
        *ptr1 = *ptr2 + i;
        ptr1++;
        ptr2--;
        
        /* Indirect indexing causing ambiguous dependencies */
        int idx = i & 0xF;
        arr1[idx] = arr2[idx] * 3;
        arr2[idx] = arr1[(idx + 1) & 0xF] + 1;
        
        sum += arr1[i] + arr2[i];
    }
    
    return sum;
}

/* 4. Test Control Dependencies */
int test_control_dep(int *data, int *mask, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Control-dependent computations */
        if (mask[i] > 0) {
            data[i] = data[i-1] * 2 + g_volatile;
            sum += data[i] * 3;
        } else {
            data[i] = data[i-2] / 2;
            sum += data[i] - 5;
        }
        
        /* Nested conditions */
        if (i % 3 == 0) {
            if (data[i] > 100) {
                mask[i] = data[i] * 2;
            } else {
                mask[i] = data[i] / 2;
            }
        } else if (i % 3 == 1) {
            mask[i] = mask[i-1] + data[i];
        } else {
            mask[i] = mask[i-2] * mask[i-1];
        }
        
        /* Loop with break condition (creates control deps) */
        for (int j = 0; j < 8; j++) {
            if (data[i] + j > 200) break;
            data[i] += j * mask[i];
        }
    }
    
    return sum;
}

/* 5. Test Mixed Data Type Dependencies */
int test_mixed_deps(float *fa, int *ia, double *da, int n) {
    int sum = 0;
    
    for (int i = 1; i < n; i++) {
        /* Mixed integer/float dependencies */
        fa[i] = fa[i-1] * 1.7f + ia[i];
        ia[i] = (int)fa[i] * 2 + ia[i-1];
        da[i] = (double)fa[i] * 0.5 + da[i-1];
        
        /* Type conversions creating dependencies */
        float ftemp = (float)ia[i-1] * 0.3f;
        int itemp = (int)da[i] * 2;
        fa[i] = ftemp + (float)itemp;
        
        sum += ia[i] + (int)fa[i] + (int)da[i];
    }
    
    return sum;
}

/* 6. Test Nested Loops with Complex Dependencies */
int test_nested_loop_deps(int *a, int *b, int n, int m) {
    int sum = 0;
    
    /* Outer loop with carried dependencies */
    for (int i = 1; i < n; i++) {
        /* Inner loop with multiple dependency types */
        for (int j = 1; j < m; j++) {
            /* RAW across inner iterations */
            a[i*m + j] = a[i*m + j-1] + b[(i-1)*m + j] + g_volatile;
            
            /* WAR within inner loop */
            int temp = b[i*m + j];
            b[i*m + j] = a[i*m + j] * 2;
            a[i*m + j] = temp + 1;
            
            /* WAW on same array */
            b[i*m + j] = a[i*m + j-1] * 3;
            b[i*m + j] = b[i*m + j] + a[(i-1)*m + j];
            
            sum += a[i*m + j] + b[i*m + j];
        }
        
        /* Inter-iteration dependency in outer loop */
        a[i*m] = a[(i-1)*m + m-1] * 2;
    }
    
    return sum;
}

/* Main function that calls all tests */
int main(int argc, char **argv) {
    int n = 1000;
    int m = 100;
    int result = 0;
    
    /* Initialize data arrays */
    int *data1 = (int*)malloc(n * sizeof(int));
    int *data2 = (int*)malloc(n * sizeof(int));
    int *data3 = (int*)malloc(n * sizeof(int));
    int *mask = (int*)malloc(n * sizeof(int));
    float *farr = (float*)malloc(n * sizeof(float));
    double *darr = (double*)malloc(n * sizeof(double));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        data1[i] = (i * 3) % 97;
        data2[i] = (i * 7) % 113;
        data3[i] = (i * 11) % 157;
        mask[i] = i % 5;
        farr[i] = (float)i * 0.7f;
        darr[i] = (double)i * 1.3;
    }
    
    /* Call all test functions to create various DDG edges */
    result += test_raw_dep(data1, data2, n);
    result += test_war_waw_dep(data1, data2, data3, n);
    result += test_memory_aliasing(data1, data2, data1 + 10, data2 + 20, n);
    result += test_control_dep(data1, mask, n);
    result += test_mixed_deps(farr, data2, darr, n);
    result += test_nested_loop_deps(data1, data2, 50, 20);
    
    /* Store to global to prevent dead code elimination */
    g_result = result;
    
    /* Print result to ensure code isn't optimized away */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    free(mask);
    free(farr);
    free(darr);
    
    return g_result != 0 ? 0 : 1;
}
