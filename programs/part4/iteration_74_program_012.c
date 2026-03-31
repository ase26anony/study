/* ddg_test.c - Program to exercise Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent optimization */
static int __attribute__((noinline, noipa)) get_iterations(void) {
    volatile int iterations = 100;
    return iterations;
}

/* Dummy sink to prevent dead code elimination */
static volatile int sink;

/* Test 1: Simple loop with register and memory dependencies */
static void __attribute__((noinline, noipa)) 
test1_loop_carried_deps(int n, int* arr1, int* arr2) {
    int acc = 0;
    int prev = arr1[0];
    
    /* Loop with multiple dependency types */
    for (int i = 1; i < n; ++i) {
        /* RAW (true) dependency on arr1[i-1] from previous iteration */
        int temp = arr1[i-1] + i;
        
        /* WAR (anti) dependency - reading arr2[i] before overwriting */
        int read_before_write = arr2[i] * 2;
        
        /* WAW (output) dependency - arr2[i] written twice */
        arr2[i] = temp + read_before_write;
        
        /* Loop-carried register dependency */
        acc = acc + arr1[i];
        
        /* Another WAW on arr2[i] */
        arr2[i] = arr2[i] + acc;
        
        /* Loop-carried memory dependency with distance 2 */
        if (i >= 2) {
            arr1[i] = arr1[i-2] * 3;
        }
        
        /* Control dependency */
        if (acc > 1000) {
            prev = temp;
        } else {
            prev = read_before_write;
        }
    }
    
    sink = acc + prev;
}

/* Test 2: Nested loops forming SCCs */
static void __attribute__((noinline, noipa))
test2_nested_scc(int n, int m, int* mat) {
    int sum = 0;
    
    /* Outer loop */
    for (int i = 1; i < n; ++i) {
        int row_acc = 0;
        
        /* Inner loop with loop-carried dependency - forms SCC */
        for (int j = 1; j < m; ++j) {
            /* RAW within inner loop - distance 1 */
            int val = mat[(i-1)*m + j] + mat[i*m + (j-1)];
            
            /* Loop-carried in inner loop */
            row_acc = row_acc + val;
            
            /* Write back creating WAW */
            mat[i*m + j] = row_acc;
            
            /* Another write to same location (WAW) */
            mat[i*m + j] = mat[i*m + j] * 2;
        }
        
        /* Loop-carried in outer loop */
        sum = sum + row_acc;
        
        /* Cross-iteration dependency in outer loop */
        if (i > 1) {
            mat[i*m] = mat[(i-2)*m + 1] + sum;
        }
    }
    
    sink = sum;
}

/* Test 3: Complex recurrence chain within iteration */
static void __attribute__((noinline, noipa))
test3_recurrence_chain(int n, float* data) {
    float x = 1.0f, y = 2.0f, z = 3.0f;
    
    for (int i = 0; i < n; ++i) {
        /* Chain of dependencies within single iteration - may create cycles in DDG */
        float t1 = x + data[i];      /* RAW on x */
        float t2 = y * t1;           /* RAW on y, t1 */
        float t3 = z - t2;           /* RAW on z, t2 */
        
        /* Circular dependencies */
        x = y + t3;                  /* RAW on y, t3; WAW on x */
        y = t1 * t3;                 /* RAW on t1, t3; WAW on y */
        z = x - y;                   /* RAW on x, y; WAW on z */
        
        /* Memory dependency with variable distance */
        int idx = i % 10;
        data[idx] = data[idx] + z;
        
        /* Control dependency based on computed value */
        if (x > y) {
            data[i % 10] = data[i % 10] * 2.0f;
        } else {
            data[i % 10] = data[i % 10] / 2.0f;
        }
    }
    
    sink = (int)(x + y + z);
}

