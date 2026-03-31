/* Test program to trigger DDG edge initialization in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent optimization */
static int __attribute__((noinline, noipa)) get_iterations(void) {
    volatile int n = 100;
    return n;
}

/* Dummy sink to prevent dead code elimination */
static volatile int sink;

/* Test 1: Simple loop with register and memory dependencies */
static void __attribute__((noinline, noipa)) 
test1_register_memory_deps(int n, int* a, int* b, int* c) {
    int acc = 0;
    for (int i = 1; i < n; ++i) {
        /* True dependency (RAW) on a[i-1] */
        int temp = a[i-1] + b[i];
        
        /* Anti dependency (WAR) - reusing 'temp' */
        temp = temp * 2;
        
        /* Output dependency (WAW) on a[i] */
        a[i] = temp + c[i];
        
        /* Loop-carried register dependency (distance 1) */
        acc = acc + a[i];
        
        /* Memory dependency with distance 2 */
        if (i >= 2) {
            b[i] = b[i-2] + 1;
        }
    }
    sink = acc;
}

/* Test 2: Nested loops for SCC formation */
static void __attribute__((noinline, noipa))
test2_nested_scc(int n, int m, int* mat) {
    for (int i = 1; i < n; ++i) {
        for (int j = 1; j < m; ++j) {
            /* Complex dependencies forming potential SCC */
            int idx = i * m + j;
            int idx_up = (i-1) * m + j;
            int idx_left = i * m + (j-1);
            
            /* Cycle within iteration: x depends on y, y depends on x */
            int x = mat[idx_up] + mat[idx_left];
            int y = x * 2;
            mat[idx] = y + mat[idx];
            
            /* Another dependency chain */
            mat[idx] = mat[idx] + (x - y);
        }
    }
    sink = mat[n*m/2];
}

/* Test 3: Conditional dependencies and control flow */
static void __attribute__((noinline, noipa))
test3_conditional_deps(int n, int* data, int* flags) {
    int sum_even = 0;
    int sum_odd = 0;
    
    for (int i = 1; i < n; ++i) {
        /* Loop-carried dependency with condition */
        if (flags[i] > 0) {
            sum_even = sum_even + data[i];
            data[i] = sum_even * 2;  /* WAW on data[i] */
        } else {
            sum_odd = sum_odd + data[i-1];  /* RAW on data[i-1] */
            data[i] = sum_odd / 2;
        }
        
        /* Cross-iteration dependency through condition */
        flags[i] = (sum_even > sum_odd) ? 1 : -1;
        
        /* Anti-dependency via temporary */
        int tmp = data[i];
        data[i] = tmp + i;  /* WAR on tmp */
    }
    sink = sum_even + sum_odd;
}

/* Test 4: Pointer arithmetic and indirect accesses */
static void __attribute__((noinline, noipa))
test4_pointer_aliasing(int n, int* arr1, int* arr2) {
    int* p1 = arr1;
    int* p2 = arr2;
    
    for (int i = 0; i < n-1; ++i) {
        /* Pointer-based dependencies */
        *(p1 + i) = *(p1 + i + 1) + *(p2 + i);  /* RAW on p1[i+1] */
        
        /* Aliasing potential */
        if (i % 2 == 0) {
            *(p2 + i) = *(p1 + i) * 3;  /* RAW on p1[i] just written */
        } else {
            *(p2 + i) = *(p2 + i - 1) + 1;  /* Loop-carried on p2 */
        }
        
        /* Output dependency through pointers */
        int* ptr = (i % 3 == 0) ? p1 : p2;
        *ptr = i;  /* Potential WAW */
    }
    sink = *p1 + *p2;
}

/* Test 5: Complex recurrence chain */
static void __attribute__((noinline, noipa))
test5_recurrence_chain(int n, int* x, int* y, int* z) {
    int a = 1, b = 2, c = 3;
    
    for (int i = 0; i < n; ++i) {
        /* Chain of dependencies within iteration */
        a = b + x[i];      /* RAW on b */
        b = c * 2;         /* RAW on c */
        c = a - i;         /* RAW on a */
        
        /* Memory dependencies with different distances */
        y[i] = a + b;
        if (i >= 3) {
            z[i] = z[i-3] + y[i-1];  /* Distance 3 and 1 */
        }
        
        /* Another loop-carried dependency */
        x[i] = (i > 0) ? x[i-1] + 1 : 0;
    }
    sink = a + b + c;
}

