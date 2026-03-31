/* Test program to trigger DDG edge initialization in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_iterations(void) {
    static volatile int counter = 100;
    return counter;
}

static void __attribute__((noinline, noipa)) sink(int value) {
    static volatile int sink_var;
    sink_var = value;
}

/* Test 1: Simple loop with register and memory dependencies */
static void __attribute__((noinline, noipa)) 
test_loop_carried_dependencies(int n, int* arr1, int* arr2) {
    int acc = 0;
    int prev = arr1[0];
    
    /* Complex loop with multiple dependency types */
    for (int i = 1; i < n; ++i) {
        /* RAW (true) dependency on arr1[i-1] from previous iteration */
        int temp = arr1[i-1] + i;
        
        /* WAR (anti) dependency - reading arr2[i] before writing to it */
        int read_before_write = arr2[i] * 2;
        
        /* WAW (output) dependency - multiple writes to same location */
        arr1[i] = temp + read_before_write;
        
        /* Loop-carried register dependency */
        acc = acc + arr1[i];
        
        /* Another WAW on arr2[i] */
        arr2[i] = acc % 256;
        
        /* Control dependency based on loop-variant value */
        if (acc > 1000) {
            prev = arr1[i] / 2;  /* Creates control flow edges */
        } else {
            prev = arr1[i] * 2;
        }
        
        /* Use prev to create additional dependency chain */
        arr1[i] = prev + arr2[i];
    }
    
    sink(acc);
}

/* Test 2: Nested loops for SCC formation */
static void __attribute__((noinline, noipa))
test_nested_loops_scc(int n, int m, int* matrix) {
    int sum = 0;
    
    /* Outer loop with carried dependency */
    for (int i = 1; i < n; ++i) {
        int row_acc = 0;
        
        /* Inner loop with complex dependencies */
        for (int j = 1; j < m; ++j) {
            /* Cycle within single iteration: x depends on y, y depends on x */
            int idx = i * m + j;
            int prev_idx = (i-1) * m + j;
            int left_idx = i * m + (j-1);
            
            /* Create a small SCC within the inner loop */
            int a = matrix[prev_idx] + 1;      /* RAW from previous row */
            int b = matrix[left_idx] * 2;      /* RAW from left column */
            
            /* Cross-iteration dependency in inner loop */
            row_acc = row_acc + a + b;
            
            /* WAW dependency */
            matrix[idx] = row_acc % 100;
            
            /* Anti-dependency: read after potential write */
            int verify = matrix[idx] + matrix[prev_idx];
            
            /* Update for next iteration with distance > 1 */
            if (j > 2) {
                matrix[idx-2] = verify;  /* Distance 2 dependency */
            }
        }
        
        /* Loop-carried dependency in outer loop */
        sum = sum + row_acc;
    }
    
    sink(sum);
}

/* Test 3: Pointer arithmetic and indirect accesses */
static void __attribute__((noinline, noipa))
test_pointer_aliasing(int n, int* data, int* indices) {
    int* ptr1 = data;
    int* ptr2 = data + n/2;
    int result = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Pointer-based accesses creating potential aliasing */
        int idx1 = indices[i] % n;
        int idx2 = indices[(i + 1) % n] % n;
        
        /* RAW through pointers with unknown aliasing */
        int val1 = ptr1[idx1];
        int val2 = ptr2[idx2];
        
        /* Complex computation with loop-carried dependency */
        result = result * 3 + val1 - val2;
        
        /* Write through pointer - potential WAW if aliased */
        ptr1[idx1] = result % 256;
        
        /* Another write with different distance */
        if (i >= 3) {
            ptr2[idx2 - 3] = result / 256;  /* Distance 3 dependency */
        }
        
        /* Control dependency based on computed value */
        volatile int flag = result > 500;
        if (flag) {
            ptr1[idx1] = ptr1[idx1] * 2;
        }
    }
    
    sink(result);
}