/* Test 4: Pointer arithmetic and aliasing */
static void __attribute__((noinline, noipa))
test4_pointer_aliasing(int n, int* base) {
    int* ptr1 = base;
    int* ptr2 = base + n/2;
    
    int acc1 = 0, acc2 = 0;
    
    for (int i = 0; i < n/2; ++i) {
        /* Potential aliasing between ptr1 and ptr2 accesses */
        int val1 = *ptr1;
        int val2 = *ptr2;
        
        /* RAW on val1 */
        acc1 = acc1 + val1;
        
        /* RAW on val2 with loop-carried dependency */
        acc2 = acc2 * 2 + val2;
        
        /* WAW through pointers */
        *ptr1 = acc1;
        *ptr2 = acc2;
        
        /* Pointer arithmetic creating different access patterns */
        ptr1++;
        ptr2--;
        
        /* Cross-iteration memory dependency */
        if (i > 0) {
            *(base + i) = *(base + i - 1) + acc1;
        }
    }
    
    sink = acc1 + acc2;
}

/* Test 5: Mixed data types and complex index calculations */
static void __attribute__((noinline, noipa))
test5_mixed_patterns(int n, double* dbl_arr, int* int_arr) {
    double dbl_acc = 0.0;
    int int_acc = 0;
    
    volatile int offset = 3; /* Prevent constant propagation */
    
    for (int i = offset; i < n - offset; ++i) {
        /* Memory dependency with non-constant distance */
        double d1 = dbl_arr[i - offset];
        double d2 = dbl_arr[i + offset];
        
        /* RAW on d1, d2 */
        dbl_acc = dbl_acc + d1 * d2;
        
        /* Integer computation with loop-carried dependency */
        int idx = (int_acc + i) % n;
        int_arr[idx] = int_arr[idx] + i;
        
        /* WAW on dbl_arr[i] */
        dbl_arr[i] = dbl_acc;
        
        /* Another write to same location */
        dbl_arr[i] = dbl_arr[i] / 2.0;
        
        /* Control dependency affecting both accumulators */
        if (dbl_acc > 100.0) {
            int_acc = int_acc + int_arr[idx];
            dbl_arr[i] = dbl_arr[i] * 1.5;
        } else {
            int_acc = int_acc - int_arr[idx];
            dbl_arr[i] = dbl_arr[i] / 1.5;
        }
        
        /* Complex index creating potential aliasing */
        int mirror_idx = n - i - 1;
        dbl_arr[mirror_idx] = dbl_arr[i] + dbl_arr[mirror_idx];
    }
    
    sink = (int)dbl_acc + int_acc;
}

/* Main driver */
int main(void) {
    const int N = 100;
    const int M = 50;
    
    /* Allocate and initialize arrays with volatile elements */
    int* arr1 = (int*)malloc(N * sizeof(int));
    int* arr2 = (int*)malloc(N * sizeof(int));
    int* matrix = (int*)malloc(N * M * sizeof(int));
    float* float_arr = (float*)malloc(N * sizeof(float));
    double* double_arr = (double*)malloc(N * sizeof(double));
    
    /* Initialize with non-constant values */
    for (int i = 0; i < N; ++i) {
        arr1[i] = (i * 3) % 7;
        arr2[i] = (i * 5) % 11;
        float_arr[i] = (float)(i % 13) * 0.5f;
        double_arr[i] = (double)(i % 17) * 0.25;
    }
    
    for (int i = 0; i < N * M; ++i) {
        matrix[i] = (i * 7) % 19;
    }
    
    /* Get non-constant iteration count */
    int iterations = get_iterations();
    
    /* Run all test cases */
    test1_loop_carried_deps(iterations, arr1, arr2);
    test2_nested_scc(iterations/2, M, matrix);
    test3_recurrence_chain(iterations, float_arr);
    test4_pointer_aliasing(iterations, arr1);
    test5_mixed_patterns(iterations, double_arr, arr2);
    
    /* Compute checksum to ensure all computations are used */
    int checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum = (checksum + arr1[i] + arr2[i] + (int)float_arr[i]) % 1000;
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Sink value: %d\n", sink);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(matrix);
    free(float_arr);
    free(double_arr);
    
    return 0;
}
