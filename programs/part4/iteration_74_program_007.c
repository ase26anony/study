/* Test program to exercise DDG edge creation and initialization */
#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent optimization */
static int __attribute__((noinline, noipa)) get_iterations(void) {
    static volatile int counter = 100;
    return counter;
}

/* Volatile sink to prevent dead code elimination */
static volatile int sink = 0;

/* Test 1: Simple loop with register and memory dependencies */
static void __attribute__((noinline, noipa)) 
test_loop_carried_deps(int n, int* arr1, int* arr2) {
    int i;
    int acc = 0;
    int prev = arr1[0];
    
    /* Loop with multiple dependency types */
    for (i = 1; i < n; ++i) {
        /* True dependency (RAW) on arr1[i-1] from previous iteration */
        int temp = arr1[i-1] + i;
        
        /* Anti dependency (WAR) - reading arr2[i] then writing to arr1[i] */
        int val = arr2[i] * 2;
        
        /* Output dependency (WAW) - multiple writes to arr1[i] */
        arr1[i] = temp + val;
        
        /* Another write to arr1[i] creating WAW */
        if (val > 100) {
            arr1[i] = val - temp;
        }
        
        /* Loop-carried register dependency */
        acc = acc + arr1[i];
        
        /* Loop-carried memory dependency with distance 2 */
        if (i >= 2) {
            arr2[i] = arr1[i-2] + acc;
        }
        
        /* Control dependency */
        if (acc > 1000) {
            prev = arr1[i];
        }
    }
    
    sink += acc + prev;
}

/* Test 2: Nested loops for SCC formation */
static void __attribute__((noinline, noipa))
test_nested_loops_scc(int n, int m, int* mat) {
    int i, j;
    
    for (i = 1; i < n; ++i) {
        int row_acc = 0;
        
        /* Inner loop with loop-carried dependency */
        for (j = 1; j < m; ++j) {
            /* True dependency within inner loop */
            int idx = i * m + j;
            int prev_idx = i * m + (j-1);
            
            /* Loop-carried dependency in inner loop (forms SCC) */
            mat[idx] = mat[prev_idx] + mat[idx] * 2;
            
            /* Anti dependency */
            int read_val = mat[idx - m];  /* element above */
            mat[idx] = mat[idx] + read_val;
            
            /* Register dependency chain within iteration (cycle) */
            int x = row_acc + 1;
            int y = x * 2;
            row_acc = y / 3;
            
            /* Output dependency */
            mat[idx] = row_acc;
        }
        
        /* Loop-carried dependency between outer loop iterations */
        if (i > 0) {
            mat[i * m] = mat[(i-1) * m + (m-1)] + 1;
        }
    }
    
    /* Compute checksum */
    int sum = 0;
    for (i = 0; i < n * m; ++i) {
        sum += mat[i];
    }
    sink += sum;
}

/* Test 3: Complex recurrence with mixed dependencies */
static void __attribute__((noinline, noipa))
test_complex_recurrence(int n, float* a, float* b, float* c) {
    float x = 1.0f, y = 2.0f, z = 3.0f;
    int i;
    
    for (i = 0; i < n; ++i) {
        /* Cycle of dependencies within one iteration */
        float t1 = x + y;
        float t2 = y * z;
        float t3 = z - x;
        
        /* True dependencies forming a chain */
        x = t1 + a[i];
        y = t2 + b[i];
        z = t3 + c[i];
        
        /* Memory dependency with pointer aliasing possibility */
        a[i] = x * 0.5f;
        b[i] = y * 1.5f;
        c[i] = z * 2.5f;
        
        /* Loop-carried dependency with distance 3 */
        if (i >= 3) {
            a[i] = a[i] + a[i-3];
        }
        
        /* Control dependency affecting loop-carried variable */
        if (x > y) {
            z = z + 1.0f;
        } else {
            y = y - 1.0f;
        }
    }
    
    sink += (int)(x + y + z);
}

/* Test 4: Pointer arithmetic and indirect accesses */
static void __attribute__((noinline, noipa))
test_pointer_aliasing(int n, int* base, int* indices) {
    int* ptr1 = base;
    int* ptr2 = base + n/2;
    int acc = 0;
    int i;
    
    for (i = 0; i < n; ++i) {
        /* Indirect access creating potential aliasing */
        int idx = indices[i] % n;
        
        /* True dependency through pointer */
        int val1 = ptr1[idx];
        
        /* Anti dependency - read then write through potentially aliased pointer */
        int val2 = ptr2[i % (n/2)];
        
        /* Write creating output dependency */
        ptr1[idx] = val1 + val2 + i;
        
        /* Another write that may alias */
        if (idx == i % (n/2)) {
            ptr2[i % (n/2)] = val1 * 2;
        }
        
        /* Loop-carried register dependency */
        acc = acc ^ (ptr1[idx] + ptr2[i % (n/2)]);
        
        /* Memory dependency with compile-time unknown distance */
        if (i > 0) {
            int prev_idx = indices[i-1] % n;
            ptr1[prev_idx] = ptr1[prev_idx] + acc;
        }
    }
    
    sink += acc;
}

