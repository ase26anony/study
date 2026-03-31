/* ddg_test.c - Test program for DDG edge initialization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent optimization */
static int __attribute__((noinline, noipa)) get_iterations(void) {
    volatile int iterations = 100;
    return iterations;
}

/* Dummy volatile sink to prevent dead code elimination */
static volatile int sink = 0;

/* Test 1: Simple loop with register and memory dependencies */
static void __attribute__((noinline, noipa)) 
test1_register_memory_deps(int n, int* arr1, int* arr2) {
    int acc = 0;
    int prev = arr1[0];
    
    /* Loop with RAW, WAR, and WAW dependencies */
    for (int i = 1; i < n; ++i) {
        /* RAW: Read arr1[i] before writing arr2[i] */
        int temp = arr1[i] + prev;  /* Uses prev from previous iteration (distance 1) */
        
        /* WAR: Read arr2[i-1] before overwriting arr2[i] */
        acc += arr2[i-1];  /* Anti-dependency on arr2[i-1] from previous iteration */
        
        /* WAW: Multiple writes to arr2[i] */
        arr2[i] = temp * 2;  /* First write */
        if (acc > 100) {
            arr2[i] = temp * 3;  /* Second write - output dependency */
        }
        
        /* Loop-carried register dependency */
        prev = temp;  /* Creates edge with distance=1 */
        
        /* Control dependency */
        if (arr1[i] > 50) {
            acc -= 10;  /* Control-dependent on arr1[i] comparison */
        }
    }
    
    /* Prevent optimization */
    sink = acc + prev;
}

/* Test 2: Nested loops for SCC formation */
static void __attribute__((noinline, noipa))
test2_nested_scc(int n, int m, int* mat) {
    int sum = 0;
    
    /* Outer loop */
    for (int i = 1; i < n; ++i) {
        int row_acc = 0;
        
        /* Inner loop with loop-carried dependency */
        for (int j = 1; j < m; ++j) {
            /* Complex memory access pattern */
            int idx = i * m + j;
            int prev_idx = (i-1) * m + j;
            int left_idx = i * m + (j-1);
            
            /* Multiple dependencies forming potential cycles */
            int val1 = mat[prev_idx] + 1;    /* RAW on previous row */
            int val2 = mat[left_idx] * 2;    /* RAW on previous column */
            
            /* Cycle within iteration: val3 depends on val4, val4 depends on val3 */
            int val3 = val1 + val2;
            int val4 = val3 * 3;
            val3 = val4 - 1;  /* Creates cycle in dependency graph */
            
            mat[idx] = val3 + val4;
            row_acc += mat[idx];
            
            /* Anti-dependency */
            mat[prev_idx] = row_acc;  /* WAR on mat[prev_idx] */
        }
        
        sum += row_acc;
    }
    
    sink = sum;
}

/* Test 3: Pointer arithmetic with aliasing */
static void __attribute__((noinline, noipa))
test3_pointer_aliasing(int n, int* data) {
    int* ptr1 = data;
    int* ptr2 = data + n/2;
    int* ptr3 = data + n/4;
    
    int acc1 = 0, acc2 = 0;
    
    for (int i = 0; i < n/2; ++i) {
        /* Indirect accesses with potential aliasing */
        *ptr1 = *ptr2 + *ptr3;      /* RAW on *ptr2 and *ptr3 */
        acc1 += *ptr1;              /* RAW on *ptr1 */
        
        *ptr2 = acc1 - i;           /* WAW on *ptr2 */
        acc2 = *ptr2 * 2;           /* RAW on *ptr2 */
        
        *ptr3 = acc2 + *ptr1;       /* WAR on *ptr1, WAW on *ptr3 */
        
        /* Pointer arithmetic creates complex address dependencies */
        ptr1++;
        if (i % 3 == 0) {
            ptr2++;  /* Conditional pointer update */
        }
        ptr3 += (i & 1);  /* Patterned pointer update */
    }
    
    sink = acc1 + acc2;
}

