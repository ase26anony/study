/* ddg_test.c - Test program for Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent optimization */
static int __attribute__((noinline, noipa)) get_iteration_count(int seed) {
    volatile int v = seed;
    return v + 1000;  /* Non-constant iteration count */
}

/* Dummy sink to prevent dead code elimination */
static volatile int sink = 0;

/* Test 1: Simple loop with register and memory dependencies */
static void __attribute__((noinline, noipa)) 
test1_register_memory_deps(int n, int* arr1, int* arr2) {
    int acc = arr1[0];  /* Initial accumulator */
    
    /* Loop with RAW, WAR, and WAW dependencies */
    for (int i = 1; i < n; ++i) {
        /* RAW: Read arr1[i-1] before overwriting it */
        int temp = arr1[i-1] + arr2[i];
        
        /* WAR: Read arr1[i] before potentially overwriting in next iteration */
        acc = acc + arr1[i];
        
        /* WAW: Multiple writes to arr1[i] with different expressions */
        arr1[i] = temp * 2;
        arr1[i] = arr1[i] + acc;  /* Overwrite with new value */
        
        /* Loop-carried dependency on acc (distance 1) */
        arr2[i] = acc - i;
    }
    
    sink = acc;  /* Prevent elimination */
}

/* Test 2: Complex loop with control dependencies */
static void __attribute__((noinline, noipa))
test2_control_deps(int n, float* data, float threshold) {
    float sum = 0.0f;
    float max_val = data[0];
    int count = 0;
    
    for (int i = 1; i < n; ++i) {
        /* Control dependency: Branch depends on loop-variant value */
        if (data[i-1] > threshold) {
            /* True dependency chain within conditional */
            max_val = (data[i] > max_val) ? data[i] : max_val;
            count++;
        } else {
            /* Alternative dependency chain */
            sum += data[i];
        }
        
        /* Loop-carried anti-dependency (WAR) */
        float old_data = data[i];
        data[i] = max_val * 0.5f + sum;
        
        /* Output dependency (WAW) on data[i] */
        if (count % 2 == 0) {
            data[i] = old_data * 2.0f;  /* Overwrites previous write */
        }
    }
    
    sink = (int)(sum + max_val + count);
}

/* Test 3: Nested loops for SCC formation */
static void __attribute__((noinline, noipa))
test3_nested_scc(int n, int m, int* matrix) {
    /* Outer loop with dependency */
    for (int i = 1; i < n; ++i) {
        int row_acc = matrix[i * m];
        
        /* Inner loop with loop-carried dependency (forms SCC) */
        for (int j = 1; j < m; ++j) {
            /* Recurrence chain within inner loop iteration */
            int idx = i * m + j;
            int prev_idx = i * m + (j - 1);
            
            /* Cycle of dependencies: x depends on y, y depends on x */
            int x = matrix[prev_idx] + j;
            int y = x * 2 - matrix[idx];
            matrix[idx] = y + x;  /* Uses both x and y */
            
            /* Loop-carried in inner loop */
            row_acc += matrix[idx];
        }
        
        /* Loop-carried in outer loop */
        matrix[i * m] = row_acc;
    }
    
    sink = matrix[(n-1) * m + (m-1)];
}

/* Test 4: Pointer arithmetic and indirect accesses */
static void __attribute__((noinline, noipa))
test4_pointer_aliasing(int n, int* base, int* offsets) {
    int* ptr1 = base;
    int* ptr2 = base + n/2;
    
    /* Complex pointer-based dependencies */
    for (int i = 0; i < n/2; ++i) {
        /* Aliasing possibilities create ambiguous dependencies */
        int val1 = *ptr1;
        int val2 = *(ptr2 + offsets[i]);
        
        /* RAW through pointers */
        *ptr1 = val1 + val2;
        
        /* WAR: Reading what was just written through different pointer */
        int val3 = *(base + i);
        
        /* WAW: Multiple writes to potentially overlapping locations */
        *(ptr1 + 1) = val3 * 2;
        
        /* Update pointers with loop-carried dependency */
        ptr1++;
        ptr2 = ptr2 + (val1 % 3);  /* Non-linear pointer update */
    }
    
    sink = *base;
}

