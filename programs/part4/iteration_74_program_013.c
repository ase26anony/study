#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_iter_count(void) {
    volatile int count = 100;
    return count;
}

static int __attribute__((noinline, noipa)) get_stride(void) {
    volatile int stride = 3;
    return stride;
}

static void __attribute__((noinline, noipa)) use(int val) {
    volatile int sink = val;
    (void)sink;
}

/* Test Case 1: Simple loop with register and memory dependencies */
static void __attribute__((noinline, noipa)) 
test_loop_carried_deps(int n, int* arr1, int* arr2) {
    int acc = 0;
    int prev = arr1[0];
    
    for (int i = 1; i < n; ++i) {
        /* RAW dependency on arr1[i-1] from previous iteration */
        int temp = arr1[i-1] + arr2[i];
        
        /* WAR dependency: temp is written, then used */
        acc = acc + temp;  /* Register-based loop-carried dependency */
        
        /* WAW dependency on arr1[i] */
        arr1[i] = prev * 2;
        
        /* Loop-carried memory dependency with distance 1 */
        prev = arr1[i] + temp;
        
        /* Control dependency based on loop-variant value */
        if (acc > 1000) {
            arr2[i] = arr2[i] / 2;  /* Anti-dependency on arr2[i] */
        } else {
            arr2[i] = arr2[i] * 3;  /* Output dependency on arr2[i] */
        }
    }
    
    use(acc);
    use(arr1[n-1]);
    use(arr2[n-1]);
}

/* Test Case 2: Nested loops forming SCCs */
static void __attribute__((noinline, noipa))
test_nested_loops_scc(int n, int m, int* mat) {
    int sum = 0;
    
    for (int i = 1; i < n; ++i) {
        int row_acc = 0;
        
        /* Inner loop with loop-carried dependency */
        for (int j = 1; j < m; ++j) {
            /* Complex memory dependencies forming SCC */
            int idx = i * m + j;
            int prev_idx = (i-1) * m + j;
            int left_idx = i * m + (j-1);
            
            /* Cycle of dependencies within one iteration */
            int a = mat[idx] + mat[prev_idx];
            int b = a * 2 - mat[left_idx];
            mat[idx] = b + row_acc;  /* RAW on mat[idx] */
            
            /* Loop-carried register dependency in inner loop */
            row_acc = row_acc + mat[idx] % 7;
            
            /* Anti-dependency on mat[prev_idx] */
            mat[prev_idx] = mat[prev_idx] + 1;
        }
        
        /* Loop-carried dependency between outer loop iterations */
        sum = sum + row_acc;
        
        /* Control dependency in outer loop */
        if (sum % 2 == 0) {
            mat[i * m] = sum;
        }
    }
    
    use(sum);
    use(mat[n*m/2]);
}

/* Test Case 3: Pointer arithmetic with aliasing */
static void __attribute__((noinline, noipa))
test_pointer_aliasing(int n, int* base) {
    int* ptr1 = base;
    int* ptr2 = base + n/2;
    int* ptr3 = base + n/4;
    
    int val1 = 0, val2 = 0, val3 = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Pointer-based dependencies with potential aliasing */
        *ptr1 = *ptr2 + *ptr3;  /* RAW on *ptr2 and *ptr3 */
        
        /* Anti-dependency: read then write */
        val1 = *ptr1;
        *ptr1 = val1 + i;  /* WAW on *ptr1 */
        
        /* Complex recurrence chain */
        val2 = val1 * 2 + val3;
        val3 = val2 - val1;
        val1 = val3 % 17;
        
        /* Pointer updates with stride */
        ptr1 = ptr1 + 1;
        if (i % 3 == 0) {
            ptr2 = ptr2 + 2;  /* Control-dependent pointer update */
        }
        ptr3 = ptr3 + (i % 2);  /* Data-dependent pointer update */
    }
    
    use(val1);
    use(val2);
    use(val3);
    use(*base);
}

