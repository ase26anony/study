/* Test program to trigger DDG edge creation and initialization */
#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_iterations(void) {
    volatile int iterations = 100;
    return iterations;
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
        /* RAW dependency on arr1[i-1] from previous iteration */
        int temp = arr1[i-1] + arr2[i];  // Memory dependency
        
        /* WAR dependency: temp is written, then used in acc calculation */
        acc = acc + temp;  // Register dependency with distance 1
        
        /* WAW dependency: arr3[i] written multiple times in loop */
        arr3[i] = acc;  // Output dependency
        
        /* Loop-carried true dependency through prev */
        arr1[i] = prev + get_value(i);  // Distance 1 memory dependency
        prev = arr1[i];  // Register dependency chain
        
        /* Anti-dependency: arr2[i] read after potential write in prev iteration */
        arr2[i] = arr2[i] + 1;  // Anti-dependency if arr2 aliases something
    }
    
    /* Prevent dead code elimination */
    volatile int sink = acc + arr3[n-1];
    (void)sink;
}

/* Test 2: Nested loops for SCC formation */
static void __attribute__((noinline, noipa))
test2_nested_loops_scc(int n, int m, int* mat) {
    int sum = 0;
    
    for (int i = 1; i < n; ++i) {
        int row_acc = 0;
        
        /* Inner loop with loop-carried dependency */
        for (int j = 1; j < m; ++j) {
            /* Complex memory dependencies forming potential cycles */
            int idx = i * m + j;
            int prev_idx = (i-1) * m + j;
            int left_idx = i * m + (j-1);
            
            /* Multiple inter-dependent statements creating small SCC */
            int a = mat[prev_idx] * 2;      // RAW from previous row
            int b = mat[left_idx] + a;      // RAW from left cell + a
            mat[idx] = b + row_acc;         // WAW + RAW from b and row_acc
            row_acc = mat[idx] - j;         // WAR on mat[idx], carries to next j
            
            /* Cross-iteration dependency in inner loop */
            sum = sum + row_acc;            // Register dependency across j
        }
        
        /* Outer loop carried dependency */
        mat[i * m] = sum;                   // Memory dependency across i
    }
    
    volatile int sink = sum + mat[(n-1)*m + (m-1)];
    (void)sink;
}

/* Test 3: Loop with control dependencies */
static void __attribute__((noinline, noipa))
test3_control_deps(int n, int* data, int* output) {
    int threshold = 50;
    int count = 0;
    int running_sum = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Loop-carried register dependency */
        running_sum = running_sum + data[i];
        
        /* Control dependency: branch depends on loop-variant value */
        if (running_sum > threshold) {
            /* True branch creates dependencies within SCC */
            output[i] = running_sum - count;  // RAW on running_sum, WAW on output
            count = count + 1;                // Register dependency
            threshold = threshold + output[i]; // WAR on output[i], carries to next iter
        } else {
            /* False branch also has dependencies */
            output[i] = data[i] * 2;          // RAW on data[i]
            running_sum = running_sum / 2;    // WAR on running_sum
        }
        
        /* Cross-iteration anti-dependency */
        data[i] = data[i] + i;  // WAW on data[i], anti on next iteration's read
    }
    
    volatile int sink = count + output[n-1];
    (void)sink;
}

/* Test 4: Pointer-based aliasing dependencies */
static void __attribute__((noinline, noipa))
test4_pointer_aliasing(int n, int* base, int* alt) {
    int* ptr1 = base;
    int* ptr2 = alt;
    int accum = 0;
    
    /* Create potential pointer aliasing */
    if (n > 10) {
        ptr2 = base + 5;  /* Now ptr2 aliases part of base array */
    }
    
    for (int i = 0; i < n; ++i) {
        /* Pointer arithmetic creating complex address calculations */
        int* current = ptr1 + i;
        int* other = ptr2 + (i % 8);
        
        /* Memory dependencies with potential aliasing */
        int val1 = *current;                 /* Read from base location */
        int val2 = *other;                   /* May alias with current */
        
        /* Inter-dependent statements */
        *current = val1 + accum;             /* WAW on *current, RAW on val1/accum */
        accum = val2 * 3;                    /* RAW on val2, register dependency */
        
        /* Loop-carried through accum */
        *other = accum - i;                  /* WAW on *other, may alias *current */
        
        /* Additional dependency chain */
        if (i % 3 == 0) {
            ptr1[i] = ptr1[i] + ptr2[i/2];   /* Complex indexed access */
        }
    }
    
    volatile int sink = accum + base[n-1];
    (void)sink;
}