/* Test 4: Mixed data types and recurrence chain */
static void __attribute__((noinline, noipa))
test_recurrence_chain(int n, float* farr, int* iarr) {
    float f_acc = 0.0f;
    int i_acc = 0;
    
    /* Recurrence chain: x depends on y, y depends on z, z depends on x */
    float x = 1.0f, y = 2.0f, z = 3.0f;
    
    for (int i = 0; i < n; ++i) {
        /* Cycle of dependencies within single iteration */
        float new_x = y * z + farr[i];
        int new_y = (int)(x * 100) + iarr[i];
        float new_z = (float)new_y / 10.0f + x;
        
        /* Loop-carried dependencies on all three variables */
        x = new_x + f_acc;
        y = new_y + i_acc;
        z = new_z + x;
        
        /* Update accumulators with mixed types */
        f_acc = f_acc + x;
        i_acc = i_acc + (int)y;
        
        /* Memory dependencies with different distances */
        farr[i] = x;
        if (i >= 2) {
            iarr[i-2] = i_acc;  /* Distance 2 dependency */
        }
        
        /* Output dependency on same memory location */
        farr[i] = z;  /* WAW on farr[i] */
    }
    
    sink((int)(f_acc + i_acc));
}

/* Test 5: Matrix multiplication kernel style */
static void __attribute__((noinline, noipa))
test_matrix_pattern(int n, int* A, int* B, int* C) {
    /* Simplified matrix-style computation */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int sum = 0;
            
            /* Innermost loop with complex dependencies */
            for (int k = 0; k < n; ++k) {
                /* RAW dependencies from A and B */
                int a_val = A[i * n + k];
                int b_val = B[k * n + j];
                
                /* Loop-carried dependency in k-loop */
                sum = sum + a_val * b_val;
                
                /* Anti-dependency: read C before potential write */
                int old_c = C[i * n + j];
                
                /* Control dependency */
                if (sum > old_c) {
                    /* Create output dependency */
                    C[i * n + j] = sum;
                }
            }
            
            /* Final write with WAW potential */
            C[i * n + j] = sum % 1000;
        }
    }
    
    /* Compute checksum */
    int total = 0;
    for (int i = 0; i < n * n; ++i) {
        total += C[i];
    }
    sink(total);
}

int main(void) {
    /* Use volatile to get non-constant loop bounds */
    volatile int base_n = get_iterations();
    int n = base_n % 100 + 50;  /* Ensure reasonable size */
    
    /* Allocate and initialize test arrays */
    int* arr1 = (int*)malloc(n * sizeof(int));
    int* arr2 = (int*)malloc(n * sizeof(int));
    int* matrix = (int*)malloc(n * n * sizeof(int));
    int* indices = (int*)malloc(n * sizeof(int));
    float* farr = (float*)malloc(n * sizeof(float));
    int* iarr = (int*)malloc(n * sizeof(int));
    int* matA = (int*)malloc(n * n * sizeof(int));
    int* matB = (int*)malloc(n * n * sizeof(int));
    int* matC = (int*)malloc(n * n * sizeof(int));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < n; ++i) {
        arr1[i] = i * 3 % 256;
        arr2[i] = i * 7 % 256;
        indices[i] = (i * 13) % n;
        farr[i] = (float)(i % 100) / 10.0f;
        iarr[i] = i * 5 % 256;
    }
    
    for (int i = 0; i < n * n; ++i) {
        matrix[i] = i % 256;
        matA[i] = (i * 3) % 256;
        matB[i] = (i * 7) % 256;
        matC[i] = 0;
    }
    
    printf("Starting DDG edge initialization tests...\n");
    
    /* Run all test cases */
    test_loop_carried_dependencies(n, arr1, arr2);
    printf("Test 1 completed\n");
    
    test_nested_loops_scc(n, n/2, matrix);
    printf("Test 2 completed\n");
    
    test_pointer_aliasing(n, arr1, indices);
    printf("Test 3 completed\n");
    
    test_recurrence_chain(n, farr, iarr);
    printf("Test 4 completed\n");
    
    test_matrix_pattern(n/4, matA, matB, matC);  /* Smaller size for speed */
    printf("Test 5 completed\n");
    
    /* Compute final checksum */
    int final_checksum = 0;
    for (int i = 0; i < n; ++i) {
        final_checksum += arr1[i] + arr2[i] + (int)farr[i] + iarr[i];
    }
    for (int i = 0; i < n * n; ++i) {
        final_checksum += matrix[i] + matC[i];
    }
    
    printf("Final checksum: %d\n", final_checksum % 1000000);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(matrix);
    free(indices);
    free(farr);
    free(iarr);
    free(matA);
    free(matB);
    free(matC);
    
    return 0;
}
