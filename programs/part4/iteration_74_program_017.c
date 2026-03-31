/* Test program to trigger DDG edge initialization in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_iterations(void) {
    volatile int n = 100;
    return n;
}

static void __attribute__((noinline, noipa)) use_value(volatile int* sink, int val) {
    *sink = val;
}

/* Test 1: Simple loop with register and memory dependencies */
static void __attribute__((noinline, noipa)) 
test1_loop_carried_deps(int n, int* arr1, int* arr2) {
    volatile int sink = 0;
    int acc = 0;
    
    /* Loop with multiple dependency types */
    for (int i = 1; i < n; ++i) {
        /* RAW (true) dependency: arr1[i] depends on arr1[i-1] */
        arr1[i] = arr1[i-1] + arr2[i];
        
        /* WAR (anti) dependency: arr2[i] read before written */
        int temp = arr2[i] * 2;
        
        /* WAW (output) dependency: acc written multiple times */
        acc = acc + temp;
        
        /* Loop-carried register dependency with distance 1 */
        arr2[i] = acc + i;
        
        /* Control dependency */
        if (arr1[i] > 100) {
            acc = acc / 2;  /* Creates control flow edge */
        }
    }
    
    use_value(&sink, acc + arr1[n-1]);
}

/* Test 2: Nested loops for SCC formation */
static void __attribute__((noinline, noipa))
test2_nested_loops_scc(int n, int m, int* matrix) {
    volatile int sink = 0;
    int sum = 0;
    
    /* Outer loop */
    for (int i = 1; i < n; ++i) {
        int row_acc = 0;
        
        /* Inner loop with loop-carried dependency */
        for (int j = 1; j < m; ++j) {
            /* Memory dependency within inner loop */
            int idx = i * m + j;
            int prev_idx = i * m + (j-1);
            
            /* RAW dependency with distance 1 in inner loop */
            matrix[idx] = matrix[prev_idx] + matrix[idx - m];
            
            /* Register dependency chain within iteration (creates cycle) */
            int x = row_acc + 1;
            int y = x * 2;
            row_acc = y - 1;
            
            /* Anti-dependency */
            int temp = matrix[idx];
            matrix[idx] = temp * 3;
        }
        
        /* Loop-carried dependency in outer loop */
        sum = sum + row_acc;
        
        /* Control dependency based on outer loop computation */
        if (sum > 1000) {
            matrix[i * m] = sum % 256;
        }
    }
    
    use_value(&sink, sum);
}

/* Test 3: Complex recurrence with pointer arithmetic */
static void __attribute__((noinline, noipa))
test3_pointer_aliasing(int n, int* data) {
    volatile int sink = 0;
    int* ptr1 = data;
    int* ptr2 = data + n/2;
    
    /* Create aliasing possibilities */
    for (int i = 0; i < n/2; ++i) {
        /* Potential aliasing through pointer arithmetic */
        *ptr1 = *ptr2 + i;
        
        /* RAW dependency through pointers */
        int val = *ptr1;
        
        /* WAW dependency - same location written twice */
        *ptr1 = val * 2;
        
        /* Anti-dependency through second pointer */
        int temp = *ptr2;
        *ptr2 = temp + *ptr1;
        
        /* Pointer movement creates loop-carried dependencies */
        ptr1++;
        ptr2--;
        
        /* Small SCC within iteration */
        int a = val + 1;
        int b = a * 3;
        int c = b - a;
        val = c / 2;
        
        /* Use val to prevent elimination */
        if (val > 100) {
            sink = val;
        }
    }
    
    use_value(&sink, *data + *(data + n/2 - 1));
}

/* Test 4: Mixed dependencies with if-else chain */
static void __attribute__((noinline, noipa))
test4_control_flow(int n, int* arr, int* brr) {
    volatile int sink = 0;
    int state = 0;
    
    for (int i = 1; i < n; ++i) {
        /* Loop-carried dependency on state */
        int old_state = state;
        
        /* Complex control flow */
        if (arr[i] > brr[i-1]) {
            /* True dependency path */
            state = old_state + arr[i];
            brr[i] = state * 2;
        } else if (arr[i] < brr[i-1]) {
            /* Alternative path with different dependencies */
            state = old_state - brr[i-1];
            brr[i] = state / 2;
            
            /* Nested control for extra complexity */
            if (state < 0) {
                arr[i] = -state;
            }
        } else {
            /* Third path */
            state = old_state * 2;
            brr[i] = arr[i] + 5;
        }
        
        /* Output dependency on arr */
        arr[i] = state + i;
        
        /* Memory dependency with variable distance */
        if (i >= 3) {
            arr[i] += arr[i-3];  /* Distance 3 dependency */
        }
    }
    
    use_value(&sink, state + arr[n-1]);
}

