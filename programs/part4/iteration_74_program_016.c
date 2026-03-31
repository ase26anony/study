/* Test program to exercise DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_iterations(void) {
    volatile int n = 100;
    return n;
}

static void __attribute__((noinline, noipa)) sink(int value) {
    volatile int sink_var = value;
    (void)sink_var;
}

/* Test 1: Simple loop with register and memory dependencies */
static void __attribute__((noinline, noipa)) 
test1_loop_carried_deps(int n, int* arr1, int* arr2) {
    int acc = 0;
    int prev = arr1[0];
    
    /* Complex loop with multiple dependency types */
    for (int i = 1; i < n; ++i) {
        /* RAW (true) dependency: arr1[i-1] -> arr1[i] */
        int temp = arr1[i-1] + i;
        
        /* WAR (anti) dependency: arr2[i] read, then written */
        int val = arr2[i] * 2;
        
        /* WAW (output) dependency: arr1[i] written twice */
        arr1[i] = temp + val;
        arr1[i] = arr1[i] * 3;  // Second write creates WAW
        
        /* Loop-carried register dependency with distance 1 */
        acc = acc + arr1[i];
        
        /* Loop-carried memory dependency with distance 2 */
        if (i >= 2) {
            arr2[i] = arr2[i-2] + acc;
        } else {
            arr2[i] = val;
        }
        
        /* Control dependency */
        if (acc > 1000) {
            prev = arr1[i] / 2;
        } else {
            prev = arr1[i];
        }
    }
    
    sink(acc + prev);
}

/* Test 2: Nested loops for SCC formation */
static void __attribute__((noinline, noipa))
test2_nested_loops_scc(int n, int m, int* matrix) {
    volatile int sink_sum = 0;
    
    /* Outer loop with carried dependency */
    for (int i = 1; i < n; ++i) {
        int row_acc = 0;
        
        /* Inner loop forms SCC with loop-carried dependency */
        for (int j = 1; j < m; ++j) {
            /* Cycle within iteration: x -> y -> x pattern */
            int idx = i * m + j;
            int prev_idx = (i-1) * m + j;
            int left_idx = i * m + (j-1);
            
            /* Create RAW dependencies forming a small graph */
            int x = matrix[prev_idx] + matrix[left_idx];
            int y = x * 2 + j;
            matrix[idx] = y - x;  // Uses x, creating cycle: x->y->matrix[idx]?->x
            
            /* Loop-carried in inner loop */
            row_acc = row_acc + matrix[idx];
            
            /* Anti-dependency in inner loop */
            int temp = matrix[idx];
            matrix[idx] = row_acc + temp;  // WAR: read then write same location
        }
        
        /* Loop-carried in outer loop */
        sink_sum = sink_sum + row_acc;
        
        /* Output dependency between iterations */
        matrix[i * m] = sink_sum;
    }
    
    sink(sink_sum);
}

/* Test 3: Complex recurrence chain within loop body */
static void __attribute__((noinline, noipa))
test3_recurrence_chain(int n, float* data, float* coeffs) {
    float x = 1.0f, y = 2.0f, z = 3.0f;
    
    for (int i = 0; i < n; ++i) {
        /* Recurrence chain: x -> y -> z -> x (cycle within iteration) */
        float new_x = y * coeffs[i % 8] + z;
        float new_y = x * 2.0f - new_x;
        float new_z = new_y / 3.0f + new_x;
        
        /* Loop-carried dependencies with different distances */
        x = new_z + data[i];          // Distance 1
        if (i >= 3) {
            y = new_y + data[i-3];    // Distance 3
        } else {
            y = new_y;
        }
        z = new_x * x;                // Uses current x
        
        /* Memory dependency with stride */
        int write_idx = (i * 7) % n;  // Non-linear access pattern
        data[write_idx] = x + y + z;
        
        /* Control flow creating control dependencies */
        volatile int flag = (i & 1);
        if (flag) {
            x = x * 0.5f;
        } else {
            y = y * 1.5f;
        }
    }
    
    sink((int)(x + y + z));
}

