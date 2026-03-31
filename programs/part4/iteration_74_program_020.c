/* Test program to trigger DDG edge creation and initialization */
#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_iterations(void) {
    volatile int n = 100;
    return n;
}

static int __attribute__((noinline, noipa)) get_value(int i) {
    volatile int v = i * 3 + 7;
    return v;
}

/* Test 1: Simple loop with register and memory dependencies */
static void __attribute__((noinline, noipa)) 
test1_loop_carried_deps(int n, int* arr1, int* arr2, int* arr3) {
    int acc = 0;
    int prev = arr1[0];
    
    for (int i = 1; i < n; ++i) {
        /* True dependency (RAW) on arr1[i-1] from previous iteration */
        int temp = arr1[i-1] + get_value(i);
        
        /* Anti dependency (WAR) - arr2 is read then written */
        int read_val = arr2[i];
        arr2[i] = temp * 2;
        
        /* Output dependency (WAW) - arr3 written multiple times */
        arr3[i] = read_val + acc;
        arr3[i] = arr3[i] * 3;  // Second write to same location
        
        /* Loop-carried register dependency */
        acc = acc + temp + read_val;
        
        /* Memory-based loop-carried dependency */
        arr1[i] = prev + arr2[i];
        prev = temp;
    }
    
    /* Volatile sink to prevent elimination */
    volatile int sink = acc + arr1[n-1] + arr2[n-1];
    (void)sink;
}

/* Test 2: Nested loops for SCC formation */
static void __attribute__((noinline, noipa))
test2_nested_loops_scc(int n, int m, int* mat) {
    for (int i = 1; i < n; ++i) {
        int row_acc = 0;
        
        /* Inner loop with loop-carried dependency forming SCC */
        for (int j = 1; j < m; ++j) {
            /* True dependency within inner loop */
            int val = mat[i*m + j-1] + mat[(i-1)*m + j];
            
            /* Anti dependency */
            int old = mat[i*m + j];
            mat[i*m + j] = val * 2;
            
            /* Output dependency */
            row_acc = row_acc + old;
            row_acc = row_acc * 3;  // WAW on row_acc
            
            /* Cross-iteration dependency in inner loop */
            mat[i*m + j] = mat[i*m + j] + row_acc;
        }
        
        /* Loop-carried dependency across outer loop */
        mat[i*m] = mat[(i-1)*m] + row_acc;
    }
    
    volatile int sink = mat[n*m - 1];
    (void)sink;
}

/* Test 3: Loop with control dependencies */
static void __attribute__((noinline, noipa))
test3_control_deps(int n, int* data, int* output) {
    int threshold = get_value(n / 2);
    int state = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Loop-carried dependency on state */
        int input = data[i] + state;
        
        /* Control dependency - branch depends on loop-variant value */
        if (input > threshold) {
            state = state * 2 + 1;
            output[i] = input - threshold;
        } else {
            state = state / 2 - 1;
            output[i] = input + threshold;
        }
        
        /* Additional true dependency chain */
        int temp = output[i];
        output[i] = temp * temp % 100;
        
        /* Anti dependency through data array */
        data[i] = data[i] + (i % 10);
    }
    
    volatile int sink = state + output[n-1];
    (void)sink;
}

/* Test 4: Complex recurrence chain within iteration */
static void __attribute__((noinline, noipa))
test4_recurrence_chain(int n, int* a, int* b, int* c) {
    for (int i = 1; i < n; ++i) {
        /* Chain of dependencies within single iteration (potential cycle in DDG) */
        int x = a[i-1] + b[i];
        int y = x * 2 - c[i];
        int z = y + a[i];
        a[i] = z / 3;
        c[i] = x + y;
        b[i] = b[i-1] + z;
        
        /* Additional output dependency */
        int temp = a[i];
        a[i] = temp + i;
    }
    
    volatile int sink = a[n-1] + b[n-1] + c[n-1];
    (void)sink;
}

