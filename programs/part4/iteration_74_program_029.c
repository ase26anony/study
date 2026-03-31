/* ddg_test.c - Test program for Data Dependency Graph edge initialization */
#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent optimization */
static int __attribute__((noinline, noipa)) get_iteration_count(int seed) {
    volatile int v = seed;
    return v + 100; /* Non-constant iteration count */
}

/* Dummy sink to prevent dead code elimination */
static volatile int sink;

/* Test 1: Simple loop with register and memory dependencies */
static void __attribute__((noinline, noipa)) 
test1_loop_carried_deps(int n, int* arr1, int* arr2) {
    int acc = 0;
    int prev = arr1[0];
    
    /* Loop with multiple dependency types */
    for (int i = 1; i < n; ++i) {
        /* RAW (true) dependency on arr1[i-1] from previous iteration */
        int temp = arr1[i-1] + i;
        
        /* WAR (anti) dependency on arr1[i] */
        arr1[i-1] = arr2[i] * 2;
        
        /* WAW (output) dependency on arr1[i] */
        arr1[i] = temp + prev;
        
        /* Loop-carried register dependency */
        acc = acc + arr1[i];
        
        /* Control dependency */
        if (acc > 1000) {
            prev = arr1[i] / 2;  /* Creates additional dependencies */
        } else {
            prev = arr1[i] * 3;
        }
        
        /* Another RAW dependency with distance 2 */
        if (i >= 2) {
            arr2[i] = arr1[i-2] + acc;
        }
    }
    
    sink = acc + prev; /* Volatile sink */
}

/* Test 2: Nested loops for SCC formation */
static void __attribute__((noinline, noipa))
test2_nested_loops_scc(int n, int m, int* mat) {
    int sum = 0;
    
    /* Outer loop */
    for (int i = 1; i < n; ++i) {
        int row_acc = 0;
        
        /* Inner loop with loop-carried dependency - forms SCC */
        for (int j = 1; j < m; ++j) {
            /* Recurrence chain within single iteration */
            int x = mat[i*m + j-1] + 1;
            int y = x * 2 - mat[(i-1)*m + j];
            
            /* Cycle: x depends on y, y depends on x through mat */
            mat[i*m + j] = y + row_acc;
            row_acc = row_acc + x;
            
            /* Another dependency with distance in inner loop */
            if (j >= 3) {
                mat[i*m + j] += mat[i*m + j-3];
            }
        }
        
        /* Loop-carried dependency in outer loop */
        sum = sum + row_acc;
        
        /* Anti-dependency between iterations */
        mat[(i-1)*m + 0] = sum % 256;
    }
    
    sink = sum;
}

/* Test 3: Complex pointer arithmetic and aliasing */
static void __attribute__((noinline, noipa))
test3_pointer_aliasing(int n, int* base1, int* base2) {
    int* p1 = base1;
    int* p2 = base2;
    int* p3 = base1 + n/2; /* Potential aliasing */
    
    int acc1 = 0, acc2 = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Indirect accesses with potential aliasing */
        int val1 = *p1;
        int val2 = *p2;
        
        /* Pointer arithmetic creates complex dependencies */
        *p3 = val1 + val2 + acc1;
        
        /* Multiple updates to same location (WAW) */
        if (val1 > val2) {
            *p3 = *p3 * 2;
        }
        
        /* Loop-carried dependencies through pointers */
        acc1 = acc1 + *p1;
        acc2 = acc2 + *p2;
        
        /* Update pointers - creates control flow dependencies */
        p1 += (i % 3) + 1;
        p2 += (i % 2) + 1;
        
        /* Conditional with dependency on loop-variant value */
        if (acc1 > acc2) {
            p3 = base1 + (i % (n/2));
        } else {
            p3 = base2 + (i % (n/2));
        }
    }
    
    sink = acc1 + acc2;
}

