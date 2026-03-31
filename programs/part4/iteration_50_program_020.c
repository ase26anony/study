/* test_ddg_coverage.c
 * This program creates loops with various dependency patterns to trigger
 * DDG edge creation in GCC's instruction scheduler.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_array[1024];
int g_result = 0;

/* Function prototypes */
int test_raw_dep(int *a, int *b, int n);
int test_war_waw_dep(int *a, int *b, int *c, int n);
int test_memory_aliasing(int *arr1, int *arr2, int *p, int *q, int n);
int test_control_dep(int *data, int *flags, int n);
int test_mixed_deps(float *fa, int *ia, double *da, int n);
int test_nested_loops(int *matrix, int rows, int cols);
int test_loop_carried_dep(int *a, int n, int distance);

/* Test 1: True Data Dependencies (RAW - Read After Write) */
int test_raw_dep(int *a, int *b, int n) {
    int sum = 0;
    
    /* Multiple RAW dependencies with different distances */
    for (int i = 2; i < n; i++) {
        /* Flow dependency with distance 1 */
        a[i] = a[i-1] + b[i];
        
        /* Flow dependency with distance 2 */
        b[i] = a[i-2] * 3;
        
        /* Chain of dependencies */
        int temp = a[i] + b[i];
        sum += temp;
        a[i] = sum % 100;
    }
    
    /* Another loop with floating point RAW dependencies */
    float *fa = (float*)a;
    for (int i = 1; i < n; i++) {
        fa[i] = fa[i-1] * 1.5f + (float)i;
    }
    
    return sum;
}

/* Test 2: Anti and Output Dependencies (WAR/WAW) */
int test_war_waw_dep(int *a, int *b, int *c, int n) {
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        /* WAR (Anti-dependency): Read of a[i] before it's overwritten */
        int read_val = a[i] + b[i];
        
        /* WAW (Output-dependency): Multiple writes to same location */
        a[i] = read_val * 2;
        a[i] = a[i] + c[i];  // Overwrites previous value
        
        /* Another WAR example */
        b[i] = a[i] + 5;
        int temp = a[i];  // Read a[i] after b[i] calculation
        
        /* Complex WAW with condition */
        if (temp > 100) {
            c[i] = temp / 2;
            c[i] = c[i] * 3;  // Another WAW
        }
        
        result += a[i] + b[i] + c[i];
    }
    
    return result;
}

/* Test 3: Memory Aliasing Dependencies */
int test_memory_aliasing(int *arr1, int *arr2, int *p, int *q, int n) {
    int sum = 0;
    
    /* Force potential aliasing */
    p = &arr1[n/2];
    q = &arr2[n/3];
    
    for (int i = 1; i < n-1; i++) {
        /* Accesses that may alias */
        arr1[i] = arr2[i-1] + arr2[i+1];
        
        /* Pointer accesses with unknown relationship */
        *p = *p + arr1[i];
        *q = *q - arr2[i];
        
        /* More complex aliasing pattern */
        int *ptr1 = &arr1[i % 10];
        int *ptr2 = &arr2[i % 7];
        *ptr1 = *ptr1 + i;
        *ptr2 = *ptr2 - i;
        
        /* Volatile access creates hard dependency */
        sum += g_volatile;
        g_volatile = i;
        
        sum += arr1[i] + arr2[i];
    }
    
    return sum;
}

/* Test 4: Control Dependencies */
int test_control_dep(int *data, int *flags, int n) {
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        /* Branch creates control dependencies */
        if (flags[i] > 0) {
            data[i] = data[i] * 2;
            total += data[i];
            
            /* Nested condition */
            if (data[i] > 1000) {
                data[i] = data[i] / 3;
                total -= 50;
            }
        } else {
            data[i] = data[i] + flags[i];
            total += data[i] * 3;
        }
        
        /* Another condition with else-if chain */
        if (i % 3 == 0) {
            data[i] += 7;
        } else if (i % 3 == 1) {
            data[i] -= 5;
        } else {
            data[i] *= 2;
        }
        
        /* Loop with break/continue creates control flow */
        if (total > 1000000) {
            break;
        }
    }
    
    return total;
}

/* Test 5: Mixed Data Types and Dependencies */
int test_mixed_deps(float *fa, int *ia, double *da, int n) {
    double sum = 0.0;
    
    for (int i = 1; i < n; i++) {
        /* Mixed type dependencies */
        fa[i] = fa[i-1] + (float)ia[i];
        ia[i] = (int)fa[i] * 2;
        da[i] = da[i-1] + (double)ia[i];
        
        /* Cross-type dependencies */
        float temp_f = (float)da[i] * 0.5f;
        int temp_i = (int)temp_f;
        double temp_d = (double)temp_i * 1.5;
        
        /* Update with mixed operations */
        fa[i] += temp_f;
        ia[i] ^= temp_i;  /* XOR creates data dependency */
        da[i] = temp_d;
        
        sum += fa[i] + ia[i] + da[i];
    }
    
    return (int)sum;
}