/* Test 5: Pointer arithmetic with aliasing */
static void __attribute__((noinline, noipa))
test5_pointer_aliasing(int n, int* base) {
    int* ptr1 = base;
    int* ptr2 = base + n/2;
    int acc = 0;
    
    for (int i = 0; i < n/2; ++i) {
        /* Potential aliasing between ptr1 and ptr2 accesses */
        int val1 = *ptr1;
        int val2 = *(ptr2 - i);
        
        /* True dependency through pointer */
        *ptr1 = val1 + val2 + acc;
        
        /* Anti dependency */
        int old = *(ptr1 + 1);
        *(ptr1 + 1) = *ptr1 * 2;
        
        /* Loop-carried through accumulator and pointer */
        acc = acc + old + val2;
        ptr1++;
        
        /* Output dependency on acc */
        acc = acc % 1000;
    }
    
    volatile int sink = acc + *base + *(base + n/2 - 1);
    (void)sink;
}

/* Test 6: Matrix multiplication style (2D array with complex deps) */
static void __attribute__((noinline, noipa))
test6_matrix_style(int n, int* A, int* B, int* C) {
    /* Simplified matrix-style computation */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int sum = 0;
            for (int k = 0; k < n; ++k) {
                /* True dependencies on A and B */
                int a_val = A[i*n + k];
                int b_val = B[k*n + j];
                
                /* Loop-carried in k-loop */
                sum = sum + a_val * b_val;
                
                /* Anti dependency through B */
                B[k*n + j] = B[k*n + j] + (k % 7);
            }
            /* Output dependency on C */
            C[i*n + j] = sum;
            C[i*n + j] = C[i*n + j] % 100;  // Second write
        }
        
        /* Loop-carried dependency in i-loop through A */
        if (i > 0) {
            A[i*n] = A[(i-1)*n] + C[i*n];
        }
    }
    
    volatile int sink = C[n*n - 1];
    (void)sink;
}

int main(void) {
    /* Use volatile to get unknown-at-compile-time sizes */
    volatile int base_n = 50;
    int n = base_n;
    int m = n / 2;
    
    /* Allocate arrays with dynamic sizes */
    int* arr1 = (int*)malloc(n * sizeof(int));
    int* arr2 = (int*)malloc(n * sizeof(int));
    int* arr3 = (int*)malloc(n * sizeof(int));
    int* mat = (int*)malloc(n * m * sizeof(int));
    int* data = (int*)malloc(n * sizeof(int));
    int* output = (int*)malloc(n * sizeof(int));
    int* a = (int*)malloc(n * sizeof(int));
    int* b = (int*)malloc(n * sizeof(int));
    int* c = (int*)malloc(n * sizeof(int));
    int* base = (int*)malloc(n * sizeof(int));
    int* A = (int*)malloc(n * n * sizeof(int));
    int* B = (int*)malloc(n * n * sizeof(int));
    int* C = (int*)malloc(n * n * sizeof(int));
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < n; ++i) {
        arr1[i] = get_value(i);
        arr2[i] = i * 2;
        arr3[i] = i * 3;
        data[i] = i * 5;
        output[i] = 0;
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
        base[i] = i * 4;
    }
    
    for (int i = 0; i < n * m; ++i) {
        mat[i] = i;
    }
    
    for (int i = 0; i < n * n; ++i) {
        A[i] = i % 10;
        B[i] = (i + 3) % 10;
        C[i] = 0;
    }
    
    /* Execute all test cases */
    test1_loop_carried_deps(n, arr1, arr2, arr3);
    test2_nested_loops_scc(n, m, mat);
    test3_control_deps(n, data, output);
    test4_recurrence_chain(n, a, b, c);
    test5_pointer_aliasing(n, base);
    test6_matrix_style(m, A, B, C);  // Use m for smaller matrix
    
    /* Aggregate results into volatile sink */
    volatile int final_sink = 
        arr1[n-1] + arr2[n-1] + arr3[n-1] +
        mat[n*m-1] + output[n-1] + 
        a[n-1] + b[n-1] + c[n-1] +
        base[n-1] + C[m*m-1];
    
    printf("Result checksum: %d\n", final_sink);
    
    /* Cleanup */
    free(arr1); free(arr2); free(arr3);
    free(mat); free(data); free(output);
    free(a); free(b); free(c);
    free(base); free(A); free(B); free(C);
    
    return 0;
}
