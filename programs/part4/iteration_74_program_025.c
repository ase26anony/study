#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
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
    
    for (int i = 1; i < n; ++i) {
        /* True dependency (RAW) on arr1[i-1] */
        int temp = arr1[i-1] + i;
        
        /* Anti dependency (WAR) - reusing acc */
        acc = acc + temp;
        
        /* Output dependency (WAW) - multiple writes to arr2[i] */
        arr2[i] = acc;
        arr2[i] = arr2[i] * 2;  // WAW on arr2[i]
        
        /* Loop-carried true dependency with distance 1 */
        arr1[i] = prev + arr2[i];
        prev = arr1[i];
        
        /* Memory dependency with distance 2 */
        if (i >= 2) {
            arr2[i] += arr1[i-2];
        }
    }
    
    use(acc + arr1[n-1] + arr2[n-1]);
}

/* Test 2: Nested loops forming SCCs */
static void __attribute__((noinline, noipa))
test2_nested_scc(int n, int m, int* mat) {
    int sum = 0;
    
    for (int i = 1; i < n; ++i) {
        int row_acc = 0;
        
        /* Inner loop with loop-carried dependency */
        for (int j = 1; j < m; ++j) {
            /* True dependency within inner loop */
            int val = mat[(i-1)*m + j] + mat[i*m + (j-1)];
            
            /* Anti dependency on row_acc */
            row_acc = row_acc + val;
            
            /* Output dependency */
            mat[i*m + j] = row_acc;
            mat[i*m + j] = mat[i*m + j] - j;  // WAW
            
            /* Cross-iteration dependency in inner loop */
            if (j > 1) {
                mat[i*m + j] += mat[i*m + (j-2)];
            }
        }
        
        /* Loop-carried dependency between outer iterations */
        sum += row_acc;
        if (i > 1) {
            mat[i*m + 0] = sum + mat[(i-2)*m + 0];
        }
    }
    
    use(sum + mat[(n-1)*m + (m-1)]);
}

/* Test 3: Conditional dependencies and control flow */
static void __attribute__((noinline, noipa))
test3_conditional_deps(int n, int* data, int* out) {
    int threshold = 50;
    int state = 0;
    int counter = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Control dependency based on loop-variant value */
        if (data[i] > threshold) {
            /* True dependency chain within conditional */
            state = state + data[i];
            out[i] = state;
            
            /* Anti dependency */
            counter = counter + 1;
            out[i] = out[i] * counter;  // WAW
        } else {
            /* Different dependency chain */
            state = state - data[i];
            out[i] = state / 2;
            
            /* Output dependency across paths */
            counter = counter - 1;
            out[i] = out[i] + counter;  // WAW
        }
        
        /* Loop-carried dependency through state */
        data[i] = state % 100;
        
        /* Recurrence chain within iteration (potential SCC) */
        int x = state + 1;
        int y = x * 2;
        state = y - 1;
    }
    
    use(state + counter + out[n-1]);
}

/* Test 4: Pointer arithmetic and indirect accesses */
static void __attribute__((noinline, noipa))
test4_pointer_aliasing(int n, int* base) {
    int* ptr1 = base;
    int* ptr2 = base + n/2;
    int acc1 = 0, acc2 = 0;
    
    for (int i = 0; i < n/2; ++i) {
        /* Aliased memory accesses */
        *ptr1 = *ptr2 + i;          /* RAW on *ptr2 */
        acc1 = acc1 + *ptr1;        /* RAW on *ptr1 */
        
        /* Anti dependency through pointers */
        *ptr2 = acc1 - *ptr1;       /* WAR on *ptr1, RAW on acc1 */
        acc2 = acc2 + *ptr2;        /* RAW on *ptr2 */
        
        /* Pointer movement creates changing dependencies */
        ptr1++;
        ptr2--;
        
        /* Loop-carried through accumulators */
        if (i > 0) {
            base[i] = acc1 + base[i-1];
        }
    }
    
    use(acc1 + acc2 + base[n/4]);
}