/* Test 5: Reduction with if-converted dependencies */
static void __attribute__((noinline, noipa))
test_reduction_with_control(int n, int* data, int threshold) {
    int sum_pos = 0, sum_neg = 0, count = 0;
    int i;
    
    for (i = 0; i < n; ++i) {
        int val = data[i];
        
        /* Control dependency converted to data dependency */
        int is_positive = val > threshold;
        int pos_contrib = is_positive ? val : 0;
        int neg_contrib = is_positive ? 0 : -val;
        
        /* Loop-carried dependencies */
        sum_pos = sum_pos + pos_contrib;
        sum_neg = sum_neg + neg_contrib;
        count = count + is_positive;
        
        /* Memory dependency with output dependency */
        data[i] = pos_contrib - neg_contrib;
        
        /* Additional dependency chain */
        if (i >= 2) {
            data[i] = data[i] + data[i-2];
        }
    }
    
    sink += sum_pos - sum_neg + count;
}

/* Matrix multiplication kernel for complex memory patterns */
static void __attribute__((noinline, noipa))
test_matrix_multiply(int n, int* A, int* B, int* C) {
    int i, j, k;
    
    for (i = 0; i < n; ++i) {
        for (j = 0; j < n; ++j) {
            int sum = 0;
            for (k = 0; k < n; ++k) {
                /* True dependencies on A and B */
                int a_val = A[i * n + k];
                int b_val = B[k * n + j];
                
                /* Loop-carried dependency in innermost loop */
                sum = sum + a_val * b_val;
                
                /* Anti dependency through C (read in next iteration) */
                if (k > 0) {
                    /* Read from C that was written previously */
                    int prev_c = C[i * n + j];
                    sum = sum - prev_c / 100;
                }
            }
            /* Output dependency - write to C */
            C[i * n + j] = sum;
            
            /* Loop-carried dependency in middle loop */
            if (j > 0) {
                C[i * n + j] = C[i * n + j] + C[i * n + (j-1)];
            }
        }
        
        /* Loop-carried dependency in outer loop */
        if (i > 0) {
            for (j = 0; j < n; ++j) {
                C[i * n + j] = C[i * n + j] + C[(i-1) * n + j] / 2;
            }
        }
    }
    
    /* Compute checksum */
    int total = 0;
    for (i = 0; i < n * n; ++i) {
        total += C[i];
    }
    sink += total;
}

int main(void) {
    /* Use volatile to get non-constant loop bounds */
    volatile int base_n = get_iterations();
    int n = base_n;
    
    /* Allocate arrays with sufficient size */
    int size = n > 0 ? n : 100;
    int mat_size = 50;
    
    int* arr1 = (int*)malloc(size * sizeof(int));
    int* arr2 = (int*)malloc(size * sizeof(int));
    int* mat = (int*)malloc(mat_size * mat_size * sizeof(int));
    float* fa = (float*)malloc(size * sizeof(float));
    float* fb = (float*)malloc(size * sizeof(float));
    float* fc = (float*)malloc(size * sizeof(float));
    int* indices = (int*)malloc(size * sizeof(int));
    int* data = (int*)malloc(size * sizeof(int));
    
    int* A = (int*)malloc(mat_size * mat_size * sizeof(int));
    int* B = (int*)malloc(mat_size * mat_size * sizeof(int));
    int* C = (int*)malloc(mat_size * mat_size * sizeof(int));
    
    /* Initialize arrays with non-constant patterns */
    for (int i = 0; i < size; ++i) {
        arr1[i] = i * 3;
        arr2[i] = i * 7;
        fa[i] = i * 0.5f;
        fb[i] = i * 1.5f;
        fc[i] = i * 2.5f;
        indices[i] = (i * 13) % size;
        data[i] = (i * 17) % 100;
    }
    
    for (int i = 0; i < mat_size * mat_size; ++i) {
        mat[i] = i;
        A[i] = i % 10;
        B[i] = (i * 3) % 10;
        C[i] = 0;
    }
    
    /* Run all test cases */
    test_loop_carried_deps(size, arr1, arr2);
    test_nested_loops_scc(20, 20, mat);
    test_complex_recurrence(size, fa, fb, fc);
    test_pointer_aliasing(size, arr1, indices);
    test_reduction_with_control(size, data, 50);
    test_matrix_multiply(20, A, B, C);
    
    /* Print result to ensure execution */
    printf("Result checksum: %d\n", sink);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(mat);
    free(fa);
    free(fb);
    free(fc);
    free(indices);
    free(data);
    free(A);
    free(B);
    free(C);
    
    return 0;
}
