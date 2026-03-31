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
test1_register_memory_deps(int n, int* arr1, int* arr2) {
    int acc = 0;
    int prev = arr1[0];
    
    for (int i = 1; i < n; ++i) {
        /* RAW dependency on arr1[i] from previous iteration */
        int temp = arr1[i] + prev;           /* Statement A */
        
        /* WAR dependency: temp is read, then written */
        temp = temp * 2;                     /* Statement B */
        
        /* WAW dependency on arr2[i] */
        arr2[i] = temp;                      /* Statement C */
        arr2[i] = arr2[i] + arr2[i-1];       /* Statement D - WAW on arr2[i] */
        
        /* Loop-carried register dependency */
        acc = acc + temp;                    /* Statement E - distance 1 */
        
        /* Loop-carried memory dependency */
        prev = arr1[i];                      /* Statement F - distance 1 */
    }
    
    sink(acc + arr2[n-1]);
}

/* Test 2: Complex loop with control dependencies */
static void __attribute__((noinline, noipa))
test2_control_deps(int n, int* a, int* b, int* c) {
    int sum = 0;
    int threshold = 50;
    
    for (int i = 0; i < n; ++i) {
        /* Memory dependency chain */
        int val = a[i] + (i > 0 ? a[i-1] : 0);  /* RAW on a[i], a[i-1] */
        
        /* Control dependency based on loop-variant value */
        if (val > threshold) {                   /* Control dep on val */
            b[i] = val * 2;                      /* Statement G */
            sum += b[i];                         /* Statement H - RAW on b[i] */
        } else {
            b[i] = val / 2;                      /* Statement I */
            sum -= b[i];                         /* Statement J - RAW on b[i] */
        }
        
        /* Output dependency with pointer aliasing possibility */
        c[i] = sum;                              /* Statement K */
        
        /* Anti-dependency (WAR) */
        val = c[i] + i;                          /* Statement L - WAR on val */
        
        /* Another loop-carried dependency */
        threshold = (threshold + val) % 100;      /* Statement M - distance 1 */
    }
    
    sink(sum + c[n-1]);
}

/* Test 3: Nested loops for SCC formation */
static void __attribute__((noinline, noipa))
test3_nested_scc(int n, int m, int* mat) {
    volatile int sink_sum = 0;
    
    for (int i = 1; i < n; ++i) {
        int row_acc = 0;
        
        /* Inner loop with loop-carried dependency */
        for (int j = 1; j < m; ++j) {
            /* Complex memory access pattern */
            int idx = i * m + j;
            
            /* Multiple dependencies within single iteration (potential cycle) */
            int x = mat[idx] + mat[idx - 1];      /* Statement N */
            int y = x * 2;                        /* Statement O - RAW on x */
            mat[idx] = y + mat[idx - m];          /* Statement P - RAW on y, WAW on mat[idx] */
            
            /* Recurrence chain within iteration */
            row_acc = row_acc + mat[idx];         /* Statement Q - loop-carried */
            
            /* Cross-iteration memory dependency */
            mat[idx] = mat[idx] + row_acc;        /* Statement R - WAW on mat[idx] */
        }
        
        sink_sum += row_acc;
    }
    
    sink(sink_sum);
}

/* Test 4: Pointer arithmetic and indirect accesses */
static void __attribute__((noinline, noipa))
test4_pointer_aliasing(int n, int* base, int* results) {
    int* ptr1 = base;
    int* ptr2 = base + n/2;
    
    int accum = 0;
    
    for (int i = 0; i < n/2; ++i) {
        /* Pointer-based memory accesses with potential aliasing */
        int val1 = *ptr1;                         /* Statement S */
        int val2 = *ptr2;                         /* Statement T */
        
        /* Complex dependency chain */
        *ptr1 = val1 + val2;                      /* Statement U - WAW on *ptr1 */
        accum = accum + *ptr1;                    /* Statement V - RAW on *ptr1 */
        
        /* Anti-dependency */
        val1 = accum + i;                         /* Statement W - WAR on val1 */
        
        /* Output dependency with pointer increment */
        *ptr2 = val1 * 2;                         /* Statement X - WAW on *ptr2 */
        
        /* Loop-carried pointer movement */
        ptr1++;                                   /* Statement Y */
        ptr2--;                                   /* Statement Z - distance 1 in opposite direction */
        
        /* Cross-iteration dependency through accum */
        results[i] = accum;                       /* Statement AA */
    }
    
    sink(accum + results[n/2 - 1]);
}

