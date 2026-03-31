#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_iteration_count(int base) {
    volatile int v = base;
    return v + 100;
}

static int __attribute__((noinline, noipa)) get_stride(void) {
    volatile int v = 2;
    return v;
}

static void __attribute__((noinline, noipa)) sink(int value) {
    volatile int v = value;
    (void)v;
}

/* Test 1: Simple loop with register and memory dependencies */
static void __attribute__((noinline, noipa)) 
test1_register_memory_deps(int n, int* arr1, int* arr2) {
    int acc = 0;
    int prev = arr1[0];
    
    for (int i = 1; i < n; ++i) {
        /* RAW: Read arr1[i] after potential write in previous iteration */
        int temp = arr1[i] + acc;          
        
        /* WAR: Write to arr1[i] after reading it above */
        arr1[i] = prev * 2;                 
        
        /* WAW: Multiple writes to acc within loop */
        acc = temp + arr2[i];               
        
        /* Loop-carried dependency with distance 1 */
        prev = arr1[i] + i;                 
        
        /* Memory dependency with distance 2 */
        if (i >= 2) {
            arr2[i] = arr2[i-2] + 1;        
        }
    }
    
    sink(acc + prev);
}

/* Test 2: Nested loops forming SCCs */
static void __attribute__((noinline, noipa))
test2_nested_loops_scc(int n, int m, int* mat) {
    int sum = 0;
    
    for (int i = 1; i < n; ++i) {
        int row_acc = 0;
        
        /* Inner loop with loop-carried dependency */
        for (int j = 1; j < m; ++j) {
            /* Cycle within iteration: x depends on y, y depends on x */
            int x = row_acc + mat[i*m + j];
            int y = x * 2 - mat[i*m + j-1];
            row_acc = y + j;
            
            /* Cross-iteration dependency in inner loop */
            mat[i*m + j] = mat[i*m + j-1] + x;
        }
        
        /* Outer loop-carried dependency */
        sum = sum + row_acc + mat[(i-1)*m + 0];
    }
    
    sink(sum);
}

/* Test 3: Complex control dependencies */
static void __attribute__((noinline, noipa))
test3_control_deps(int n, int* data, int threshold) {
    int count = 0;
    int acc_even = 0, acc_odd = 0;
    int prev_result = data[0];
    
    for (int i = 1; i < n; ++i) {
        /* Control-dependent computation */
        if (data[i] > threshold) {
            acc_even = acc_even + prev_result;
            prev_result = data[i] * 3;
            count++;
        } else {
            acc_odd = acc_odd + prev_result;
            prev_result = data[i] / 2;
        }
        
        /* Loop-carried through control-dependent variable */
        data[i] = prev_result + i;
        
        /* Anti-dependency (WAR) */
        threshold = threshold + (i % 5);
    }
    
    sink(count + acc_even + acc_odd);
}

/* Test 4: Pointer arithmetic and indirect accesses */
static void __attribute__((noinline, noipa))
test4_pointer_aliasing(int n, int* base) {
    int* ptr1 = base;
    int* ptr2 = base + n/2;
    int result = 0;
    
    for (int i = 0; i < n/2; ++i) {
        /* Potential aliasing creates complex memory deps */
        *ptr1 = *ptr2 + result;
        result = *ptr1 - i;
        
        /* Pointer movement creates different access patterns */
        ptr1++;
        ptr2--;
        
        /* Output dependency (WAW) on same location through ptr1 */
        if (i % 3 == 0) {
            *(ptr1 - 1) = result * 2;
        }
    }
    
    sink(result);
}

/* Test 5: Matrix-style computation with 2D dependencies */
static void __attribute__((noinline, noipa))
test5_matrix_pattern(int rows, int cols, int* A, int* B, int* C) {
    /* Simple matrix multiplication-like pattern */
    for (int i = 1; i < rows; ++i) {
        for (int j = 1; j < cols; ++j) {
            /* Multiple dependencies crossing loop boundaries */
            int diag = A[(i-1)*cols + (j-1)];
            int left = A[i*cols + (j-1)];
            int up = A[(i-1)*cols + j];
            
            /* Recurrence chain within iteration */
            int t1 = diag + left;
            int t2 = up * t1;
            int t3 = t2 - B[i*cols + j];
            
            /* Loop-carried in both dimensions */
            A[i*cols + j] = t3 + C[i*cols + j];
            
            /* Cross-iteration dependency with distance > 1 */
            if (j >= 2) {
                B[i*cols + j] = B[i*cols + j-2] + t1;
            }
        }
    }
    
    /* Consume results */
    volatile int check = A[(rows-1)*cols + (cols-1)] + B[0];
    (void)check;
}

/* Test 6: Mixed dependency types with function calls */
static int __attribute__((noinline, noipa)) helper(int x, int y) {
    volatile int v = x;
    return v + y;
}

static void __attribute__((noinline, noipa))
test6_mixed_with_calls(int n, int* arr) {
    int state1 = arr[0];
    int state2 = arr[1];
    
    for (int i = 2; i < n; ++i) {
        /* Function calls create opaque operations */
        int new_val = helper(state1, state2);
        
        /* True dependency chain */
        state1 = state2 + arr[i];
        state2 = new_val * i;
        
        /* Memory anti-dependency */
        arr[i-1] = state1;
        
        /* Output dependency */
        if (i % 4 == 0) {
            arr[i] = state2;  /* WAW with potential future write */
        }
    }
    
    sink(state1 + state2);
}

int main(void) {
    const int N = 1024;
    const int ROWS = 64;
    const int COLS = 64;
    
    /* Allocate and initialize arrays with volatile to prevent const propagation */
    int* arr1 = (int*)malloc(N * sizeof(int));
    int* arr2 = (int*)malloc(N * sizeof(int));
    int* matrix = (int*)malloc(ROWS * COLS * sizeof(int));
    int* data = (int*)malloc(N * sizeof(int));
    
    volatile int init_val = 1;
    for (int i = 0; i < N; ++i) {
        arr1[i] = init_val + i;
        arr2[i] = init_val * i;
        data[i] = init_val + (i % 100);
    }
    
    for (int i = 0; i < ROWS * COLS; ++i) {
        matrix[i] = (i % 50) + 1;
    }
    
    /* Get non-constant iteration counts */
    int iter_count = get_iteration_count(500);
    int threshold = get_stride() * 25;
    
    /* Execute test cases */
    test1_register_memory_deps(iter_count, arr1, arr2);
    test2_nested_loops_scc(ROWS, COLS, matrix);
    test3_control_deps(iter_count, data, threshold);
    test4_pointer_aliasing(iter_count, arr1);
    
    int* A = (int*)malloc(ROWS * COLS * sizeof(int));
    int* B = (int*)malloc(ROWS * COLS * sizeof(int));
    int* C = (int*)malloc(ROWS * COLS * sizeof(int));
    
    for (int i = 0; i < ROWS * COLS; ++i) {
        A[i] = i % 100;
        B[i] = (i + 1) % 100;
        C[i] = (i + 2) % 100;
    }
    
    test5_matrix_pattern(ROWS, COLS, A, B, C);
    test6_mixed_with_calls(iter_count, arr2);
    
    /* Aggregate results to prevent elimination */
    volatile int final_check = 0;
    for (int i = 0; i < 100; ++i) {
        final_check += arr1[i] + arr2[i] + data[i] + A[i] + B[i];
    }
    
    printf("Checksum: %d\n", final_check);
    
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