/* Test 5: Complex recurrence with multiple distances */
static void __attribute__((noinline, noipa))
test5_multi_distance(int n, int* seq) {
    int a = seq[0], b = seq[1], c = seq[2];
    
    for (int i = 3; i < n; ++i) {
        /* Distance 1 dependency */
        int t1 = a + i;
        
        /* Distance 2 dependency */
        int t2 = b + t1;
        
        /* Distance 3 dependency */
        int t3 = c + t2;
        
        /* Update chain with different distances */
        a = b + seq[i-1];      /* distance 1 on b, memory distance 1 */
        b = c + seq[i-2];      /* distance 1 on c, memory distance 2 */
        c = t3 + seq[i-3];     /* distance 1 on t3, memory distance 3 */
        
        /* Multiple writes to same memory */
        seq[i] = a + b;
        seq[i] = seq[i] * c;   /* WAW */
        
        /* Cross-iteration memory dependency */
        if (i % 4 == 0) {
            seq[i] += seq[i-4];
        }
    }
    
    use(a + b + c + seq[n-1]);
}

/* Test 6: Matrix multiplication style dependencies */
static void __attribute__((noinline, noipa))
test6_matrix_style(int n, int* A, int* B, int* C) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int sum = 0;
            
            for (int k = 0; k < n; ++k) {
                /* True dependencies on A and B */
                int prod = A[i*n + k] * B[k*n + j];
                
                /* Anti dependency on sum */
                sum = sum + prod;
                
                /* Cross-iteration dependency in k-loop */
                if (k > 0) {
                    sum += A[i*n + k-1];
                }
            }
            
            /* Output dependency */
            C[i*n + j] = sum;
            C[i*n + j] = C[i*n + j] + i + j;  /* WAW */
            
            /* Loop-carried dependency in j-loop */
            if (j > 0) {
                B[i*n + j] = C[i*n + j] + C[i*n + j-1];
            }
        }
        
        /* Loop-carried dependency in i-loop */
        if (i > 0) {
            A[i*n + 0] = C[i*n + 0] + C[(i-1)*n + 0];
        }
    }
    
    use(C[(n-1)*n + (n-1)]);
}

int main(void) {
    /* Use volatile to prevent compile-time computation */
    volatile int N = get_iterations();
    volatile int M = get_stride();
    
    int n = N;
    int m = M;
    
    /* Allocate arrays with dynamic sizes */
    int* arr1 = (int*)malloc(n * sizeof(int));
    int* arr2 = (int*)malloc(n * sizeof(int));
    int* mat = (int*)malloc(n * m * sizeof(int));
    int* data = (int*)malloc(n * sizeof(int));
    int* out = (int*)malloc(n * sizeof(int));
    int* seq = (int*)malloc(n * sizeof(int));
    int* A = (int*)malloc(n * n * sizeof(int));
    int* B = (int*)malloc(n * n * sizeof(int));
    int* C = (int*)malloc(n * n * sizeof(int));
    
    /* Initialize with non-constant values */
    for (int i = 0; i < n; ++i) {
        arr1[i] = i * 3 % 100;
        arr2[i] = i * 7 % 100;
        data[i] = i * 11 % 100;
        out[i] = 0;
        seq[i] = i * 5 % 100;
    }
    
    for (int i = 0; i < n * m; ++i) {
        mat[i] = i % 100;
    }
    
    for (int i = 0; i < n * n; ++i) {
        A[i] = i % 50;
        B[i] = (i * 2) % 50;
        C[i] = 0;
    }
    
    /* Execute all test cases */
    test1_loop_carried_deps(n, arr1, arr2);
    test2_nested_scc(n, m, mat);
    test3_conditional_deps(n, data, out);
    test4_pointer_aliasing(n, arr1);  /* Reuse arr1 for aliasing test */
    test5_multi_distance(n, seq);
    test6_matrix_style(m, A, B, C);   /* Use m for smaller matrix */
    
    /* Compute checksum to prevent dead code elimination */
    volatile int checksum = 0;
    for (int i = 0; i < n; ++i) {
        checksum += arr1[i] + arr2[i] + data[i] + out[i] + seq[i];
    }
    
    for (int i = 0; i < n * m; ++i) {
        checksum += mat[i];
    }
    
    for (int i = 0; i < m * m; ++i) {
        checksum += A[i] + B[i] + C[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(mat);
    free(data);
    free(out);
    free(seq);
    free(A);
    free(B);
    free(C);
    
    return 0;
}