/* Test 4: Matrix multiplication kernel style */
static void __attribute__((noinline, noipa))
test4_matrix_style(int n, int* A, int* B, int* C) {
    /* Simple matrix multiplication pattern with dependencies */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int sum = 0;
            
            /* Innermost loop with accumulation */
            for (int k = 0; k < n; ++k) {
                /* Memory dependencies from three arrays */
                sum += A[i*n + k] * B[k*n + j];
                
                /* Loop-carried dependency through sum */
                if (k > 0) {
                    /* Anti-dependency on C */
                    C[i*n + j] = sum + C[i*n + j];
                }
            }
            
            /* Output dependency on C */
            C[i*n + j] = sum;
            
            /* Cross-iteration dependency */
            if (j > 0) {
                A[i*n + j] += C[i*n + j-1];
            }
        }
    }
    
    /* Compute checksum */
    int total = 0;
    for (int i = 0; i < n*n; ++i) {
        total += C[i];
    }
    sink = total;
}

/* Test 5: Mixed scalar and array dependencies with if-conversion potential */
static void __attribute__((noinline, noipa))
test5_mixed_dependencies(int n, int* data, int* output) {
    int hist[4] = {0, 0, 0, 0};
    int last_val = data[0];
    int running_sum = 0;
    
    for (int i = 1; i < n; ++i) {
        /* Multiple interleaved dependencies */
        int curr = data[i];
        
        /* True dependency chain */
        int diff = curr - last_val;
        int abs_diff = (diff > 0) ? diff : -diff;
        
        /* Output dependency on hist[] */
        int idx = abs_diff % 4;
        hist[idx] = hist[idx] + 1;
        
        /* Anti-dependency on data[] */
        data[i-1] = running_sum % 100;
        
        /* Loop-carried through running_sum */
        running_sum = running_sum + abs_diff + hist[idx];
        
        /* Control dependency affecting multiple variables */
        if (running_sum > 1000) {
            output[i] = hist[idx] * 2;
            last_val = curr / 2;
        } else {
            output[i] = hist[idx];
            last_val = curr * 2;
        }
        
        /* Dependency with distance 3 */
        if (i >= 3) {
            output[i] += output[i-3];
        }
    }
    
    sink = running_sum + hist[0] + hist[1] + hist[2] + hist[3];
}

int main(int argc, char** argv) {
    /* Use volatile to prevent constant propagation */
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 42;
    int N = get_iteration_count(seed);
    int M = N / 2 + 5;
    
    /* Allocate arrays with dynamic sizes */
    int size = (N > 100) ? N : 100;
    int* arr1 = (int*)malloc(size * sizeof(int));
    int* arr2 = (int*)malloc(size * sizeof(int));
    int* mat = (int*)malloc(size * size * sizeof(int));
    int* output = (int*)malloc(size * sizeof(int));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < size; ++i) {
        arr1[i] = (i * 37 + seed) % 100;
        arr2[i] = (i * 73 + seed) % 100;
        output[i] = 0;
    }
    
    for (int i = 0; i < size * size; ++i) {
        mat[i] = (i * 11 + seed) % 100;
    }
    
    printf("Starting DDG edge initialization tests...\n");
    
    /* Run all test cases */
    test1_loop_carried_deps(N, arr1, arr2);
    printf("Test 1 completed\n");
    
    test2_nested_loops_scc(N/4, M/4, mat);
    printf("Test 2 completed\n");
    
    test3_pointer_aliasing(N, arr1, arr2);
    printf("Test 3 completed\n");
    
    test4_matrix_style(N/8, arr1, arr2, output);
    printf("Test 4 completed\n");
    
    test5_mixed_dependencies(N, arr1, output);
    printf("Test 5 completed\n");
    
    /* Compute final checksum */
    int final_check = 0;
    for (int i = 0; i < size && i < 50; ++i) {
        final_check += arr1[i] + arr2[i] + output[i];
    }
    
    printf("Final checksum: %d\n", final_check);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(mat);
    free(output);
    
    return 0;
}