/* Test 4: Matrix multiplication kernel with complex dependencies */
static void __attribute__((noinline, noipa))
test4_matrix_multiply(int n, int* A, int* B, int* C) {
    /* Simple matrix multiplication with added dependencies */
    for (int i = 0; i < n; ++i) {
        int row_acc = 0;
        
        for (int j = 0; j < n; ++j) {
            int sum = 0;
            int prev_sum = 0;
            
            for (int k = 0; k < n; ++k) {
                /* Standard matrix multiply */
                sum += A[i * n + k] * B[k * n + j];
                
                /* Added loop-carried dependency */
                if (k > 0) {
                    /* Use previous iteration's value */
                    sum += prev_sum / 2;  /* Distance 1 dependency */
                }
                
                /* Anti-dependency through temporary */
                prev_sum = sum;
                
                /* Output dependency through array */
                C[i * n + j] = sum;  /* Multiple writes in k-loop create WAW */
            }
            
            /* Control dependency based on computed value */
            if (sum > 1000) {
                row_acc += sum * 2;
            } else {
                row_acc += sum;
            }
        }
        
        /* Loop-carried dependency across i iterations */
        if (i > 0) {
            A[i * n] = row_acc + A[(i-1) * n];  /* Distance 1 memory dependency */
        }
    }
    
    /* Compute checksum */
    int total = 0;
    for (int i = 0; i < n * n; ++i) {
        total += C[i];
    }
    sink = total;
}

/* Test 5: Complex recurrence chain */
static void __attribute__((noinline, noipa))
test5_recurrence_chain(int n, int* arr) {
    int x = 1, y = 2, z = 3;
    
    for (int i = 0; i < n; ++i) {
        /* Chain of dependencies within single iteration */
        int t1 = x + y;     /* Depends on x, y */
        int t2 = y * z;     /* Depends on y, z */
        int t3 = t1 - t2;   /* Depends on t1, t2 */
        
        /* Cycle: x depends on z, z depends on new_x */
        int new_x = t3 + arr[i];
        int new_z = x * 2;  /* Depends on old x */
        
        /* Loop-carried dependencies with different distances */
        if (i >= 2) {
            y = arr[i-2] + new_x;  /* Distance 2 */
        }
        
        x = new_x;  /* Distance 1 */
        z = new_z;  /* Distance 1 */
        
        /* Memory dependency with stride */
        arr[i] = x + z;
        
        /* Control flow creating complex DDG */
        switch (i % 4) {
            case 0: x += 1; break;
            case 1: y *= 2; break;
            case 2: z -= 1; break;
            case 3: arr[i] = x + y + z; break;
        }
    }
    
    sink = x + y + z;
}

/* Main function orchestrating all tests */
int main(void) {
    /* Use volatile to prevent compile-time computation */
    volatile int N = get_iterations();
    int n = N;
    
    /* Allocate arrays with volatile initialization */
    int* arr1 = (int*)malloc(n * sizeof(int));
    int* arr2 = (int*)malloc(n * sizeof(int));
    int* arr3 = (int*)malloc(n * n * sizeof(int));
    int* arr4 = (int*)malloc(n * n * sizeof(int));
    int* arr5 = (int*)malloc(n * n * sizeof(int));
    
    if (!arr1 || !arr2 || !arr3 || !arr4 || !arr5) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant values */
    for (int i = 0; i < n; ++i) {
        arr1[i] = (i * 17) % 101;
        arr2[i] = (i * 23) % 103;
    }
    
    for (int i = 0; i < n * n; ++i) {
        arr3[i] = (i * 7) % 107;
        arr4[i] = (i * 11) % 109;
        arr5[i] = (i * 13) % 113;
    }
    
    /* Execute all test cases */
    test1_register_memory_deps(n, arr1, arr2);
    test2_nested_scc(n, n, arr3);
    test3_pointer_aliasing(n, arr1);
    test4_matrix_multiply(n/4, arr3, arr4, arr5);  /* Smaller size for n*n loops */
    test5_recurrence_chain(n, arr2);
    
    /* Compute final checksum */
    int checksum = 0;
    for (int i = 0; i < n; ++i) {
        checksum += arr1[i] + arr2[i];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Sink value: %d\n", sink);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(arr5);
    
    return 0;
}
