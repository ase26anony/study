/* Test program to trigger Data Dependency Graph edge initialization */
#include <stdio.h>
#include <stdlib.h>

/* Dummy opaque functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_iterations(void) {
    volatile int v = 100;
    return v;
}

static int __attribute__((noinline, noipa)) get_value(int i) {
    return (i * 3) % 7;
}

static void __attribute__((noinline, noipa)) sink(int value) {
    volatile int v = value;
    (void)v;
}

/* Test 1: Simple loop with register and memory dependencies */
static void __attribute__((noinline, noipa)) 
test1_loop_carried_deps(int n, int* arr1, int* arr2) {
    int acc = 0;
    int prev = 0;
    
    /* Loop with multiple dependency types */
    for (int i = 1; i < n; ++i) {
        /* RAW (true) dependency: arr1[i-1] -> arr1[i] */
        arr1[i] = arr1[i-1] + get_value(i);
        
        /* WAR (anti) dependency: arr2[i] read then written */
        int temp = arr2[i];
        arr2[i] = temp * 2 + i;
        
        /* WAW (output) dependency: acc written multiple times */
        acc = acc + arr1[i];
        
        /* Loop-carried register dependency with distance 1 */
        int current = prev + arr2[i];
        sink(current);  /* Prevent elimination */
        prev = current;
    }
    
    /* Final sink to prevent dead code elimination */
    sink(acc);
    sink(prev);
}

/* Test 2: Complex recurrence chain within loop body */
static void __attribute__((noinline, noipa))
test2_recurrence_chain(int n, int* a, int* b) {
    int x = 1, y = 2, z = 3;
    
    for (int i = 0; i < n; ++i) {
        /* Create a cycle of dependencies within one iteration */
        x = y + a[i];      /* RAW: y -> x */
        y = z * b[i];      /* RAW: z -> y */
        z = x - i;         /* RAW: x -> z */
        
        /* Memory dependencies with different distances */
        if (i >= 2) {
            a[i] = b[i-2] + z;  /* Distance 2 memory dependency */
        }
        
        /* Control dependency */
        if (x > y) {
            b[i] = x * 2;
        } else {
            b[i] = y / 2;
        }
    }
    
    sink(x + y + z);
}

/* Test 3: Nested loops for SCC formation */
static void __attribute__((noinline, noipa))
test3_nested_loops_scc(int n, int m, int* matrix) {
    int sum = 0;
    
    /* Outer loop */
    for (int i = 1; i < n; ++i) {
        int row_acc = 0;
        
        /* Inner loop with loop-carried dependency */
        for (int j = 1; j < m; ++j) {
            /* 2D array access with multiple dependencies */
            int idx = i * m + j;
            int prev_idx = (i-1) * m + j;
            int left_idx = i * m + (j-1);
            
            /* Complex memory dependencies forming potential SCC */
            matrix[idx] = matrix[prev_idx] + matrix[left_idx] + get_value(j);
            
            /* Register dependency within inner loop */
            row_acc = row_acc + matrix[idx];
            
            /* Anti-dependency */
            int temp = matrix[idx];
            matrix[idx] = temp * 3 - j;
        }
        
        /* Loop-carried dependency between outer iterations */
        sum = sum + row_acc;
    }
    
    sink(sum);
}

/* Test 4: Pointer arithmetic and indirect accesses */
static void __attribute__((noinline, noipa))
test4_pointer_aliasing(int n, int* base) {
    int* p1 = base;
    int* p2 = base + n/2;
    int* p3 = base + n/3;
    
    int acc1 = 0, acc2 = 0;
    
    for (int i = 0; i < n/2; ++i) {
        /* Pointer-based accesses creating potential aliasing */
        *p1 = *p2 + *p3;      /* RAW: p2,p3 -> p1 */
        
        /* Update pointers - creates dependencies through pointers */
        p1++;
        p2--;
        p3 += (i % 2) ? 1 : -1;
        
        /* Register dependencies with pointer values */
        acc1 = acc1 + (int)(p1 - base);
        acc2 = acc2 * 2 + *p1;
        
        /* Conditional creating control dependency */
        if (acc1 > acc2) {
            *p1 = acc1 - acc2;
        } else {
            *p2 = acc2 - acc1;
        }
    }
    
    sink(acc1);
    sink(acc2);
}

