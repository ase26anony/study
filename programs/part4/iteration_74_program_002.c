#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_iterations(void) {
    volatile int n = 1000;
    return n;
}

static int __attribute__((noinline, noipa)) get_value(int i) {
    volatile int v = i % 7;
    return v;
}

static void __attribute__((noinline, noipa)) sink(int value) {
    volatile int sink_var = value;
    (void)sink_var;
}

/* Test 1: Simple loop with register and memory dependencies */
static void __attribute__((noinline, noipa)) 
test1_loop_carried_dependencies(int n, int* arr1, int* arr2) {
    int acc = 0;
    int prev = arr1[0];
    
    for (int i = 1; i < n; ++i) {
        // True dependency (RAW) on arr1[i-1] from previous iteration
        int temp = arr1[i-1] + arr2[i];
        
        // Anti dependency (WAR) - reusing 'prev' variable
        prev = temp * 2;
        
        // Output dependency (WAW) - multiple writes to arr1[i]
        arr1[i] = prev + i;
        
        // Another write to arr1[i] creating WAW
        if (i % 3 == 0) {
            arr1[i] = arr1[i] - 1;
        }
        
        // Loop-carried register dependency (distance 1)
        acc = acc + temp;
        
        // Memory dependency with distance 2
        if (i >= 2) {
            arr2[i] = arr2[i-2] + acc;
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
        
        // Inner loop with loop-carried dependency
        for (int j = 1; j < m; ++j) {
            // True dependency within inner loop (distance 1)
            int val = mat[(i-1)*m + j] + mat[i*m + (j-1)];
            
            // Anti dependency - reusing variable
            row_acc = row_acc + val;
            
            // Output dependency
            mat[i*m + j] = row_acc + (i * j);
            
            // Cycle within single iteration: x depends on y, y depends on x
            int x = row_acc + 1;
            int y = x * 2;
            row_acc = y - x;  // Completes the cycle
        }
        
        // Loop-carried dependency between outer loop iterations
        sum = sum + row_acc;
        
        // Memory dependency with non-unit distance
        if (i >= 3) {
            mat[i*m] = mat[(i-3)*m] + sum;
        }
    }
    
    sink(sum);
}

/* Test 3: Complex control dependencies */
static void __attribute__((noinline, noipa))
test3_control_dependencies(int n, int* data, int* out) {
    int threshold = 50;
    int count = 0;
    int running_sum = 0;
    
    for (int i = 0; i < n; ++i) {
        // Loop-carried dependency
        running_sum = running_sum + data[i];
        
        // Control dependency - branch depends on loop-variant value
        if (running_sum > threshold) {
            // True dependency inside controlled block
            int temp = data[i] * 2;
            
            // Anti dependency
            count = count + 1;
            
            // Output dependency
            out[i] = temp;
            
            // Modify running_sum creating feedback to control condition
            running_sum = running_sum / 2;
        } else {
            // Different dependency chain in else path
            out[i] = data[i] + count;
            
            // Another loop-carried dependency
            threshold = threshold - 1;
        }
        
        // Cross-iteration memory dependency
        if (i > 0) {
            data[i] = data[i-1] + out[i];
        }
    }
    
    sink(count + running_sum);
}

/* Test 4: Pointer-based aliasing dependencies */
static void __attribute__((noinline, noipa))
test4_pointer_aliasing(int n, int* base) {
    int* ptr1 = base;
    int* ptr2 = base + n/2;
    int* ptr3 = base + n/4;
    
    int acc1 = 0, acc2 = 0;
    
    for (int i = 0; i < n/2; ++i) {
        // Aliased writes through different pointers
        *ptr1 = *ptr2 + *ptr3;
        
        // True dependency through pointer
        int val = *ptr1 + i;
        
        // Write to potentially aliased location
        *ptr2 = val * 2;
        
        // Another read from aliased location
        acc1 = acc1 + *ptr3;
        
        // Write to third pointer (potential WAW with ptr1/ptr2)
        *ptr3 = acc1 - val;
        
        // Loop-carried register dependency
        acc2 = acc2 + *ptr1;
        
        // Pointer arithmetic creating evolving aliasing
        ptr1++;
        ptr2--;
        ptr3 += (i % 2);
    }
    
    sink(acc1 + acc2);
}

/* Test 5: Matrix multiplication style - complex 2D dependencies */
static void __attribute__((noinline, noipa))
test5_matrix_style(int n, int* A, int* B, int* C) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int sum = 0;
            
            for (int k = 0; k < n; ++k) {
                // True dependencies on A and B
                int a_val = A[i*n + k];
                int b_val = B[k*n + j];
                
                // Loop-carried dependency in innermost loop
                sum = sum + a_val * b_val;
                
                // Cross-iteration dependency in k-loop
                if (k > 0) {
                    A[i*n + k] = A[i*n + k] + (k % 5);
                }
            }
            
            // Output dependency - multiple potential writes
            C[i*n + j] = sum;
            
            // Anti dependency - reusing sum
            if (j > 0) {
                C[i*n + j] = C[i*n + j] + C[i*n + (j-1)];
            }
        }
        
        // Loop-carried dependency in i-loop
        if (i > 0) {
            B[i*n] = B[(i-1)*n] + 1;
        }
    }
    
    // Compute checksum
    int total = 0;
    for (int i = 0; i < n*n; ++i) {
        total += C[i];
    }
    sink(total);
}

