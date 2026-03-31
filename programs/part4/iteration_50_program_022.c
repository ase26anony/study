/* test_ddg_coverage.c
 * Designed to trigger DDG edge creation in GCC's scheduler
 * Compile with: gcc -O2 -fmodulo-sched -funroll-loops -c test_ddg_coverage.c
 * Or: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_array[1024];
int g_result = 0;

/* Function to prevent dead code elimination */
static int use_value(int x) {
    g_volatile = x;
    return x & 1;
}

/* Test 1: True Data Dependencies (RAW) with loop-carried dependencies */
int test_raw_dep(int *a, int *b, int n) {
    int sum = 0;
    /* Multiple RAW dependencies with different distances */
    for (int i = 2; i < n; i++) {
        a[i] = a[i-1] + b[i];          /* Distance 1 RAW */
        a[i] += a[i-2] * 2;            /* Distance 2 RAW */
        b[i] = a[i] + i;               /* Immediate RAW */
        sum += a[i] + b[i];
    }
    return use_value(sum);
}

/* Test 2: Anti (WAR) and Output (WAW) Dependencies */
int test_war_waw_dep(float *fa, float *fb, int n) {
    float temp = 0.0f;
    /* Mix of WAR and WAW dependencies */
    for (int i = 0; i < n; i++) {
        float t1 = fa[i] + fb[i];      /* Read fa[i], fb[i] */
        fa[i] = t1 * 2.0f;             /* WAR: Write fa[i] after read above */
        float t2 = fb[i] * 3.0f;       /* WAR: Read fb[i] */
        fb[i] = t1 + t2;               /* WAW: Write fb[i] (multiple writes in loop) */
        fb[i] = fb[i] * 0.5f;          /* WAW: Another write to fb[i] */
        temp += fa[i] + fb[i];
    }
    return use_value((int)temp);
}

/* Test 3: Memory Aliasing with pointers */
int test_memory_aliasing(int *arr1, int *arr2, int *ptr1, int *ptr2, int n) {
    int sum = 0;
    /* Complex pointer arithmetic that may alias */
    for (int i = 0; i < n; i++) {
        *ptr1 = arr1[i] + arr2[i];     /* May alias with arr1/arr2 accesses */
        arr1[i] = *ptr2 + i;           /* May alias with ptr2 */
        ptr1 = &arr1[(i + 1) % n];     /* Change pointer target */
        ptr2 = &arr2[(i + 2) % n];     /* Change pointer target */
        sum += arr1[i] + arr2[i];
    }
    return use_value(sum);
}

/* Test 4: Control Dependencies with branching */
int test_control_dep(int *data, int *out, int n, int threshold) {
    int count = 0;
    /* Loop with internal control flow */
    for (int i = 0; i < n; i++) {
        int val = data[i];
        if (val > threshold) {
            out[i] = val * 2;          /* Control-dependent store */
            count += val;
        } else if (val < -threshold) {
            out[i] = val / 2;          /* Another control-dependent path */
            count -= val;
        } else {
            out[i] = val + threshold;  /* Default path */
            count++;
        }
        
        /* Nested condition to create more complex CFG */
        if (i % 3 == 0) {
            out[i] += g_volatile;      /* Volatile access in control flow */
        }
    }
    return use_value(count);
}

/* Test 5: Mixed dependencies with function calls */
int test_mixed_deps(int *a, int *b, int *c, int n) {
    int sum = 0;
    /* Loop with all dependency types */
    for (int i = 1; i < n - 1; i++) {
        /* RAW chain */
        int t1 = a[i-1] + b[i];
        
        /* WAR: Read a[i] before overwriting */
        int t2 = a[i] * 2;
        
        /* WAW: Multiple writes to b[i] */
        b[i] = t1 + t2;
        b[i] = b[i] - c[i];            /* WAW */
        
        /* Memory dependency with potential aliasing */
        a[i] = b[i+1] + t2;            /* RAW with distance 1 */
        
        /* Output dependency on c */
        c[i] = i * 3;                  /* WAW if c[i] written next iteration */
        
        /* Complex expression to prevent vectorization */
        sum += (a[i] * b[i]) / (c[i] + 1);
    }
    return use_value(sum);
}

/* Test 6: Nested loops for more complex DDG */
int test_nested_loops(int *matrix, int rows, int cols) {
    int total = 0;
    /* Nested loops create more complex dependency graphs */
    for (int i = 1; i < rows - 1; i++) {
        for (int j = 1; j < cols - 1; j++) {
            int idx = i * cols + j;
            /* Stencil computation with multiple dependencies */
            matrix[idx] = (matrix[idx - 1] +           /* Left neighbor */
                          matrix[idx + 1] +           /* Right neighbor */
                          matrix[idx - cols] +        /* Top neighbor */
                          matrix[idx + cols]) / 4;    /* Bottom neighbor */
            
            /* Anti-dependency */
            int old_val = matrix[idx];
            matrix[idx] = matrix[idx] + old_val * g_volatile;
            
            total += matrix[idx];
        }
    }
    return use_value(total);
}

/* Test 7: Loop with volatile and barrier-like operations */
int test_volatile_ops(int *arr, int n) {
    volatile int sync = 0;
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        /* Volatile read creates hard dependency */
        int v = sync;
        
        /* Computation dependent on volatile */
        arr[i] = arr[i] + v + i;
        
        /* Volatile write creates output dependency */
        sync = arr[i] & 0xFF;
        
        /* Memory barrier effect */
        result += arr[i];
        
        /* Another volatile access */
        g_volatile = i % 256;
    }
    return use_value(result);
}

/* Main function that runs all tests */
int main(int argc, char **argv) {
    int n = 1000;
    if (argc > 1) n = atoi(argv[1]);
    if (n < 10) n = 1000;
    
    /* Allocate and initialize test arrays */
    int *arr1 = (int*)malloc(n * sizeof(int));
    int *arr2 = (int*)malloc(n * sizeof(int));
    int *arr3 = (int*)malloc(n * sizeof(int));
    float *farr1 = (float*)malloc(n * sizeof(float));
    float *farr2 = (float*)malloc(n * sizeof(float));
    int *matrix = (int*)malloc(n * n * sizeof(int));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < n; i++) {
        arr1[i] = (i * 37) % 101;
        arr2[i] = (i * 53) % 103;
        arr3[i] = (i * 71) % 107;
        farr1[i] = (float)(i % 97) * 0.1f;
        farr2[i] = (float)(i % 89) * 0.2f;
    }
    
    for (int i = 0; i < n * n; i++) {
        matrix[i] = (i * 31) % 127;
    }
    
    int *ptr1 = &arr1[10];
    int *ptr2 = &arr2[20];
    
    /* Run all tests to create various DDG edges */
    int checksum = 0;
    
    checksum += test_raw_dep(arr1, arr2, n);
    checksum += test_war_waw_dep(farr1, farr2, n);
    checksum += test_memory_aliasing(arr1, arr2, ptr1, ptr2, n);
    checksum += test_control_dep(arr3, arr1, n, 50);
    checksum += test_mixed_deps(arr1, arr2, arr3, n);
    checksum += test_nested_loops(matrix, 64, 64);
    checksum += test_volatile_ops(arr3, n);
    
    /* Use results to prevent optimization */
    g_result = checksum;
    
    /* Print result to ensure code isn't dead */
    printf("DDG test checksum: %d\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(farr1);
    free(farr2);
    free(matrix);
    
    return checksum == 0 ? 0 : 1;
}
