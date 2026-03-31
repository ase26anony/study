/* ddg_test.c - Complex data dependency patterns to trigger DDG edge creation */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Opaque functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_iter_count(void) {
    volatile int count = 100;
    return count;
}

static int __attribute__((noinline, noipa)) get_stride(void) {
    volatile int stride = 2;
    return stride;
}

static void __attribute__((noinline, noipa)) use_value(volatile int* sink, int value) {
    *sink = value;
}

/* Test 1: Simple loop with register and memory dependencies */
static void __attribute__((noinline, noipa)) 
test1_register_memory_deps(int n, int* arr1, int* arr2, volatile int* sink) {
    int acc = 0;
    int prev = arr1[0];
    
    /* Complex loop with multiple dependency types */
    for (int i = 1; i < n; ++i) {
        /* RAW dependency on arr1[i] from previous iteration */
        int temp = prev + arr2[i];  // Uses prev from i-1 or initialization
        
        /* WAR dependency - arr1[i] read before written */
        int read_val = arr1[i];
        
        /* WAW dependency - arr1[i] written multiple times */
        arr1[i] = temp * 2;
        
        /* Output dependency on arr1[i] */
        arr1[i] = arr1[i] + read_val;
        
        /* Loop-carried register dependency */
        acc = acc + temp;
        
        /* Anti-dependency (WAR) on prev */
        prev = arr1[i] * 3;
        
        /* Control dependency */
        if (acc > 1000) {
            arr2[i] = arr2[i] - 1;  // Creates control flow edge
        }
    }
    
    use_value(sink, acc + arr1[n-1]);
}

/* Test 2: Nested loops for SCC formation */
static void __attribute__((noinline, noipa))
test2_nested_scc(int n, int m, int* matrix, volatile int* sink) {
    int sum = 0;
    
    /* Outer loop with carried dependency */
    for (int i = 0; i < n; ++i) {
        int row_acc = 0;
        
        /* Inner loop with complex dependencies forming SCC */
        for (int j = 0; j < m; ++j) {
            /* Cycle of dependencies within one iteration */
            int idx = i * m + j;
            
            /* Chain: a -> b -> c -> a */
            int a = matrix[idx] + row_acc;
            int b = a * 2 + j;
            int c = b - matrix[idx];
            matrix[idx] = c + a;  // Uses both c and a
            
            /* Loop-carried dependency in inner loop */
            row_acc = row_acc + matrix[idx];
            
            /* Memory dependency with distance */
            if (j >= 2) {
                matrix[idx] = matrix[idx] + matrix[idx - 2];  // Distance 2
            }
        }
        
        /* Outer loop carried dependency */
        sum = sum + row_acc;
        
        /* Cross-iteration memory dependency */
        if (i > 0) {
            matrix[i * m] = matrix[i * m] + matrix[(i-1) * m + m/2];
        }
    }
    
    use_value(sink, sum);
}

/* Test 3: Pointer arithmetic and indirect accesses */
static void __attribute__((noinline, noipa))
test3_pointer_aliasing(int n, int* data, int* indices, volatile int* sink) {
    int result = 0;
    int* ptr = data;
    
    for (int i = 0; i < n; ++i) {
        /* Pointer-based access creating potential aliasing */
        int* current = ptr + indices[i % 16];
        
        /* RAW through pointer */
        int val = *current + result;
        
        /* WAR through different pointer */
        int* other = data + (i & 15);
        int read = *other;
        
        /* WAW through pointer */
        *current = val * 2;
        
        /* Anti-dependency through pointer */
        *other = read + i;
        
        /* Loop-carried through pointer */
        ptr = data + (i % 8);
        
        /* Complex recurrence chain */
        int x = val + 1;
        int y = x * 2 - read;
        result = result + y;
        x = y / 3;  // Creates cycle: x used to compute y, then recomputed from y
    }
    
    use_value(sink, result);
}

/* Test 4: Conditional dependencies and mixed patterns */
static void __attribute__((noinline, noipa))
test4_conditional_deps(int n, int* a, int* b, int* c, volatile int* sink) {
    int acc1 = 0, acc2 = 0;
    int state = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Multiple interleaved dependencies */
        int t1 = a[i] + state;      // RAW on state
        int t2 = b[i] * 2;
        
        /* Control-dependent computation */
        if (t1 > t2) {
            acc1 = acc1 + t1;       // Control flow edge
            c[i] = t1 - t2;
            state = state + 1;      // Modified in conditional
        } else {
            acc2 = acc2 + t2;
            c[i] = t2 - t1;
            state = state - 1;      // Modified in else branch
        }
        
        /* Output dependency on a[i] */
        a[i] = c[i] * 3;
        
        /* Anti-dependency on b[i] */
        b[i] = a[i] + i;
        
        /* Loop-carried with distance 2 */
        if (i >= 2) {
            a[i] = a[i] + acc1 - acc2 + a[i-2];
        }
        
        /* Complex conditional chain */
        int cond = (acc1 > acc2) ? acc1 : acc2;
        state = state ^ cond;  // Non-linear update
    }
    
    use_value(sink, acc1 + acc2 + state);
}