/* Test 6: Recurrence chain within single iteration */
static void __attribute__((noinline, noipa))
test6_recurrence_chain(int n, int* arr) {
    int x = 1, y = 2, z = 3;
    
    for (int i = 0; i < n; ++i) {
        // Chain of dependencies within single iteration
        // x -> y -> z -> x forms a cycle
        x = y + arr[i];
        y = z * 2;
        z = x - i;
        
        // Another parallel chain
        int a = x + y;
        int b = a * z;
        arr[i] = b;
        
        // Loop-carried dependency with distance 2
        if (i >= 2) {
            x = x + arr[i-2];
        }
        
        // Control dependency based on cyclic values
        if (x > y) {
            y = y + z;
        } else {
            z = z - x;
        }
    }
    
    sink(x + y + z);
}

int main(void) {
    // Use volatile to prevent compile-time known values
    volatile int size = get_iterations();
    int n = size;
    
    // Allocate arrays with dynamic sizes
    int* arr1 = (int*)malloc(n * sizeof(int));
    int* arr2 = (int*)malloc(n * sizeof(int));
    int* arr3 = (int*)malloc(n * sizeof(int));
    int* matrix = (int*)malloc(n * n * sizeof(int));
    
    // Initialize with non-constant values
    for (int i = 0; i < n; ++i) {
        arr1[i] = get_value(i);
        arr2[i] = get_value(i + 1);
        arr3[i] = get_value(i + 2);
    }
    
    for (int i = 0; i < n * n; ++i) {
        matrix[i] = get_value(i);
    }
    
    // Execute all test cases
    test1_loop_carried_dependencies(n, arr1, arr2);
    test2_nested_loops_scc(n, n/2, matrix);
    test3_control_dependencies(n, arr3, arr1);
    test4_pointer_aliasing(n, arr2);
    
    // For matrix test, use smaller size to avoid huge allocation
    int small_n = n > 50 ? 50 : n;
    int* A = (int*)malloc(small_n * small_n * sizeof(int));
    int* B = (int*)malloc(small_n * small_n * sizeof(int));
    int* C = (int*)malloc(small_n * small_n * sizeof(int));
    
    for (int i = 0; i < small_n * small_n; ++i) {
        A[i] = get_value(i);
        B[i] = get_value(i + 100);
        C[i] = 0;
    }
    
    test5_matrix_style(small_n, A, B, C);
    test6_recurrence_chain(n, arr3);
    
    // Compute final checksum
    int final_sum = 0;
    for (int i = 0; i < n; ++i) {
        final_sum += arr1[i] + arr2[i] + arr3[i];
    }
    
    printf("Checksum: %d\n", final_sum);
    
    // Cleanup
    free(arr1);
    free(arr2);
    free(arr3);
    free(matrix);
    free(A);
    free(B);
    free(C);
    
    return 0;
}
