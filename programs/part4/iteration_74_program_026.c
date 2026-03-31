/* Test program to exercise DDG edge creation and initialization */
#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent optimization */
static int __attribute__((noinline, noipa)) get_iterations(void) {
    static volatile int counter = 100;
    return counter;
}

/* Dummy sink to prevent dead code elimination */
static volatile int sink;

/* Test 1: Simple loop with register and memory dependencies */
static void __attribute__((noinline, noipa)) 
test1_register_memory_deps(int n, int *restrict a, int *restrict b, int *restrict c) {
    int acc = 0;
    int prev = a[0];
    
    for (int i = 1; i < n; ++i) {
        /* RAW dependency on a[i-1] from previous iteration */
        int temp = prev + b[i];  
        
        /* WAR dependency: reading b[i] before writing to it */
        b[i] = temp * 2;         
        
        /* WAW dependency on acc */
        acc = acc + temp;        
        
        /* Loop-carried dependency with distance 1 */
        prev = a[i];             
        
        /* Output dependency on c[i] */
        c[i] = acc;              
        
        /* Anti-dependency: reading c[i-1] */
        if (i > 1) {
            sink = c[i-1];       
        }
    }
    
    /* Final sink to prevent optimization */
    sink = acc + prev;
}

/* Test 2: Nested loops for SCC formation */
static void __attribute__((noinline, noipa))
test2_nested_scc(int n, int m, int *restrict mat) {
    int sum = 0;
    
    for (int i = 1; i < n; ++i) {
        int row_acc = 0;
        
        /* Inner loop with loop-carried dependency */
        for (int j = 1; j < m; ++j) {
            /* Complex recurrence chain within iteration */
            int x = mat[i*m + j-1];
            int y = x + row_acc;
            mat[i*m + j] = y * 2;
            row_acc = mat[i*m + j] - x;
            
            /* Cross-iteration dependency in inner loop */
            if (j > 1) {
                mat[i*m + j] += mat[i*m + j-2];
            }
        }
        
        /* Loop-carried dependency in outer loop */
        sum += row_acc;
        
        /* Cross-iteration memory dependency */
        if (i > 1) {
            mat[i*m] += sum;
        }
    }
    
    sink = sum;
}

/* Test 3: Conditional dependencies and control flow */
static void __attribute__((noinline, noipa))
test3_conditional_deps(int n, int *restrict arr, int threshold) {
    int count = 0;
    int last_val = arr[0];
    
    for (int i = 1; i < n; ++i) {
        /* Control dependency based on loop-variant value */
        if (last_val > threshold) {
            /* True dependency chain inside conditional */
            int new_val = last_val * 2;
            arr[i] = new_val + i;
            count += new_val;
        } else {
            /* Alternative dependency chain */
            arr[i] = last_val / 2;
            count -= last_val;
        }
        
        /* Loop-carried dependency through last_val */
        last_val = arr[i];
        
        /* Anti-dependency through shared temporary */
        int tmp = arr[i-1];
        if (tmp % 2 == 0) {
            arr[i-1] = tmp + 1;  /* WAR on arr[i-1] */
        }
    }
    
    sink = count;
}

/* Test 4: Pointer arithmetic and indirect accesses */
static void __attribute__((noinline, noipa))
test4_pointer_aliasing(int n, int *restrict base) {
    int *ptr1 = base;
    int *ptr2 = base + n/2;
    int acc1 = 0, acc2 = 0;
    
    for (int i = 0; i < n/2; ++i) {
        /* Potential aliasing between ptr1 and ptr2 accesses */
        int val1 = *ptr1;
        int val2 = *ptr2;
        
        /* Cross-iteration dependency through pointers */
        acc1 = acc1 + val1;
        acc2 = acc2 * 2 + val2;
        
        /* Write with potential aliasing */
        *ptr1 = acc1;
        *ptr2 = acc2;
        
        /* Pointer arithmetic creating complex addressing */
        ptr1++;
        ptr2--;
        
        /* Recurrence within iteration */
        int temp = acc1;
        acc1 = acc2 + temp;
        acc2 = temp - acc1;
    }
    
    sink = acc1 + acc2;
}

/* Test 5: Matrix multiplication kernel with complex dependencies */
static void __attribute__((noinline, noipa))
test5_matrix_mult(int n, 
                  int *restrict A, 
                  int *restrict B, 
                  int *restrict C) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int sum = 0;
            
            /* Inner reduction loop with multiple dependencies */
            for (int k = 0; k < n; ++k) {
                /* True dependencies on A and B */
                int a_val = A[i*n + k];
                int b_val = B[k*n + j];
                
                /* Loop-carried dependency on sum */
                sum += a_val * b_val;
                
                /* Cross-iteration dependency in k-loop */
                if (k > 0) {
                    /* Use previous iteration's values */
                    sum += A[i*n + k-1] - B[(k-1)*n + j];
                }
            }
            
            /* Output dependency on C */
            C[i*n + j] = sum;
            
            /* Anti-dependency: read before potential write in next iteration */
            if (j > 0) {
                sink = C[i*n + j-1];
            }
        }
        
        /* Outer loop dependency */
        if (i > 0) {
            C[i*n] += C[(i-1)*n + n-1];
        }
    }
}

/* Test 6: Complex recurrence with multiple dependency types */
static void __attribute__((noinline, noipa))
test6_mixed_dependencies(int n, int *restrict data) {
    int x = data[0];
    int y = data[1];
    int z = 0;
    
    for (int i = 2; i < n; ++i) {
        /* Cycle of dependencies within iteration */
        int t1 = x + y;      /* depends on x, y from prev iteration or cycle */
        int t2 = y * z;      /* depends on y, z */
        int t3 = t1 - t2;    /* depends on t1, t2 */
        
        /* Loop-carried dependencies with different distances */
        x = t3 + data[i];    /* distance 1 */
        if (i > 3) {
            y = data[i-2] + z;  /* distance 2 */
        }
        z = t2 - x;          /* distance 1 */
        
        /* Memory dependencies with stride */
        data[i] = x + y + z;
        
        /* Control dependency */
        if (x > y) {
            data[i-1] = z;   /* WAR on data[i-1] */
        }
    }
    
    sink = x + y + z;
}

int main(void) {
    /* Use volatile to get non-constant loop bounds */
    volatile int iterations = get_iterations();
    int n = iterations;
    
    /* Allocate test arrays */
    int *arr1 = (int*)malloc(n * sizeof(int));
    int *arr2 = (int*)malloc(n * sizeof(int));
    int *arr3 = (int*)malloc(n * sizeof(int));
    int *matrix = (int*)malloc(n * n * sizeof(int));
    
    if (!arr1 || !arr2 || !arr3 || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant values */
    for (int i = 0; i < n; ++i) {
        arr1[i] = i * 3 % 7;
        arr2[i] = i * 5 % 11;
        arr3[i] = i * 7 % 13;
    }
    
    for (int i = 0; i < n * n; ++i) {
        matrix[i] = i % 17;
    }
    
    /* Execute test cases */
    test1_register_memory_deps(n, arr1, arr2, arr3);
    test2_nested_scc(n/2, n/2, matrix);
    test3_conditional_deps(n, arr1, 10);
    test4_pointer_aliasing(n, arr2);
    test5_matrix_mult(n/4, arr1, arr2, arr3);
    test6_mixed_dependencies(n, arr1);
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < n; ++i) {
        checksum += arr1[i] + arr2[i] + arr3[i];
    }
    
    for (int i = 0; i < n * n; ++i) {
        checksum += matrix[i];
    }
    
    checksum += sink;
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(matrix);
    
    return 0;
}