/* Test 5: Matrix multiplication style - complex 2D access patterns */
static void __attribute__((noinline, noipa))
test5_matrix_style(int n, int* A, int* B, int* C, volatile int* sink) {
    /* Simplified matrix-style computation */
    for (int i = 0; i < n; ++i) {
        int* rowA = A + i * n;
        int* rowC = C + i * n;
        
        for (int j = 0; j < n; ++j) {
            int sum = 0;
            
            /* Inner product with carried dependency */
            for (int k = 0; k < n; ++k) {
                /* RAW on rowA[k] and B[k*n + j] */
                int prod = rowA[k] * B[k * n + j];
                
                /* Loop-carried in innermost loop */
                sum = sum + prod;
                
                /* Anti-dependency through B */
                if (k > 0) {
                    B[k * n + j] = B[k * n + j] + prod % 7;
                }
            }
            
            /* Output dependency on C */
            rowC[j] = sum;
            
            /* Cross-iteration dependency in middle loop */
            if (j > 0) {
                rowC[j] = rowC[j] + rowC[j-1] / 2;
            }
        }
        
        /* Outer loop carried dependency */
        if (i > 0) {
            for (int j = 0; j < n; ++j) {
                C[i * n + j] = C[i * n + j] + C[(i-1) * n + j] % 11;
            }
        }
    }
    
    /* Compute checksum */
    int total = 0;
    for (int i = 0; i < n * n; ++i) {
        total = total ^ C[i];
    }
    use_value(sink, total);
}

/* Test 6: Reduction with multiple accumulators and if-conversion */
static void __attribute__((noinline, noipa))
test6_reduction_chains(int n, int* data, volatile int* sink) {
    int sum1 = 0, sum2 = 0, sum3 = 0;
    int chain1 = data[0], chain2 = data[1];
    
    for (int i = 0; i < n; ++i) {
        /* Multiple parallel reduction chains */
        sum1 = sum1 + data[i];
        sum2 = sum2 + data[i] * 2;
        sum3 = sum3 + data[i] * 3;
        
        /* Inter-dependent chains */
        chain1 = chain1 + sum1 - sum2;
        chain2 = chain2 + sum2 - sum3;
        
        /* Cross-chain dependencies */
        sum1 = sum1 + chain2 % 5;
        sum2 = sum2 + chain1 % 7;
        
        /* Memory dependency with stride */
        if (i % 3 == 0) {
            data[i] = data[i] + sum3;
        }
        
        /* Complex recurrence to form SCC in DDG */
        int t1 = chain1 + i;
        int t2 = t1 * chain2;
        chain1 = t2 - sum1;
        chain2 = t1 + sum2;
    }
    
    use_value(sink, sum1 + sum2 + sum3 + chain1 + chain2);
}

int main(void) {
    /* Use volatile to get dynamic iteration counts */
    volatile int base_n = get_iter_count();
    int n = base_n;
    int m = get_stride() * 10;
    
    /* Allocate arrays with enough size */
    int size = n * n > 1000 ? n * n : 1000;
    int* arr1 = (int*)malloc(size * sizeof(int));
    int* arr2 = (int*)malloc(size * sizeof(int));
    int* arr3 = (int*)malloc(size * sizeof(int));
    int* indices = (int*)malloc(16 * sizeof(int));
    
    /* Initialize with non-constant patterns */
    for (int i = 0; i < size; ++i) {
        arr1[i] = (i * 3) % 97;
        arr2[i] = (i * 5) % 101;
        arr3[i] = (i * 7) % 103;
    }
    for (int i = 0; i < 16; ++i) {
        indices[i] = (i * 11) % 8;
    }
    
    volatile int sink1 = 0, sink2 = 0, sink3 = 0;
    volatile int sink4 = 0, sink5 = 0, sink6 = 0;
    
    /* Execute all test cases */
    test1_register_memory_deps(n, arr1, arr2, &sink1);
    test2_nested_scc(n/2, m/2, arr3, &sink2);
    test3_pointer_aliasing(n, arr1, indices, &sink3);
    test4_conditional_deps(n, arr2, arr3, arr1, &sink4);
    test5_matrix_style(8, arr1, arr2, arr3, &sink5);  /* Use smaller n for matrix */
    test6_reduction_chains(n, arr1, &sink6);
    
    /* Aggregate results to prevent dead code elimination */
    volatile int final_result = sink1 + sink2 + sink3 + sink4 + sink5 + sink6;
    
    printf("DDG test checksum: %d\n", final_result);
    
    free(arr1);
    free(arr2);
    free(arr3);
    free(indices);
    
    return 0;
}
