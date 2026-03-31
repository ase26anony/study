/* Test program to trigger DDG edge initialization in ddg.cc */
#include <stdio.h>
#include <stdlib.h>

/* Dummy opaque functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_iterations(void) {
    volatile int n = 100;
    return n;
}

static int __attribute__((noinline, noipa)) get_stride(void) {
    volatile int s = 2;
    return s;
}

static void __attribute__((noinline, noipa)) use(int val) {
    volatile int sink = val;
    (void)sink;
}

/* Test 1: Simple loop with register and memory dependencies */
static void __attribute__((noinline, noipa)) 
test1_loop_carried_deps(int n, int* arr1, int* arr2) {
    int acc = 0;
    int prev = arr1[0];
    
    /* Complex loop with multiple dependency types */
    for (int i = 1; i < n; ++i) {
        /* RAW (true) dependency: arr1[i] depends on prev from previous iteration */
        int temp = prev + arr2[i];  /* distance = 1 */
        
        /* WAR (anti) dependency: arr1[i-1] is read, then written */
        int old_val = arr1[i-1];
        arr1[i-1] = old_val * 2;    /* anti-dependency on arr1[i-1] */
        
        /* WAW (output) dependency: arr1[i] written multiple times */
        arr1[i] = temp;
        arr1[i] = arr1[i] + i;      /* output dependency on arr1[i] */
        
        /* Loop-carried register dependency */
        acc = acc + temp;           /* distance = 1, register-based */
        
        /* Control dependency */
        if (acc > 1000) {
            arr2[i] = arr2[i] / 2;  /* control-dependent store */
        }
        
        prev = temp;                /* feeds next iteration */
    }
    
    use(acc);
    use(arr1[n-1]);
}

/* Test 2: Nested loops forming SCCs */
static void __attribute__((noinline, noipa))
test2_nested_scc(int n, int m, int* mat) {
    int sum = 0;
    
    /* Outer loop with carried dependency */
    for (int i = 1; i < n; ++i) {
        int row_acc = 0;
        
        /* Inner loop with complex dependencies - forms SCC */
        for (int j = 1; j < m; ++j) {
            /* Cycle of dependencies within one iteration */
            int x = mat[i*m + j-1] + 1;      /* RAW on mat[i*m + j-1] */
            int y = x * 2;                   /* RAW on x */
            mat[i*m + j] = y + mat[(i-1)*m + j]; /* RAW on y and mat[(i-1)*m + j] */
            
            /* Anti-dependency in inner loop */
            int old = mat[i*m + j-1];
            row_acc = row_acc + old;         /* RAW on row_acc (carried) */
            mat[i*m + j-1] = old + j;        /* WAR on mat[i*m + j-1] */
        }
        
        /* Loop-carried dependency between outer iterations */
        sum = sum + row_acc;                 /* distance = 1 */
        
        /* Control flow affecting loop-carried variable */
        if (sum < 0) {
            mat[i*m] = -mat[i*m];            /* control-dependent */
        }
    }
    
    use(sum);
}

/* Test 3: Pointer-based accesses with aliasing */
static void __attribute__((noinline, noipa))
test3_pointer_aliasing(int n, int* base) {
    int* p1 = base;
    int* p2 = base + n/2;
    int* p3 = base + n/4;
    
    int val1 = 0, val2 = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Potential aliasing creates complex memory dependencies */
        *p1 = *p2 + *p3;                     /* RAW on *p2, *p3 */
        
        /* Pointer arithmetic creates varying access patterns */
        int temp = *p1;
        *p2 = temp + i;                      /* WAR on *p2 */
        
        /* Chain of dependencies */
        val1 = val1 + temp;                  /* register carried */
        val2 = val1 * 2;                     /* RAW on val1 (within iteration) */
        *p3 = val2;                          /* WAW on *p3 */
        
        /* Update pointers - creates control-like dependencies */
        if (i % 3 == 0) {
            p1++;
        } else if (i % 3 == 1) {
            p2++;
        } else {
            p3++;
        }
    }
    
    use(val1);
    use(val2);
}