/* Test Case 4: Matrix multiplication style dependencies */
static void __attribute__((noinline, noipa))
test_matrix_style(int n, int* A, int* B, int* C) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int acc = 0;
            
            /* Reduction loop with multiple dependencies */
            for (int k = 0; k < n; ++k) {
                /* Memory dependencies from three arrays */
                int a_idx = i * n + k;
                int b_idx = k * n + j;
                int c_idx = i * n + j;
                
                /* RAW on A[a_idx] and B[b_idx] */
                int prod = A[a_idx] * B[b_idx];
                
                /* Loop-carried register dependency in innermost loop */
                acc = acc + prod;
                
                /* Anti-dependency on A through pointer */
                if (k % 2 == 0) {
                    A[a_idx] = A[a_idx] + 1;  /* WAW on A[a_idx] */
                }
            }
            
            /* Output dependency on C */
            C[i * n + j] = acc;
            
            /* Loop-carried dependency in j-loop */
            if (j > 0) {
                C[i * n + j] = C[i * n + j] + C[i * n + (j-1)] / 2;
            }
        }
        
        /* Loop-carried dependency in i-loop */
        if (i > 0) {
            for (int j = 0; j < n; ++j) {
                B[i * n + j] = B[(i-1) * n + j] + 1;  /* RAW on B with distance 1 */
            }
        }
    }
    
    use(C[n*n/2]);
    use(A[0]);
    use(B[n-1]);
}

/* Test Case 5: Conditional dependencies with varying distances */
static void __attribute__((noinline, noipa))
test_variable_distance(int n, int* data, int* result) {
    int stride = get_stride();
    int hist[5] = {0};
    
    for (int i = 0; i < n; ++i) {
        /* Dependency with variable distance based on stride */
        int src_idx = i;
        int dst_idx = (i + stride) % n;
        
        /* True dependency with non-constant distance */
        result[dst_idx] = data[src_idx] + hist[i % 5];
        
        /* Update histogram with loop-carried dependency */
        hist[i % 5] = hist[i % 5] + data[src_idx];
        
        /* Control flow creating complex dependency web */
        switch (data[src_idx] % 4) {
            case 0:
                result[src_idx] = result[dst_idx] * 2;  /* RAW on result[dst_idx] */
                break;
            case 1:
                data[src_idx] = result[src_idx] + 1;    /* WAR on result[src_idx] */
                break;
            case 2:
                result[dst_idx] = result[dst_idx] / 2;  /* WAW on result[dst_idx] */
                break;
            default:
                /* Recurrence chain */
                int x = hist[0];
                hist[0] = x + hist[1];
                hist[1] = hist[0] - x;
                break;
        }
    }
    
    for (int i = 0; i < 5; ++i) {
        use(hist[i]);
    }
    use(result[n/2]);
}

int main(void) {
    const int N = 100;
    const int M = 50;
    
    /* Allocate and initialize arrays with volatile to prevent constant propagation */
    int* arr1 = (int*)malloc(N * sizeof(int));
    int* arr2 = (int*)malloc(N * sizeof(int));
    int* matrix = (int*)malloc(N * M * sizeof(int));
    int* A = (int*)malloc(N * N * sizeof(int));
    int* B = (int*)malloc(N * N * sizeof(int));
    int* C = (int*)malloc(N * N * sizeof(int));
    int* data = (int*)malloc(N * sizeof(int));
    int* result = (int*)malloc(N * sizeof(int));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; ++i) {
        volatile int seed = i * 1103515245 + 12345;
        arr1[i] = seed & 0xFF;
        arr2[i] = (seed >> 8) & 0xFF;
        data[i] = (seed >> 16) & 0xFF;
        result[i] = 0;
    }
    
    for (int i = 0; i < N * M; ++i) {
        matrix[i] = i % 100;
    }
    
    for (int i = 0; i < N * N; ++i) {
        A[i] = (i * 7) % 100;
        B[i] = (i * 13) % 100;
        C[i] = 0;
    }
    
    int iter_count = get_iter_count();
    
    /* Execute all test cases to trigger DDG edge creation */
    test_loop_carried_deps(iter_count, arr1, arr2);
    test_nested_loops_scc(iter_count/2, M, matrix);
    test_pointer_aliasing(iter_count, arr1);
    test_matrix_style(10, A, B, C);  /* Smaller size for matrix */
    test_variable_distance(iter_count, data, result);
    
    /* Compute and print checksum */
    volatile int checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum = checksum + arr1[i] + arr2[i] + data[i] + result[i];
    }
    for (int i = 0; i < N * M; ++i) {
        checksum = checksum + matrix[i];
    }
    for (int i = 0; i < N * N; ++i) {
        checksum = checksum + A[i] + B[i] + C[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(matrix);
    free(A);
    free(B);
    free(C);
    free(data);
    free(result);
    
    return 0;
}
