/* Test program to trigger DDG edge creation and initialization */
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
    
    /* Loop with multiple dependency types */
    for (int i = 1; i < n; ++i) {
        /* RAW (true) dependency: arr1[i-1] -> arr1[i] */
        int temp = arr1[i-1] + i;
        
        /* WAR (anti) dependency: arr1[i] read, then written */
        int read_val = arr1[i];
        arr1[i] = temp + read_val;
        
        /* WAW (output) dependency: arr2[i] written twice */
        arr2[i] = read_val * 2;
        arr2[i] = arr2[i] + acc;
        
        /* Loop-carried register dependency */
        acc = acc + arr1[i];
        
        /* Loop-carried memory dependency with distance 2 */
        if (i >= 2) {
            arr2[i] += arr2[i-2];
        }
        
        /* Control dependency */
        if (arr1[i] > 100) {
            prev = arr1[i];
        } else {
            prev = temp;
        }
    }
    
    sink(acc + prev);
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
            /* Recurrence chain within iteration - creates cycle */
            int x = mat[i*m + j-1];
            int y = x * 2 + row_acc;
            mat[i*m + j] = y;
            row_acc = y + mat[(i-1)*m + j];
            
            /* Cross-iteration dependency in inner loop */
            if (j >= 2) {
                mat[i*m + j] += mat[i*m + j-2];
            }
        }
        
        /* Loop-carried dependency in outer loop */
        sum += row_acc;
        
        /* Anti-dependency between iterations */
        int temp = mat[i*m];
        mat[i*m] = sum;
        sum = temp + sum;
    }
    
    sink(sum);
}

/* Test 3: Complex dependencies with conditionals */
static void __attribute__((noinline, noipa))
test3_conditional_deps(int n, int* a, int* b, int* c) {
    int x = 0, y = 0, z = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Multiple inter-statement dependencies within iteration */
        x = a[i] + y;      /* RAW: y -> x */
        y = b[i] * x;      /* RAW: x -> y, WAR: y */
        z = c[i] - x;      /* RAW: x -> z */
        
        /* Output dependency */
        a[i] = x + y + z;
        
        /* Loop-carried with distance 3 */
        if (i >= 3) {
            b[i] += b[i-3];
        }
        
        /* Control flow creating control dependencies */
        volatile int cond = a[i];
        if (cond > 0) {
            x = y + 1;
        } else {
            x = z - 1;
        }
        
        /* Another recurrence */
        z = x * 2;
    }
    
    sink(x + y + z);
}

/* Test 4: Pointer-based aliasing dependencies */
static void __attribute__((noinline, noipa))
test4_pointer_aliasing(int n, int* base) {
    int* p1 = base;
    int* p2 = base + n/2;
    int* p3 = base + n/4;
    
    int acc1 = 0, acc2 = 0;
    
    for (int i = 0; i < n/2; ++i) {
        /* Potential aliasing creates complex memory dependencies */
        *p1 = *p2 + acc1;
        *p2 = *p3 * 2;
        *p3 = *p1 - acc2;
        
        /* Register dependencies */
        acc1 = acc1 + *p1;
        acc2 = acc2 + *p2 + *p3;
        
        /* Pointer arithmetic - may create unknown dependencies */
        p1++;
        p2--;
        p3 += (i % 2) ? 1 : -1;
        
        /* Loop-carried through pointers */
        if (i > 0) {
            *(p1-1) += acc1;
        }
    }
    
    sink(acc1 + acc2);
}

/* Test 5: Reduction with multiple accumulators */
static void __attribute__((noinline, noipa))
test5_multiple_reductions(int n, int* data) {
    int sum1 = 0, sum2 = 0, sum3 = 0;
    int prod1 = 1, prod2 = 1;
    
    for (int i = 0; i < n; ++i) {
        /* Independent reductions */
        sum1 += data[i];
        sum2 += data[i] * i;
        sum3 += data[i] + i;
        
        /* Product reductions with anti-dependencies */
        int old_prod1 = prod1;
        prod1 = prod1 * (data[i] + 1);
        prod2 = prod2 * (old_prod1 % 256);
        
        /* Cross-iteration dependency between different variables */
        if (i > 0) {
            data[i] += sum1 - sum2;
        }
        
        /* Conditional update creating control flow */
        if (prod1 > 1000000) {
            prod1 = prod1 / 2;
            sum1 = sum1 - data[i];
        }
    }
    
    sink(sum1 + sum2 + sum3 + prod1 + prod2);
}

/* Test 6: Matrix-style computation */
static void __attribute__((noinline, noipa))
test6_matrix_kernel(int n, int* A, int* B, int* C) {
    /* Simple matrix add with dependencies */
    for (int i = 1; i < n; ++i) {
        for (int j = 1; j < n; ++j) {
            /* 2D stencil-like dependencies */
            int idx = i*n + j;
            int idx_up = (i-1)*n + j;
            int idx_left = i*n + (j-1);
            
            /* Multiple memory dependencies */
            C[idx] = A[idx] + B[idx];
            C[idx] += C[idx_up] + C[idx_left];
            
            /* Cross-row dependency */
            if (i >= 2) {
                C[idx] += A[(i-2)*n + j];
            }
            
            /* Register accumulation */
            B[idx] = B[idx] * 2 - C[idx];
        }
    }
    
    /* Final reduction */
    int total = 0;
    for (int i = 0; i < n*n; ++i) {
        total += C[i];
    }
    sink(total);
}

int main(void) {
    /* Use volatile to get unknown but bounded iteration counts */
    volatile int N = get_iterations();
    int n = N;
    
    /* Allocate arrays with volatile initialization */
    int* arr1 = (int*)malloc(n * sizeof(int));
    int* arr2 = (int*)malloc(n * sizeof(int));
    int* arr3 = (int*)malloc(n * sizeof(int));
    int* matrix = (int*)malloc(n * n * sizeof(int));
    
    /* Initialize with non-constant values */
    for (int i = 0; i < n; ++i) {
        arr1[i] = (i * 7) % 19;
        arr2[i] = (i * 13) % 23;
        arr3[i] = (i * 17) % 29;
    }
    
    for (int i = 0; i < n*n; ++i) {
        matrix[i] = (i * 11) % 31;
    }
    
    /* Run all test cases */
    test1_loop_carried_deps(n, arr1, arr2);
    test2_nested_loops_scc(n, n/2, matrix);
    test3_conditional_deps(n, arr1, arr2, arr3);
    test4_pointer_aliasing(n, arr1);
    test5_multiple_reductions(n, arr3);
    test6_matrix_kernel(n/2, arr1, arr2, matrix);
    
    /* Compute and print a checksum */
    int checksum = 0;
    for (int i = 0; i < n; ++i) {
        checksum += arr1[i] + arr2[i] + arr3[i];
    }
    for (int i = 0; i < n*n/4; ++i) {
        checksum += matrix[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(arr1);
    free(arr2);
    free(arr3);
    free(matrix);
    
    return 0;
}