/* Test 4: Matrix multiplication style - 2D array with complex deps */
static void __attribute__((noinline, noipa))
test4_matrix_style(int n, int* A, int* B, int* C) {
    /* Simple matrix multiply pattern with dependencies */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int sum = 0;
            for (int k = 0; k < n; ++k) {
                /* True dependencies through accumulation */
                sum = sum + A[i*n + k] * B[k*n + j]; /* RAW on sum */
                
                /* Anti-dependency through array reuse */
                int old_A = A[i*n + k];
                A[i*n + k] = old_A + 1;              /* WAR on A */
            }
            C[i*n + j] = sum;                        /* output dependency */
            
            /* Loop-carried in j dimension */
            if (j > 0) {
                C[i*n + j] = C[i*n + j] + C[i*n + j-1]; /* RAW on C with distance 1 */
            }
        }
        
        /* Loop-carried in i dimension with control */
        if (i > 0) {
            int prev_row_sum = 0;
            for (int k = 0; k < n; ++k) {
                prev_row_sum += C[(i-1)*n + k];      /* distance = 1 in i */
            }
            C[i*n] = C[i*n] + prev_row_sum;          /* control and data dep */
        }
    }
}

/* Test 5: Recurrence chain within single iteration */
static void __attribute__((noinline, noipa))
test5_recurrence_chain(int n, int* data) {
    int x = 1, y = 2, z = 3;
    
    for (int i = 0; i < n; ++i) {
        /* Cycle of dependencies within one iteration */
        x = y + data[i];     /* RAW on y */
        y = z * 2;           /* RAW on z */
        z = x - i;           /* RAW on x - completes cycle */
        
        /* Memory dependency chain */
        data[i] = x + y + z; /* WAW on data[i] */
        
        /* Loop-carried with different distances */
        if (i >= 2) {
            data[i] = data[i] + data[i-2];  /* distance = 2 */
        }
        
        /* Control affecting recurrence variables */
        if (z > 100) {
            x = x / 2;       /* control-dependent update */
        }
    }
    
    use(x + y + z);
}

/* Test 6: Complex stride patterns */
static void __attribute__((noinline, noipa))
test6_strided_access(int n, int* arr, int stride) {
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple accumulators with different update patterns */
        acc1 = acc1 + arr[i];                    /* distance = 1 */
        acc2 = acc2 + arr[i * stride % n];       /* non-unit stride */
        acc3 = acc3 + arr[(i + 1) % n];          /* wrap-around */
        
        /* Cross-iteration dependencies with stride */
        if (i >= stride) {
            arr[i] = arr[i] + arr[i - stride];   /* distance = stride */
        }
        
        /* Output dependency with anti-dependency */
        int temp = arr[i];
        arr[i] = acc1 + acc2;                    /* WAW on arr[i] */
        acc1 = temp + acc3;                      /* WAR on acc1 */
        
        /* Control creating merge point */
        if (acc1 > acc2) {
            acc3 = acc1 - acc2;                  /* control flow */
        } else {
            acc3 = acc2 - acc1;
        }
    }
    
    use(acc1 + acc2 + acc3);
}

int main(void) {
    /* Use volatile to prevent compile-time determination */
    volatile int N = get_iterations();
    volatile int M = get_stride();
    
    /* Allocate arrays with sufficient size */
    int size = 200;
    int* arr1 = (int*)malloc(size * sizeof(int));
    int* arr2 = (int*)malloc(size * sizeof(int));
    int* mat = (int*)malloc(size * size * sizeof(int));
    int* base = (int*)malloc(size * sizeof(int));
    int* A = (int*)malloc(size * size * sizeof(int));
    int* B = (int*)malloc(size * size * sizeof(int));
    int* C = (int*)malloc(size * size * sizeof(int));
    int* data = (int*)malloc(size * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < size; i++) {
        arr1[i] = i % 10 + 1;
        arr2[i] = i % 7 + 1;
        base[i] = i % 5 + 1;
        data[i] = i % 8 + 1;
    }
    
    for (int i = 0; i < size * size; i++) {
        mat[i] = i % 9 + 1;
        A[i] = i % 6 + 1;
        B[i] = i % 5 + 1;
        C[i] = i % 7 + 1;
    }
    
    /* Execute all test cases */
    test1_loop_carried_deps(N, arr1, arr2);
    test2_nested_scc(N/2, M*2, mat);
    test3_pointer_aliasing(N, base);
    test4_matrix_style(10, A, B, C);  /* Smaller size for matrix */
    test5_recurrence_chain(N, data);
    test6_strided_access(N, arr1, M);
    
    /* Compute checksum to prevent dead code elimination */
    volatile int checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += arr1[i] + arr2[i] + base[i] + data[i];
    }
    
    for (int i = 0; i < size * size; i++) {
        checksum += mat[i] + A[i] + B[i] + C[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(mat);
    free(base);
    free(A);
    free(B);
    free(C);
    free(data);
    
    return 0;
}