/* Test 5: Recurrence chain within single iteration */
static void __attribute__((noinline, noipa))
test5_recurrence_chain(int n, int* vec) {
    for (int i = 0; i < n; ++i) {
        /* Multiple statements forming dependency cycle within iteration */
        int x = vec[i] + i;      /* Statement A: RAW on vec[i] */
        int y = x * 2 - 5;       /* Statement B: RAW on x */
        int z = y + x / 3;       /* Statement C: RAW on y and x */
        vec[i] = z + vec[i];     /* Statement D: RAW on z and vec[i], WAW on vec[i] */
        
        /* This creates a small SCC: A->B->C->D and D->A (through vec[i] in next iter) */
        
        /* Additional loop-carried dependency */
        if (i > 0) {
            vec[i] = vec[i] + vec[i-1];  /* RAW on vec[i-1], distance 1 */
        }
    }
    
    volatile int sink = vec[n-1];
    (void)sink;
}

/* Test 6: Matrix-style computation with 2D dependencies */
static void __attribute__((noinline, noipa))
test6_matrix_kernel(int rows, int cols, int* A, int* B, int* C) {
    /* Simplified matrix multiplication pattern */
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int sum = 0;
            
            /* Innermost loop creates complex dependency web */
            for (int k = 0; k < cols; ++k) {
                /* Memory dependencies with different strides */
                int a_idx = i * cols + k;
                int b_idx = k * cols + j;
                
                /* RAW dependencies on A and B */
                int prod = A[a_idx] * B[b_idx];
                
                /* Register dependency carried through k loop */
                sum = sum + prod;  /* Distance 1 in k dimension */
                
                /* Anti-dependency through array reuse */
                if (k % 2 == 0) {
                    B[b_idx] = B[b_idx] + 1;  /* WAW on B[b_idx] */
                }
            }
            
            /* Output dependency on C */
            C[i * cols + j] = sum;  /* WAW on C */
            
            /* Loop-carried dependency in j dimension */
            if (j > 0) {
                C[i * cols + j] = C[i * cols + j] + C[i * cols + (j-1)] / 2;
            }
        }
        
        /* Loop-carried dependency in i dimension */
        if (i > 0) {
            for (int j = 0; j < cols; ++j) {
                A[i * cols + j] = A[(i-1) * cols + j] + C[i * cols + j];
            }
        }
    }
    
    volatile int sink = C[(rows-1)*cols + (cols-1)];
    (void)sink;
}

int main(void) {
    /* Use volatile to get non-constant loop bounds */
    volatile int N = get_iterations();
    volatile int M = N / 2;
    
    /* Allocate arrays with sufficient size */
    int size = N > 100 ? N : 100;
    int* arr1 = (int*)malloc(size * sizeof(int));
    int* arr2 = (int*)malloc(size * sizeof(int));
    int* arr3 = (int*)malloc(size * sizeof(int));
    int* matrix = (int*)malloc(size * size * sizeof(int));
    int* output = (int*)malloc(size * sizeof(int));
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < size; ++i) {
        arr1[i] = get_value(i);
        arr2[i] = i * 2;
        arr3[i] = 0;
        output[i] = 0;
    }
    
    for (int i = 0; i < size * size; ++i) {
        matrix[i] = i % 100;
    }
    
    /* Execute all test cases */
    test1_loop_carried_deps(N, arr1, arr2, arr3);
    test2_nested_loops_scc(M, M, matrix);
    test3_control_deps(N, arr1, output);
    test4_pointer_aliasing(N, arr2, arr3);
    test5_recurrence_chain(N, arr1);
    
    /* For matrix test, use smaller dimensions to avoid excessive runtime */
    int small_n = N > 20 ? 20 : N;
    test6_matrix_kernel(small_n, small_n, matrix, arr2, arr3);
    
    /* Compute checksum to prevent optimization */
    volatile int checksum = 0;
    for (int i = 0; i < size; ++i) {
        checksum = checksum + arr1[i] + arr2[i] + arr3[i] + output[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(matrix);
    free(output);
    
    return 0;
}
