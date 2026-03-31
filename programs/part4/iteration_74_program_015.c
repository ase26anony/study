/* Test program to trigger DDG edge initialization in ddg.cc */
#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_iterations(void) {
    volatile int iterations = 100;
    return iterations;
}

static void __attribute__((noinline, noipa)) use_value(volatile int* sink, int value) {
    *sink = value;
}

/* Test 1: Simple loop with register and memory dependencies */
static void __attribute__((noinline, noipa)) 
test_loop_carried_deps(int n, int* arr1, int* arr2, volatile int* sink) {
    int acc = 0;
    int prev = arr1[0];
    
    /* Complex loop with multiple dependency types */
    for (int i = 1; i < n; ++i) {
        /* RAW (true) dependency: arr1[i] depends on prev from previous iteration */
        int temp = prev + arr2[i];  // Uses prev (loop-carried)
        
        /* WAR (anti) dependency: prev is read before being written */
        prev = arr1[i] * 2;         // Overwrites prev
        
        /* WAW (output) dependency: acc is written multiple times */
        acc = acc + temp;           // Reduction with loop-carried dependency
        
        /* Memory dependency with distance 2 */
        if (i > 2) {
            arr1[i] = arr1[i-2] + 1;  // Distance-2 loop-carried dependency
        }
        
        /* Control dependency */
        if (temp > 100) {
            acc = acc - 50;          // Control-dependent update
        }
    }
    
    /* Prevent dead code elimination */
    use_value(sink, acc + prev);
}

/* Test 2: Nested loops forming SCCs */
static void __attribute__((noinline, noipa))
test_nested_loops_scc(int n, int m, int* matrix, volatile int* sink) {
    int sum = 0;
    
    /* Outer loop with carried dependency */
    for (int i = 1; i < n; ++i) {
        int row_acc = 0;
        
        /* Inner loop with complex dependencies forming SCC */
        for (int j = 1; j < m; ++j) {
            /* Cycle of dependencies within one iteration (potential SCC) */
            int a = matrix[i*m + j];
            int b = a + row_acc;      // RAW: depends on a
            int c = b * 2;            // RAW: depends on b
            row_acc = c - matrix[(i-1)*m + j];  // RAW: depends on c, loop-carried from outer
            
            /* Anti-dependency in inner loop */
            matrix[i*m + j] = row_acc + j;  // WAR: overwrites location read in next iter
            
            /* Output dependency */
            sum = sum + row_acc;      // WAW: sum updated multiple times
        }
        
        /* Loop-carried dependency in outer loop */
        matrix[i*m + 0] = sum;        // Distance-1 in outer loop
    }
    
    use_value(sink, sum);
}

/* Test 3: Pointer arithmetic with aliasing */
static void __attribute__((noinline, noipa))
test_pointer_aliasing(int n, int* data, volatile int* sink) {
    int* ptr1 = data;
    int* ptr2 = data + n/2;
    int result = 0;
    
    /* Loop with pointer-based dependencies */
    for (int i = 0; i < n/2; ++i) {
        /* Potential aliasing creates complex memory dependencies */
        int val1 = *ptr1;             // Read via ptr1
        int val2 = *ptr2;             // Read via ptr2
        
        /* Cross-iteration dependencies through pointers */
        *ptr1 = val1 + val2;          // May alias with ptr2 accesses
        *ptr2 = val1 - val2;          // WAR on *ptr2
        
        /* Register dependency chain */
        result = result ^ val1;       // Loop-carried XOR reduction
        
        /* Pointer advancement with dependency */
        ptr1++;
        ptr2--;
        
        /* Conditional with loop-variant condition */
        if (result & 1) {
            val1 = val1 * 3;          // Control-dependent computation
            *ptr1 = val1;             // Memory update in control-dependent path
        }
    }
    
    use_value(sink, result);
}

