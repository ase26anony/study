/* ddg_test.c - Test program for DDG edge initialization coverage */
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
        /* RAW (true) dependency on arr1[i-1] from previous iteration */
        int temp = arr1[i-1] + i;
        
        /* WAR (anti) dependency on arr1[i] */
        arr1[i-1] = arr2[i] * 2;
        
        /* WAW (output) dependency on arr1[i] */
        arr1[i] = temp + prev;
        
        /* Loop-carried register dependency */
        prev = arr1[i] - acc;
        
        /* Another RAW dependency with distance 2 */
        if (i >= 2) {
            acc += arr1[i-2] * 3;
        }
        
        /* Control dependency */
        if (arr1[i] > 100) {
            arr2[i] = arr1[i] / 2;
        } else {
            arr2[i] = arr1[i] * 2;
        }
    }
    
    /* Prevent dead code elimination */
    sink(acc + prev);
}

/* Test 2: Nested loops for SCC formation */
static void __attribute__((noinline, noipa))
test2_nested_loops_scc(int n, int m, int* mat) {
    /* Create a small SCC in inner loop */
    for (int i = 1; i < n; ++i) {
        int x = mat[i * m];
        int y = 0;
        
        /* Inner loop with recurrence chain */
        for (int j = 1; j < m; ++j) {
            /* Cycle of dependencies within one iteration */
            int idx = i * m + j;
            x = y + mat[idx - 1];      /* RAW on y, RAW on mat */
            y = x * 2 - j;             /* RAW on x */
            mat[idx] = x + y;          /* RAW on x and y, WAW on mat */
            
            /* Loop-carried dependency with distance 1 */
            if (j > 1) {
                mat[idx] += mat[idx - 2] / 3;  /* RAW with distance 2 */
            }
        }
        
        /* Cross-iteration dependency */
        mat[i * m] = x + mat[(i-1) * m];
    }
    
    sink(mat[n * m - 1]);
}

/* Test 3: Complex conditional dependencies */
static void __attribute__((noinline, noipa))
test3_conditional_deps(int n, int* data, volatile int* flags) {
    int sum_even = 0;
    int sum_odd = 0;
    int last_even = 0;
    int last_odd = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Volatile read creates memory barrier */
        int flag = flags[i];
        
        /* Multiple interleaved dependencies */
        int val = data[i];
        
        /* Control-dependent computations */
        if (flag & 0x1) {
            /* True dependency chain */
            sum_even = sum_even + val;
            last_even = sum_even - last_odd;  /* WAR on last_odd */
            data[i] = last_even * 2;          /* WAW on data[i] */
        } else {
            /* Alternative dependency chain */
            sum_odd = sum_odd + val * 3;
            last_odd = sum_odd + last_even;   /* WAR on last_even */
            data[i] = last_odd / 2;           /* WAW on data[i] */
        }
        
        /* Cross-iteration dependency with variable distance */
        if (i > 0) {
            data[i] += data[i-1] & 0xFF;      /* RAW with distance 1 */
        }
        
        /* Output dependency on shared variable */
        if (val > 1000) {
            sum_even = sum_odd - val;         /* WAW on sum_even */
        }
    }
    
    sink(sum_even + sum_odd);
}

/* Test 4: Pointer arithmetic and aliasing */
static void __attribute__((noinline, noipa))
test4_pointer_aliasing(int n, int* base) {
    int* ptr1 = base;
    int* ptr2 = base + n/2;
    int* ptr3 = base + n/4;
    
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    for (int i = 0; i < n/2; ++i) {
        /* Potential aliasing creates complex memory dependencies */
        *ptr1 = *ptr2 + acc1;          /* RAW on *ptr2, WAR on acc1 */
        acc1 = *ptr1 - *ptr3;          /* RAW on *ptr1 and *ptr3 */
        
        *ptr2 = acc2 + i;              /* WAR on acc2 */
        acc2 = *ptr2 / (i + 1);        /* RAW on *ptr2 */
        
        *ptr3 = acc3 * 2;              /* WAR on acc3 */
        acc3 = *ptr3 + acc1;           /* RAW on *ptr3 and acc1 */
        
        /* Pointer arithmetic creates varying access patterns */
        ptr1++;
        if (i % 3 == 0) {
            ptr2--;
        } else {
            ptr3 += 2;
        }
        
        /* Loop-carried through pointers */
        if (i > 0) {
            *(ptr1 - 1) += acc2;       /* RAW on acc2 with distance 1 */
        }
    }
    
    sink(acc1 + acc2 + acc3);
}