/* Test 5: Matrix multiplication kernel with complex dependencies */
static void __attribute__((noinline, noipa))
test5_matrix_mult(int n, int* A, int* B, int* C) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int sum = 0;
            
            for (int k = 0; k < n; ++k) {
                /* Multiple memory dependencies */
                int a_val = A[i * n + k];         /* Statement BB */
                int b_val = B[k * n + j];         /* Statement CC */
                
                /* Loop-carried dependency in innermost loop */
                sum = sum + a_val * b_val;        /* Statement DD - distance 1 */
                
                /* Cross-iteration anti-dependency */
                a_val = sum % 256;                /* Statement EE - WAR on a_val */
            }
            
            /* Output dependency with 2D access pattern */
            C[i * n + j] = sum;                   /* Statement FF */
            
            /* Cross-row dependency */
            if (i > 0) {
                C[i * n + j] += C[(i-1) * n + j]; /* Statement GG - RAW on C */
            }
        }
    }
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < n * n; ++i) {
        checksum += C[i];
    }
    sink(checksum);
}

/* Test 6: Recurrence with multiple dependency types */
static void __attribute__((noinline, noipa))
test6_mixed_recurrence(int n, int* data, int* out) {
    int x = data[0];
    int y = data[1];
    int z = 0;
    
    for (int i = 2; i < n; ++i) {
        /* Cycle of dependencies within iteration */
        x = y + data[i];          /* Statement HH - RAW on y */
        y = x * 2;                /* Statement II - RAW on x */
        z = z + y;                /* Statement JJ - RAW on y, loop-carried on z */
        
        /* Memory anti-dependency */
        int temp = data[i];       /* Statement KK */
        data[i] = z;              /* Statement LL - WAW on data[i] */
        
        /* Control flow creating complex DDG */
        if (z > 100) {            /* Statement MM - control dep on z */
            out[i] = x;
        } else {
            out[i] = y;
        }
        
        /* Another loop-carried dependency */
        z = z % 1000;             /* Statement NN - WAW on z, distance 1 */
    }
    
    sink(x + y + z + out[n-1]);
}

int main(void) {
    /* Use volatile to get non-constant loop bounds */
    volatile int base_n = get_iterations();
    int n = base_n;
    
    /* Allocate arrays with sufficient size */
    int size = n + 10;
    int* arr1 = (int*)malloc(size * sizeof(int));
    int* arr2 = (int*)malloc(size * sizeof(int));
    int* arr3 = (int*)malloc(size * sizeof(int));
    int* mat = (int*)malloc(size * size * sizeof(int));
    int* results = (int*)malloc(size * sizeof(int));
    
    /* Initialize with non-constant values */
    for (int i = 0; i < size; ++i) {
        arr1[i] = (i * 3) % 97;
        arr2[i] = (i * 7) % 101;
        arr3[i] = (i * 11) % 103;
        results[i] = 0;
    }
    
    for (int i = 0; i < size * size; ++i) {
        mat[i] = (i * 13) % 107;
    }
    
    /* Execute all test cases to trigger various DDG edge creations */
    test1_register_memory_deps(n, arr1, arr2);
    test2_control_deps(n, arr1, arr2, arr3);
    test3_nested_scc(10, 10, mat);
    test4_pointer_aliasing(n, arr1, results);
    
    /* Smaller matrix for multiplication test */
    int small_n = 8;
    int* A = (int*)malloc(small_n * small_n * sizeof(int));
    int* B = (int*)malloc(small_n * small_n * sizeof(int));
    int* C = (int*)malloc(small_n * small_n * sizeof(int));
    
    for (int i = 0; i < small_n * small_n; ++i) {
        A[i] = (i * 17) % 53;
        B[i] = (i * 19) % 59;
        C[i] = 0;
    }
    
    test5_matrix_mult(small_n, A, B, C);
    test6_mixed_recurrence(n, arr1, results);
    
    /* Final sink to prevent dead code elimination */
    volatile int final_sink = 0;
    for (int i = 0; i < n; ++i) {
        final_sink += arr1[i] + arr2[i] + arr3[i] + results[i];
    }
    
    printf("Checksum: %d\n", final_sink);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(mat);
    free(results);
    free(A);
    free(B);
    free(C);
    
    return 0;
}