/* Test 4: Complex recurrence with multiple SCCs */
static void __attribute__((noinline, noipa))
test_complex_recurrence(int n, int* arr, volatile int* sink) {
    int x = 1, y = 2, z = 3;
    
    /* Loop with multiple interacting recurrence chains */
    for (int i = 0; i < n; ++i) {
        /* Chain 1: x -> y -> z -> x (cycle within iteration) */
        int new_x = y + arr[i];       // Depends on y (from prev iteration or chain)
        int new_y = z * 2;            // Depends on z
        int new_z = x + i;            // Depends on x (completes cycle)
        
        /* Chain 2: Independent but with loop-carried dependency */
        arr[i] = arr[i] + new_x;      // Self-dependency with memory
        
        /* Update all for next iteration */
        x = new_x;
        y = new_y;
        z = new_z;
        
        /* Conditional that depends on the recurrence chain */
        if (x > y && y > z) {
            arr[i] = arr[i] * 2;      // Control-dependent memory op
        }
        
        /* Another output dependency */
        x = x + z;                    // WAW on x
    }
    
    use_value(sink, x + y + z);
}

/* Test 5: Matrix multiplication kernel style */
static void __attribute__((noinline, noipa))
test_matrix_style(int n, int* A, int* B, int* C, volatile int* sink) {
    /* Simplified matrix-style computation */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int sum = 0;
            
            /* Inner product with carried dependency */
            for (int k = 0; k < n; ++k) {
                /* RAW: sum depends on previous sum value */
                sum += A[i*n + k] * B[k*n + j];
                
                /* Anti-dependency through array reuse */
                A[i*n + k] = A[i*n + k] + 1;  // WAR on A
                
                /* Output dependency in inner loop */
                C[i*n + j] = sum;             // WAW on C
            }
            
            /* Loop-carried dependency in middle loop */
            B[i*n + j] = B[i*n + j] + sum;    // Distance-1 in j-loop
        }
        
        /* Loop-carried dependency in outer loop */
        A[i*n + 0] = A[i*n + 0] * 2;          // Distance-1 in i-loop
    }
    
    int checksum = 0;
    for (int i = 0; i < n * n; ++i) {
        checksum ^= C[i];
    }
    use_value(sink, checksum);
}

int main(void) {
    volatile int sink = 0;
    int iterations = get_iterations();
    
    /* Allocate arrays with volatile initialization to prevent constant prop */
    int size = iterations + 10;
    int* arr1 = (int*)malloc(size * sizeof(int));
    int* arr2 = (int*)malloc(size * sizeof(int));
    int* matrix = (int*)malloc(size * size * sizeof(int));
    int* data = (int*)malloc(size * sizeof(int));
    int* A = (int*)malloc(size * size * sizeof(int));
    int* B = (int*)malloc(size * size * sizeof(int));
    int* C = (int*)malloc(size * size * sizeof(int));
    
    /* Initialize with non-constant patterns */
    for (int i = 0; i < size; ++i) {
        arr1[i] = i * 3 % 97;
        arr2[i] = i * 7 % 101;
        data[i] = i * 11 % 103;
    }
    
    for (int i = 0; i < size * size; ++i) {
        matrix[i] = i % 107;
        A[i] = i % 109;
        B[i] = i % 113;
        C[i] = 0;
    }
    
    /* Run all test cases to exercise different DDG patterns */
    test_loop_carried_deps(iterations, arr1, arr2, &sink);
    test_nested_loops_scc(iterations/2, iterations/2, matrix, &sink);
    test_pointer_aliasing(iterations, data, &sink);
    test_complex_recurrence(iterations, arr1, &sink);
    test_matrix_style(iterations/4, A, B, C, &sink);
    
    /* Aggregate results */
    int final_result = sink;
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(matrix);
    free(data);
    free(A);
    free(B);
    free(C);
    
    printf("DDG test completed. Checksum: %d\n", final_result);
    return 0;
}