/* Test 5: Matrix multiplication kernel */
static void __attribute__((noinline, noipa))
test5_matrix_multiply(int n, int* A, int* B, int* C) {
    /* Simplified matrix multiplication with dependencies */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int sum = 0;
            for (int k = 0; k < n; ++k) {
                /* True dependencies on A and B */
                int a_val = A[i * n + k];
                int b_val = B[k * n + j];
                
                /* Accumulator creates loop-carried dependency */
                sum += a_val * b_val;
                
                /* Anti-dependency on local variables */
                a_val = b_val + k;      /* WAR on b_val */
                b_val = a_val - j;      /* WAR on a_val */
                
                /* Store intermediate results */
                if (k % 2 == 0) {
                    C[i * n + j] += sum;  /* WAW on C with partial updates */
                }
            }
            /* Output dependency on C */
            C[i * n + j] = sum;
        }
        
        /* Cross-iteration dependency in outer loop */
        if (i > 0) {
            for (int j = 0; j < n; ++j) {
                C[i * n + j] += C[(i-1) * n + j] / 4;  /* RAW with distance 1 */
            }
        }
    }
    
    sink(C[n * n - 1]);
}

/* Test 6: Reduction with multiple accumulators */
static void __attribute__((noinline, noipa))
test6_multi_reduction(int n, int* data) {
    int acc1 = data[0];
    int acc2 = data[1];
    int acc3 = data[2];
    int tmp1, tmp2, tmp3;
    
    for (int i = 3; i < n; i += 3) {
        /* Interleaved reductions create complex dependency web */
        tmp1 = acc1 + data[i];         /* RAW on acc1 */
        tmp2 = acc2 + data[i+1];       /* RAW on acc2 */
        tmp3 = acc3 + data[i+2];       /* RAW on acc3 */
        
        /* Cross-accumulator dependencies */
        acc1 = tmp2 - tmp3;            /* RAW on tmp2 and tmp3 */
        acc2 = tmp3 * tmp1;            /* RAW on tmp3 and tmp1 */
        acc3 = tmp1 + tmp2;            /* RAW on tmp1 and tmp2 */
        
        /* Memory dependency with stride */
        data[i-1] = acc1;              /* WAW on data */
        data[i] = acc2;                /* WAW on data */
        data[i+1] = acc3;              /* WAW on data */
        
        /* Recurrence with distance 3 */
        if (i >= 6) {
            acc1 += data[i-5] & 0xF;   /* RAW with distance ~3 */
        }
    }
    
    sink(acc1 + acc2 + acc3);
}

int main(void) {
    /* Use volatile to prevent compile-time known values */
    volatile int size = get_iterations();
    int n = size;
    
    /* Allocate arrays with dynamic sizes */
    int* arr1 = (int*)malloc(n * sizeof(int));
    int* arr2 = (int*)malloc(n * sizeof(int));
    int* mat = (int*)malloc(n * n * sizeof(int));
    int* data = (int*)malloc(n * sizeof(int));
    volatile int* flags = (volatile int*)malloc(n * sizeof(int));
    
    /* Initialize with non-constant values */
    for (int i = 0; i < n; ++i) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 5 - 2;
        data[i] = i * 7 + 3;
        flags[i] = i % 2;
    }
    
    for (int i = 0; i < n * n; ++i) {
        mat[i] = i % 100;
    }
    
    printf("Starting DDG edge initialization tests...\n");
    
    /* Execute all test cases */
    test1_loop_carried_deps(n, arr1, arr2);
    printf("Test 1 completed\n");
    
    test2_nested_loops_scc(n, n, mat);
    printf("Test 2 completed\n");
    
    test3_conditional_deps(n, data, flags);
    printf("Test 3 completed\n");
    
    test4_pointer_aliasing(n, arr1);
    printf("Test 4 completed\n");
    
    test5_matrix_multiply(n/10, arr1, arr2, mat);
    printf("Test 5 completed\n");
    
    test6_multi_reduction(n, data);
    printf("Test 6 completed\n");
    
    /* Compute checksum to ensure all computations happened */
    int checksum = 0;
    for (int i = 0; i < n && i < 100; ++i) {
        checksum += arr1[i] + arr2[i] + data[i] + mat[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(mat);
    free(data);
    free((void*)flags);
    
    return 0;
}