/* Test 6: Nested Loops with Complex Dependencies */
int test_nested_loops(int *matrix, int rows, int cols) {
    int total = 0;
    
    /* Outer loop with carried dependency */
    for (int i = 1; i < rows; i++) {
        /* Inner loop with multiple dependency types */
        for (int j = 1; j < cols; j++) {
            /* RAW across rows */
            matrix[i*cols + j] = matrix[(i-1)*cols + j] + matrix[i*cols + (j-1)];
            
            /* WAR in inner loop */
            int old_val = matrix[i*cols + j];
            matrix[i*cols + j] = old_val * 2;
            
            /* WAW with condition */
            if ((i + j) % 2 == 0) {
                matrix[i*cols + j] = matrix[i*cols + j] + 10;
                matrix[i*cols + j] = matrix[i*cols + j] - 5;  // WAW
            }
            
            total += matrix[i*cols + j];
        }
        
        /* Inter-iteration dependency in outer loop */
        g_array[i % 1024] = total % 256;
    }
    
    return total;
}

/* Test 7: Loop-Carried Dependencies with Different Distances */
int test_loop_carried_dep(int *a, int n, int distance) {
    int sum = 0;
    
    /* Distance > 0 creates loop-carried dependencies */
    for (int i = distance; i < n; i++) {
        /* Dependency with specified distance */
        a[i] = a[i - distance] + i;
        
        /* Multiple distances in same loop */
        if (i >= 2) {
            a[i] += a[i-2] * 3;  /* Distance 2 */
        }
        
        if (i >= 3) {
            a[i] ^= a[i-3];  /* Distance 3, XOR dependency */
        }
        
        /* Mix with immediate dependencies */
        int temp = a[i];
        a[i] = temp * 2 - a[i];  /* Self-dependency */
        
        sum += a[i];
    }
    
    /* Another loop with variable distance */
    for (int i = 0; i < n; i++) {
        int idx = (i * 7) % n;
        if (idx > 0) {
            a[i] = a[idx] + a[0];  /* Non-constant distance */
        }
        sum += a[i];
    }
    
    return sum;
}

/* Main function that runs all tests */
int main(int argc, char **argv) {
    const int N = 1000;
    const int ROWS = 100;
    const int COLS = 100;
    
    /* Allocate and initialize test data */
    int *data1 = (int*)malloc(N * sizeof(int));
    int *data2 = (int*)malloc(N * sizeof(int));
    int *data3 = (int*)malloc(N * sizeof(int));
    int *flags = (int*)malloc(N * sizeof(int));
    float *fdata = (float*)malloc(N * sizeof(float));
    double *ddata = (double*)malloc(N * sizeof(double));
    int *matrix = (int*)malloc(ROWS * COLS * sizeof(int));
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        data1[i] = rand() % 100;
        data2[i] = rand() % 100;
        data3[i] = rand() % 100;
        flags[i] = rand() % 10;
        fdata[i] = (float)(rand() % 100) / 10.0f;
        ddata[i] = (double)(rand() % 100) / 5.0;
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        matrix[i] = rand() % 50;
    }
    
    /* Run all tests to create various DDG edges */
    int result = 0;
    
    printf("Running DDG edge creation tests...\n");
    
    /* Each test creates different dependency patterns */
    result += test_raw_dep(data1, data2, N);
    printf("Test 1 (RAW) complete\n");
    
    result += test_war_waw_dep(data1, data2, data3, N);
    printf("Test 2 (WAR/WAW) complete\n");
    
    result += test_memory_aliasing(data1, data2, &data1[0], &data2[0], N);
    printf("Test 3 (Memory Aliasing) complete\n");
    
    result += test_control_dep(data1, flags, N);
    printf("Test 4 (Control Dep) complete\n");
    
    result += test_mixed_deps(fdata, data1, ddata, N);
    printf("Test 5 (Mixed Types) complete\n");
    
    result += test_nested_loops(matrix, ROWS, COLS);
    printf("Test 6 (Nested Loops) complete\n");
    
    result += test_loop_carried_dep(data1, N, 3);
    printf("Test 7 (Loop Carried) complete\n");
    
    /* Final computation to prevent dead code elimination */
    g_result = result;
    
    /* Use result to prevent optimization */
    volatile int final_check = g_result;
    printf("Final checksum: %d\n", final_check % 1000);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    free(flags);
    free(fdata);
    free(ddata);
    free(matrix);
    
    return final_check % 256;
}