/* Test 5: Mixed dependency types with varying distances */
static void __attribute__((noinline, noipa))
test5_mixed_dependencies(int n, int* arr, int* brr) {
    int r1 = 1, r2 = 2, r3 = 3;
    
    for (int i = 0; i < n; ++i) {
        /* Distance 1 memory dependency */
        if (i > 0) {
            arr[i] = arr[i-1] + r1;
        }
        
        /* Distance 2 memory dependency */
        if (i > 1) {
            brr[i] = brr[i-2] * r2;
        }
        
        /* Register dependency chain */
        r1 = r2 + get_value(i);
        r2 = r3 - i;
        r3 = r1 * 2;
        
        /* Output dependency on array */
        arr[i % 10] = r1 + r2 + r3;
        
        /* Anti-dependency through array */
        int temp = brr[i];
        brr[i] = temp + arr[i % 10];
    }
    
    sink(r1 + r2 + r3);
}

/* Test 6: Matrix multiplication style kernel */
static void __attribute__((noinline, noipa))
test6_matrix_style(int n, int* A, int* B, int* C) {
    /* Simplified matrix-style computation */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int sum = 0;
            for (int k = 0; k < n; ++k) {
                /* 3D array access pattern */
                int a_idx = i * n + k;
                int b_idx = k * n + j;
                int c_idx = i * n + j;
                
                /* Complex data flow with multiple dependencies */
                sum += A[a_idx] * B[b_idx];
                
                /* Loop-carried in innermost loop */
                A[a_idx] += k;  /* WAW dependency */
            }
            C[i * n + j] = sum;
        }
    }
    
    /* Compute checksum */
    int check = 0;
    for (int i = 0; i < n * n; ++i) {
        check += C[i];
    }
    sink(check);
}

int main(void) {
    /* Use volatile to get non-constant loop bounds */
    volatile int base_n = 100;
    int n = base_n;
    
    /* Allocate arrays with volatile elements to prevent optimization */
    int size = n + 10;
    int* arr1 = (int*)malloc(size * sizeof(int));
    int* arr2 = (int*)malloc(size * sizeof(int));
    int* matrix = (int*)malloc(size * size * sizeof(int));
    
    /* Initialize with non-constant values */
    for (int i = 0; i < size; ++i) {
        arr1[i] = get_value(i);
        arr2[i] = get_value(i * 2);
    }
    
    for (int i = 0; i < size * size; ++i) {
        matrix[i] = get_value(i % 17);
    }
    
    printf("Starting DDG edge creation tests...\n");
    
    /* Run all test cases */
    test1_loop_carried_deps(n, arr1, arr2);
    printf("Test 1 completed\n");
    
    test2_recurrence_chain(n, arr1, arr2);
    printf("Test 2 completed\n");
    
    test3_nested_loops_scc(10, 10, matrix);
    printf("Test 3 completed\n");
    
    test4_pointer_aliasing(n, arr1);
    printf("Test 4 completed\n");
    
    test5_mixed_dependencies(n, arr1, arr2);
    printf("Test 5 completed\n");
    
    /* Smaller matrix for reasonable runtime */
    int small_n = 20;
    int* A = (int*)malloc(small_n * small_n * sizeof(int));
    int* B = (int*)malloc(small_n * small_n * sizeof(int));
    int* C = (int*)malloc(small_n * small_n * sizeof(int));
    
    for (int i = 0; i < small_n * small_n; ++i) {
        A[i] = get_value(i);
        B[i] = get_value(i + 1);
        C[i] = 0;
    }
    
    test6_matrix_style(small_n, A, B, C);
    printf("Test 6 completed\n");
    
    /* Final checksum */
    volatile int final_sink = 0;
    for (int i = 0; i < size; ++i) {
        final_sink += arr1[i] + arr2[i];
    }
    
    printf("All tests completed. Final checksum: %d\n", final_sink);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(matrix);
    free(A);
    free(B);
    free(C);
    
    return 0;
}
