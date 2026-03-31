/* ddg_test.c - Test program for Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_iterations(void) {
    volatile int iterations = 100;
    return iterations;
}

static int __attribute__((noinline, noipa)) get_value(int idx) {
    volatile int val = idx * 3 + 7;
    return val;
}

static void __attribute__((noinline, noipa)) use_result(int val) {
    volatile int sink = val;
    (void)sink;
}

/* Test 1: Simple loop with register and memory dependencies */
static int __attribute__((noinline, noipa)) 
test1_loop_carried_deps(int n, int* arr_a, int* arr_b) {
    int acc = 0;
    int prev = arr_a[0];
    
    /* Loop with multiple dependency types */
    for (int i = 1; i < n; ++i) {
        /* RAW (true) dependency on arr_a[i-1] from previous iteration */
        int temp = arr_a[i-1] + arr_b[i];
        
        /* WAR (anti) dependency on arr_a[i] */
        arr_a[i] = temp * 2;
        
        /* WAW (output) dependency on prev variable */
        prev = arr_a[i] - arr_b[i-1];
        
        /* Loop-carried register dependency on acc */
        acc = acc + prev;
        
        /* Control dependency */
        if (acc > 1000) {
            arr_b[i] = arr_b[i] / 2;
        } else {
            arr_b[i] = arr_b[i] * 3;
        }
    }
    
    /* Create recurrence within iteration for potential SCC */
    int x = acc;
    int y = x + 1;
    x = y * 2;
    y = x - 3;
    
    return x + y;
}

/* Test 2: Nested loops for SCC formation */
static int __attribute__((noinline, noipa))
test2_nested_loops_scc(int n, int m, int* matrix) {
    int sum = 0;
    
    /* Outer loop */
    for (int i = 1; i < n; ++i) {
        int row_acc = 0;
        
        /* Inner loop with loop-carried dependency */
        for (int j = 1; j < m; ++j) {
            /* True dependency across inner loop iterations */
            int val = matrix[(i-1)*m + j] + matrix[i*m + (j-1)];
            
            /* Anti dependency */
            matrix[i*m + j] = val * 2;
            
            /* Output dependency */
            row_acc = row_acc + matrix[i*m + j];
            
            /* Control dependency inside inner loop */
            if (row_acc > 100) {
                matrix[i*m + j] = matrix[i*m + j] / 2;
            }
        }
        
        /* Loop-carried dependency in outer loop */
        sum = sum + row_acc;
        
        /* Recurrence chain within outer iteration */
        int a = sum;
        int b = a + row_acc;
        int c = b * 2;
        a = c - row_acc;
        sum = a;
    }
    
    return sum;
}

/* Test 3: Complex pointer arithmetic with aliasing */
static int __attribute__((noinline, noipa))
test3_pointer_aliasing(int n, int* base_arr) {
    int* ptr1 = base_arr;
    int* ptr2 = base_arr + n/2;
    int result = 0;
    
    for (int i = 0; i < n/2; ++i) {
        /* Aliased memory accesses */
        int val1 = *ptr1;
        int val2 = *ptr2;
        
        /* True dependency through pointers */
        *ptr1 = val1 + val2;
        
        /* Anti dependency */
        *ptr2 = *ptr1 - val2;
        
        /* Output dependency on result */
        result = result + *ptr1 + *ptr2;
        
        /* Pointer arithmetic creating complex dependencies */
        ptr1++;
        ptr2--;
        
        /* Control dependency based on pointer values */
        if ((ptr1 - base_arr) % 3 == 0) {
            result = result / 2;
        }
    }
    
    return result;
}

/* Test 4: Mixed dependency types with volatile */
static int __attribute__((noinline, noipa))
test4_mixed_deps_volatile(int n, volatile int* varr) {
    int reg1 = 0, reg2 = 0, reg3 = 0;
    
    for (int i = 1; i < n; ++i) {
        /* Volatile read creates memory barrier */
        int v1 = varr[i-1];
        
        /* Chain of register dependencies */
        reg1 = reg2 + v1;
        reg2 = reg3 * 2;
        reg3 = reg1 - i;
        
        /* True dependency on volatile array */
        varr[i] = reg1 + reg2 + reg3;
        
        /* Anti dependency through volatile */
        int v2 = varr[i];
        
        /* Output dependency */
        reg1 = v2 + reg3;
        
        /* Complex control flow */
        switch (i % 4) {
            case 0: reg2 = reg1 * 3; break;
            case 1: reg2 = reg1 / 2; break;
            case 2: reg2 = reg1 + 5; break;
            default: reg2 = reg1 - 1; break;
        }
    }
    
    return reg1 + reg2 + reg3;
}

/* Test 5: Matrix multiplication kernel style */
static int __attribute__((noinline, noipa))
test5_matrix_style(int n, int* A, int* B, int* C) {
    int checksum = 0;
    
    /* Simplified matrix multiplication pattern */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int sum = 0;
            for (int k = 0; k < n; ++k) {
                /* True dependencies on A and B */
                int a_val = A[i*n + k];
                int b_val = B[k*n + j];
                
                /* Loop-carried dependency in innermost loop */
                sum = sum + a_val * b_val;
                
                /* Anti dependency on temporary */
                int temp = sum;
                sum = temp + (a_val % 2);
            }
            
            /* Output dependency on result matrix */
            C[i*n + j] = sum;
            
            /* Loop-carried in middle loop */
            checksum = checksum + sum;
        }
        
        /* Control dependency in outer loop */
        if (checksum > 1000000) {
            checksum = checksum / 2;
        }
    }
    
    return checksum;
}

/* Main function orchestrating all tests */
int main(void) {
    /* Use volatile to prevent compile-time computation */
    volatile int N = get_iterations();
    int M = 50;
    
    /* Allocate arrays with dynamic sizes */
    int size = N * M;
    int* arr1 = (int*)malloc(size * sizeof(int));
    int* arr2 = (int*)malloc(size * sizeof(int));
    int* arr3 = (int*)malloc(size * sizeof(int));
    volatile int* varr = (volatile int*)malloc(N * sizeof(int));
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < size; ++i) {
        arr1[i] = get_value(i);
        arr2[i] = get_value(i * 2);
        arr3[i] = get_value(i * 3);
    }
    
    for (int i = 0; i < N; ++i) {
        varr[i] = get_value(i * 5);
    }
    
    int total_result = 0;
    
    /* Execute all test cases */
    total_result += test1_loop_carried_deps(N, arr1, arr2);
    total_result += test2_nested_loops_scc(N/2, M/2, arr3);
    total_result += test3_pointer_aliasing(N, arr1);
    total_result += test4_mixed_deps_volatile(N, varr);
    total_result += test5_matrix_style(20, arr1, arr2, arr3);
    
    /* Prevent dead code elimination */
    use_result(total_result);
    
    /* Print result to ensure execution */
    printf("DDG Test Result: %d\n", total_result);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free((int*)varr);
    
    return 0;
}