/* Test 5: Matrix multiplication kernel with complex dependencies */
static void __attribute__((noinline, noipa))
test5_matrix_multiply(int n, int* A, int* B, int* C) {
    /* Simple matrix multiplication with dependencies */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int sum = 0;
            
            /* Innermost loop with reduction dependency */
            for (int k = 0; k < n; ++k) {
                /* Memory dependencies with different access patterns */
                int a_idx = i * n + k;
                int b_idx = k * n + j;
                int c_idx = i * n + j;
                
                /* RAW on A[a_idx] and B[b_idx] */
                sum += A[a_idx] * B[b_idx];
                
                /* Anti-dependency on C through accumulation */
                C[c_idx] = sum;  /* WAR on C[c_idx] across k iterations */
            }
            
            /* Final write with output dependency */
            C[i * n + j] = sum;
        }
    }
    
    sink = C[n * n - 1];
}

/* Test 6: Recurrence chain within single iteration */
static void __attribute__((noinline, noipa))
test6_recurrence_chain(int n, int* data) {
    int x = data[0];
    int y = data[1];
    int z = data[2];
    
    for (int i = 3; i < n; ++i) {
        /* Cycle of dependencies within one iteration */
        int new_x = y + z;
        int new_y = x * 2 - z;
        int new_z = new_x + new_y + data[i];
        
        /* Loop-carried dependencies */
        x = new_x + i;
        y = new_y - i;
        z = new_z * 2;
        
        /* Memory dependency with distance 2 */
        if (i >= 2) {
            data[i] = data[i-2] + x;
        }
    }
    
    sink = x + y + z;
}

/* Main driver */
int main(int argc, char** argv) {
    /* Use volatile to get non-constant iteration counts */
    volatile int base_iter = 100;
    int n = get_iteration_count(base_iter);
    int m = n / 2;
    
    /* Allocate and initialize test arrays */
    int* arr1 = (int*)malloc(n * sizeof(int));
    int* arr2 = (int*)malloc(n * sizeof(int));
    float* farr = (float*)malloc(n * sizeof(float));
    int* matrix = (int*)malloc(n * m * sizeof(int));
    int* offsets = (int*)malloc(n * sizeof(int));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < n; ++i) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 5 - 2;
        farr[i] = (float)(i % 100) * 0.1f;
        offsets[i] = (i % 7) - 3;
    }
    
    for (int i = 0; i < n * m; ++i) {
        matrix[i] = (i * 11) % 97;
    }
    
    /* Execute all test cases */
    test1_register_memory_deps(n, arr1, arr2);
    test2_control_deps(n, farr, 5.0f);
    test3_nested_scc(n/4, m/4, matrix);
    test4_pointer_aliasing(n, arr1, offsets);
    test6_recurrence_chain(n, arr2);
    
    /* For matrix multiply, use smaller size to avoid excessive runtime */
    int small_n = 50;
    int* A = (int*)malloc(small_n * small_n * sizeof(int));
    int* B = (int*)malloc(small_n * small_n * sizeof(int));
    int* C = (int*)malloc(small_n * small_n * sizeof(int));
    
    for (int i = 0; i < small_n * small_n; ++i) {
        A[i] = (i * 7) % 53;
        B[i] = (i * 13) % 61;
        C[i] = 0;
    }
    
    test5_matrix_multiply(small_n, A, B, C);
    
    /* Compute simple checksum */
    int checksum = sink;
    for (int i = 0; i < n; i += n/10) {
        checksum += arr1[i] + arr2[i];
    }
    
    printf("DDG Test Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr);
    free(matrix);
    free(offsets);
    free(A);
    free(B);
    free(C);
    
    return 0;
}