/* Test 5: Reduction with multiple accumulators */
static void __attribute__((noinline, noipa))
test5_multiple_reductions(int n, int* data) {
    volatile int sink = 0;
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Independent reductions */
        sum1 = sum1 + data[i];
        sum2 = sum2 + data[i] * 2;
        sum3 = sum3 + data[i] * 3;
        
        /* Cross-iteration dependency between reductions */
        if (i % 4 == 0) {
            sum1 = sum1 + sum3;
            sum3 = sum2 - sum1;
        }
        
        /* Memory update with anti-dependency */
        int old_val = data[i];
        data[i] = (sum1 + sum2 + sum3) % 100;
        
        /* Use old_val to maintain dependency */
        sum2 = sum2 + old_val;
    }
    
    use_value(&sink, sum1 + sum2 + sum3);
}

/* Test 6: Matrix-style computation */
static void __attribute__((noinline, noipa))
test6_matrix_kernel(int n, int* A, int* B, int* C) {
    volatile int sink = 0;
    
    /* Simplified matrix-style operations */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int idx = i * n + j;
            int acc = 0;
            
            /* Inner product style - creates complex dependency web */
            for (int k = 0; k < n; ++k) {
                /* RAW dependencies on A and B */
                int a_idx = i * n + k;
                int b_idx = k * n + j;
                
                /* Loop-carried dependency on acc */
                acc = acc + A[a_idx] * B[b_idx];
                
                /* Anti-dependency in the inner loop */
                int temp = A[a_idx];
                A[a_idx] = temp + 1;
            }
            
            /* WAW dependency on C */
            C[idx] = acc;
            
            /* Cross-iteration dependency */
            if (j > 0) {
                C[idx] += C[idx - 1] / 2;
            }
        }
    }
    
    /* Use result to prevent elimination */
    for (int i = 0; i < n * n; i += n + 1) {
        sink += C[i];
    }
    
    use_value(&sink, sink);
}

int main(void) {
    /* Get dynamic iteration count */
    int N = get_iterations();
    int M = N / 2;
    
    /* Allocate arrays with volatile initialization */
    int* arr1 = (int*)malloc(N * sizeof(int));
    int* arr2 = (int*)malloc(N * sizeof(int));
    int* matrix = (int*)malloc(N * M * sizeof(int));
    int* data = (int*)malloc(N * sizeof(int));
    int* A = (int*)malloc(N * N * sizeof(int));
    int* B = (int*)malloc(N * N * sizeof(int));
    int* C = (int*)malloc(N * N * sizeof(int));
    
    /* Initialize with volatile writes to prevent constant propagation */
    for (int i = 0; i < N; ++i) {
        volatile int* vptr1 = &arr1[i];
        volatile int* vptr2 = &arr2[i];
        volatile int* vptr3 = &data[i];
        *vptr1 = i * 3;
        *vptr2 = i * 7;
        *vptr3 = i * 11;
    }
    
    for (int i = 0; i < N * M; ++i) {
        volatile int* vptr = &matrix[i];
        *vptr = i % 100;
    }
    
    for (int i = 0; i < N * N; ++i) {
        volatile int* vptrA = &A[i];
        volatile int* vptrB = &B[i];
        *vptrA = i % 50;
        *vptrB = i % 30;
    }
    
    printf("Starting DDG edge initialization tests...\n");
    
    /* Run all test cases */
    test1_loop_carried_deps(N, arr1, arr2);
    printf("Test 1 completed\n");
    
    test2_nested_loops_scc(N, M, matrix);
    printf("Test 2 completed\n");
    
    test3_pointer_aliasing(N, data);
    printf("Test 3 completed\n");
    
    test4_control_flow(N, arr1, arr2);
    printf("Test 4 completed\n");
    
    test5_multiple_reductions(N, data);
    printf("Test 5 completed\n");
    
    test6_matrix_kernel(10, A, B, C);  /* Smaller size for matrix test */
    printf("Test 6 completed\n");
    
    /* Compute checksum */
    volatile int checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += arr1[i] + arr2[i] + data[i];
    }
    for (int i = 0; i < N * M; ++i) {
        checksum += matrix[i];
    }
    for (int i = 0; i < 100; ++i) {  /* Just sample C */
        checksum += C[i];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("All tests completed successfully.\n");
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(matrix);
    free(data);
    free(A);
    free(B);
    free(C);
    
    return 0;
}
