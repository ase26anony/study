/* Test program to trigger DDG edge creation and initialization */
#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_size(void) {
    volatile int size = 100;
    return size;
}

static int __attribute__((noinline, noipa)) get_value(int idx) {
    volatile int val = idx * 3 + 7;
    return val;
}

/* Test 1: Simple loop with register and memory dependencies */
static void __attribute__((noinline, noipa)) 
test1_loop_carried_deps(int n, int* arr1, int* arr2, int* arr3) {
    int acc = 0;
    int prev = arr1[0];
    
    for (int i = 1; i < n; ++i) {
        /* RAW: Read arr1[i], Write arr2[i] */
        int temp = arr1[i] + prev;  // Uses prev from previous iteration (distance 1)
        
        /* WAR: Read arr2[i-1], Write arr2[i] */
        arr2[i] = temp + arr2[i-1];  // Anti-dependency on arr2[i-1]
        
        /* WAW: Multiple writes to acc */
        acc = acc + arr2[i];  // Output dependency on acc
        
        /* Loop-carried with distance 2 */
        if (i >= 2) {
            arr3[i] = arr3[i-2] * 2;  // Distance 2 memory dependency
        }
        
        prev = temp;  // Loop-carried register dependency
    }
    
    /* Prevent dead code elimination */
    volatile int sink = acc + arr2[n-1] + arr3[n-1];
    (void)sink;
}

/* Test 2: Nested loops for SCC formation */
static void __attribute__((noinline, noipa))
test2_nested_loops_scc(int n, int m, int** matrix) {
    int outer_acc = 0;
    
    for (int i = 1; i < n; ++i) {
        int inner_acc = matrix[i][0];
        
        /* Inner loop with loop-carried dependency - forms SCC */
        for (int j = 1; j < m; ++j) {
            /* Cycle within iteration: x depends on y, y depends on x */
            int x = inner_acc + matrix[i-1][j];  // RAW from previous iteration
            int y = x * 2 - matrix[i][j-1];      // RAW within same iteration
            matrix[i][j] = y + inner_acc;        // WAW on matrix[i][j]
            inner_acc = x + y;                   // WAR on inner_acc
        }
        
        /* Cross-iteration dependency */
        outer_acc = outer_acc + inner_acc;
        
        /* Control dependency */
        if (outer_acc > 1000) {
            matrix[i][0] = outer_acc % 256;
        } else {
            matrix[i][0] = inner_acc % 128;
        }
    }
    
    volatile int sink = outer_acc + matrix[n-1][m-1];
    (void)sink;
}

/* Test 3: Complex dependencies with conditionals */
static void __attribute__((noinline, noipa))
test3_conditional_deps(int n, float* data, float* output) {
    float threshold = 50.0f;
    float state1 = data[0];
    float state2 = data[1];
    
    for (int i = 2; i < n; ++i) {
        /* Multiple interleaved dependencies */
        float val1 = data[i] + state1;           // RAW on state1 (distance 1)
        float val2 = data[i-1] * state2;         // RAW on state2 (distance 1)
        
        /* Control dependency based on loop-variant value */
        if (val1 > threshold) {
            output[i] = val1 - val2;             // RAW on val1, val2
            state1 = output[i-1] + 1.0f;         // RAW on output[i-1], WAR on state1
        } else {
            output[i] = val2 + threshold;
            state1 = val1 * 0.5f;                // WAR on state1
        }
        
        /* Output dependency chain */
        state2 = output[i] / 2.0f;               // RAW on output[i], WAR on state2
        
        /* Additional memory dependency with distance 3 */
        if (i >= 3) {
            data[i] = data[i-3] + output[i-2];   // RAW on data[i-3], output[i-2]
        }
    }
    
    volatile float sink = state1 + state2 + output[n-1];
    (void)sink;
}

/* Test 4: Pointer arithmetic and indirect accesses */
static void __attribute__((noinline, noipa))
test4_pointer_aliasing(int n, int* base, int* indices) {
    int* ptr1 = base;
    int* ptr2 = base + n/2;
    int sum = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Pointer-based accesses creating potential aliasing */
        int idx = indices[i] % n;
        
        /* RAW through pointers */
        int val = *ptr1 + ptr2[idx];             // May alias
        
        /* WAW on memory through pointer */
        *ptr1 = val + i;                         // Output dependency
        
        /* Loop-carried through pointer arithmetic */
        ptr1 = base + ((i + 1) % n);             // Changes pointer each iteration
        
        /* Anti-dependency through array */
        indices[i] = val % 100;                  // WAR on indices[i]
        
        /* Accumulator with multiple dependencies */
        sum = sum + val + ptr2[idx];             // RAW on val, ptr2[idx], WAR on sum
    }
    
    volatile int sink = sum + *base + ptr2[n/4];
    (void)sink;
}