/* Test 6: Matrix multiplication style (2D dependencies) */
static void __attribute__((noinline, noipa))
test6_matrix_style(int n, int* A, int* B, int* C) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int sum = 0;
            for (int k = 0; k < n; ++k) {
                /* True dependencies on A and B */
                sum += A[i * n + k] * B[k * n + j];
                
                /* Anti-dependency through reuse */
                int tmp = sum;
                sum = tmp + (k % 2);
            }
            /* Output dependency on C */
            C[i * n + j] = sum;
            
            /* Loop-carried in j dimension */
            if (j > 0) {
                A[i * n + j] = C[i * n + j - 1] + 1;
            }
        }
        /* Loop-carried in i dimension */
        if (i > 0) {
            B[i * n] = A[(i-1) * n] * 2;
        }
    }
    sink = C[n*n/2];
}

/* Test 7: Mixed dependency types with volatile */
static void __attribute__((noinline, noipa))
test7_volatile_mixed(int n, volatile int* varr, int* arr) {
    int local = 0;
    
    for (int i = 1; i < n; ++i) {
        /* Volatile read creates memory barrier */
        int v = varr[i];
        
        /* Multiple dependency types */
        int t1 = arr[i-1] + v;      /* RAW on arr[i-1] */
        int t2 = t1 * local;        /* RAW on t1, WAR on local */
        local = t2 + i;             /* WAW on local */
        arr[i] = local - v;         /* WAW on arr[i] */
        
        /* Volatile write */
        varr[i-1] = local;
        
        /* Control dependency */
        if (local > 100) {
            arr[i] = arr[i] / 2;
            local = local - 50;
        }
    }
    sink = local;
}

int main(void) {
    /* Use volatile to prevent compile-time computation */
    volatile int N = get_iterations();
    int M = 50;
    
    /* Allocate arrays with volatile elements to prevent optimization */
    int* arr1 = (int*)malloc(N * sizeof(int));
    int* arr2 = (int*)malloc(N * sizeof(int));
    int* arr3 = (int*)malloc(N * sizeof(int));
    int* flags = (int*)malloc(N * sizeof(int));
    volatile int* varr = (volatile int*)malloc(N * sizeof(int));
    
    int* mat = (int*)malloc(M * M * sizeof(int));
    int* A = (int*)malloc(M * M * sizeof(int));
    int* B = (int*)malloc(M * M * sizeof(int));
    int* C = (int*)malloc(M * M * sizeof(int));
    
    /* Initialize with non-constant values */
    for (int i = 0; i < N; ++i) {
        arr1[i] = i % 7;
        arr2[i] = i % 11;
        arr3[i] = i % 13;
        flags[i] = (i % 3 == 0) ? 1 : -1;
        varr[i] = i % 5;
    }
    
    for (int i = 0; i < M * M; ++i) {
        mat[i] = i % 17;
        A[i] = i % 19;
        B[i] = i % 23;
        C[i] = i % 29;
    }
    
    /* Execute all test cases */
    test1_register_memory_deps(N, arr1, arr2, arr3);
    test2_nested_scc(M, M, mat);
    test3_conditional_deps(N, arr1, flags);
    test4_pointer_aliasing(N, arr2, arr3);
    test5_recurrence_chain(N, arr1, arr2, arr3);
    test6_matrix_style(M, A, B, C);
    test7_volatile_mixed(N, varr, arr1);
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += arr1[i] + arr2[i] + arr3[i];
    }
    for (int i = 0; i < M * M; ++i) {
        checksum += mat[i] + A[i] + B[i] + C[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(flags);
    free((void*)varr);
    free(mat);
    free(A);
    free(B);
    free(C);
    
    return 0;
}