/* Test 4: Pointer arithmetic and indirect accesses */
static void __attribute__((noinline, noipa))
test4_pointer_aliasing(int n, int* base) {
    int* ptr1 = base;
    int* ptr2 = base + n/2;
    int* ptr3 = base + n/4;
    
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    for (int i = 0; i < n/2; ++i) {
        /* Pointer-based accesses creating potential aliasing */
        *ptr1 = *ptr2 + *ptr3;           // RAW from ptr2, ptr3 to ptr1
        
        /* Loop-carried through pointers */
        sum1 = sum1 + *ptr1;
        
        /* Update pointers - creates dependencies through pointer values */
        ptr1++;
        ptr2--;
        ptr3 += (i & 1) ? 1 : -1;
        
        /* Anti-dependency through memory */
        int temp = *ptr1;                // Read
        *ptr1 = sum1 + i;                // Write to same location (WAR)
        
        /* Output dependency */
        *ptr3 = temp * 2;
        *ptr3 = *ptr3 + 1;               // WAW on *ptr3
        
        /* Complex addressing */
        base[(ptr1 - base) % n] = sum1;
    }
    
    sink(sum1 + sum2 + sum3);
}

/* Test 5: Mixed data types and operations */
static void __attribute__((noinline, noipa))
test5_mixed_types(int n, double* dbl_arr, int* int_arr) {
    double dbl_acc = 0.0;
    int int_acc = 0;
    
    for (int i = 1; i < n; ++i) {
        /* Cross-type dependencies */
        double dbl_val = dbl_arr[i-1] * 1.5;
        int int_val = (int)dbl_val + int_arr[i];
        
        /* Loop-carried with different distances */
        dbl_acc = dbl_acc + dbl_val;          // Distance 1
        if (i >= 2) {
            int_acc = int_acc + int_arr[i-2]; // Distance 2
        }
        
        /* Memory dependencies with type conversion */
        dbl_arr[i] = dbl_acc + int_val;
        int_arr[i] = (int)dbl_arr[i] * 2;
        
        /* Conditional with loop-variant condition */
        if (dbl_acc > int_acc * 10.0) {
            dbl_arr[i] = dbl_arr[i] / 2.0;
            int_arr[i] = int_arr[i] + 1;
        }
        
        /* Recurrence within iteration */
        double temp = dbl_arr[i] * 0.3;
        dbl_acc = dbl_acc - temp;
        dbl_arr[i] = dbl_acc + temp;  // Creates cycle
    }
    
    sink((int)dbl_acc + int_acc);
}

/* Main driver */
int main(void) {
    /* Use volatile to prevent compile-time computation */
    volatile int iterations = get_iterations();
    int n = iterations;
    
    /* Allocate arrays with sufficient size */
    int size = 1024;
    int* arr1 = (int*)malloc(size * sizeof(int));
    int* arr2 = (int*)malloc(size * sizeof(int));
    int* matrix = (int*)malloc(size * size * sizeof(int));
    float* float_data = (float*)malloc(size * sizeof(float));
    double* dbl_arr = (double*)malloc(size * sizeof(double));
    int* int_arr = (int*)malloc(size * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < size; ++i) {
        arr1[i] = i % 37;
        arr2[i] = i % 19;
        float_data[i] = (i % 11) * 1.1f;
        dbl_arr[i] = (i % 13) * 0.7;
        int_arr[i] = i % 29;
    }
    for (int i = 0; i < size * size; ++i) {
        matrix[i] = i % 23;
    }
    
    /* Run all test cases */
    test1_loop_carried_deps(n, arr1, arr2);
    test2_nested_loops_scc(n, 64, matrix);
    test3_recurrence_chain(n, float_data, (float*)dbl_arr); /* Reuse as coeffs */
    test4_pointer_aliasing(n, arr1);
    test5_mixed_types(n, dbl_arr, int_arr);
    
    /* Compute and print checksum */
    int checksum = 0;
    for (int i = 0; i < size; ++i) {
        checksum = (checksum + arr1[i] + arr2[i] + (int)float_data[i]) & 0xFFFF;
    }
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(matrix);
    free(float_data);
    free(dbl_arr);
    free(int_arr);
    
    return 0;
}