/* Test 5: Matrix multiplication kernel with complex dependencies */
static void __attribute__((noinline, noipa))
test5_matrix_multiply(int n, int** A, int** B, int** C) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int acc = 0;
            
            /* Innermost loop with multiple dependencies */
            for (int k = 0; k < n; ++k) {
                /* RAW dependencies on A[i][k] and B[k][j] */
                int prod = A[i][k] * B[k][j];
                
                /* Loop-carried on acc */
                acc = acc + prod;                // Distance 1 register dependency
                
                /* Memory dependency with stride */
                if (k > 0) {
                    B[k][j] = B[k-1][j] + 1;     // RAW on B[k-1][j], distance 1
                }
            }
            
            /* WAW on C[i][j] */
            C[i][j] = acc;
            
            /* Cross-iteration dependency in outer loop */
            if (j > 0) {
                A[i][j] = C[i][j-1] + A[i][j];   // RAW on C[i][j-1], WAR on A[i][j]
            }
        }
    }
    
    volatile int sink = C[n-1][n-1] + A[0][0] + B[0][0];
    (void)sink;
}

/* Test 6: Recurrence chain within single iteration */
static void __attribute__((noinline, noipa))
test6_recurrence_chain(int n, int* vec) {
    int x = vec[0];
    int y = vec[1];
    int z = vec[2];
    
    for (int i = 3; i < n; ++i) {
        /* Cycle of dependencies within one iteration */
        int t1 = x + y;      // RAW on x, y
        int t2 = y * z;      // RAW on y, z
        int t3 = z - x;      // RAW on z, x
        
        /* More complex cycle */
        x = t1 + vec[i];     // RAW on t1, vec[i], WAR on x
        y = t2 + x;          // RAW on t2, x, WAR on y
        z = t3 + y;          // RAW on t3, y, WAR on z
        
        /* Memory dependency with variable distance */
        int dist = vec[i] % 4;
        if (i >= dist && dist > 0) {
            vec[i] = vec[i-dist] + x;  // RAW on vec[i-dist], x
        }
    }
    
    volatile int sink = x + y + z + vec[n-1];
    (void)sink;
}

int main(int argc, char** argv) {
    /* Use volatile to prevent compile-time optimization */
    volatile int N = get_size();
    int n = (N > 50) ? 50 : N;  /* Reasonable size */
    
    /* Allocate and initialize test arrays */
    int* arr1 = (int*)malloc(n * sizeof(int));
    int* arr2 = (int*)malloc(n * sizeof(int));
    int* arr3 = (int*)malloc(n * sizeof(int));
    float* farr = (float*)malloc(n * sizeof(float));
    int* indices = (int*)malloc(n * sizeof(int));
    
    /* Initialize with non-constant values */
    for (int i = 0; i < n; ++i) {
        arr1[i] = get_value(i);
        arr2[i] = i * 2;
        arr3[i] = i + 1;
        farr[i] = (float)(i * 1.5);
        indices[i] = (i * 3) % n;
    }
    
    /* Allocate matrix for nested tests */
    int** matrix = (int**)malloc(n * sizeof(int*));
    for (int i = 0; i < n; ++i) {
        matrix[i] = (int*)malloc(n * sizeof(int));
        for (int j = 0; j < n; ++j) {
            matrix[i][j] = i * n + j;
        }
    }
    
    printf("Starting DDG edge creation tests...\n");
    
    /* Run all test cases to trigger different DDG patterns */
    test1_loop_carried_deps(n, arr1, arr2, arr3);
    printf("Test 1 completed\n");
    
    test2_nested_loops_scc(n, n/2, matrix);
    printf("Test 2 completed\n");
    
    test3_conditional_deps(n, farr, (float*)arr3);
    printf("Test 3 completed\n");
    
    test4_pointer_aliasing(n, arr1, indices);
    printf("Test 4 completed\n");
    
    test5_matrix_multiply(n/2, matrix, matrix, matrix);
    printf("Test 5 completed\n");
    
    test6_recurrence_chain(n, arr2);
    printf("Test 6 completed\n");
    
    /* Compute checksum to ensure all computations happened */
    volatile int checksum = 0;
    for (int i = 0; i < n; ++i) {
        checksum += arr1[i] + arr2[i] + arr3[i] + (int)farr[i] + indices[i];
    }
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            checksum += matrix[i][j];
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    for (int i = 0; i < n; ++i) {
        free(matrix[i]);
    }
    free(matrix);
    free(arr1);
    free(arr2);
    free(arr3);
    free(farr);
    free(indices);
    
    return 0;
}
