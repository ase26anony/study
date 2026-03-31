/* Test program to trigger DDG edge creation and initialization */
#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent optimization */
static int __attribute__((noinline, noipa)) get_iteration_count(int seed) {
    volatile int v = seed;
    return v + 100; /* Non-constant iteration count */
}

/* Dummy sink to prevent dead code elimination */
static volatile int sink;

/* Test 1: Simple loop with register and memory dependencies */
static void __attribute__((noinline, noipa)) 
test1_register_memory_deps(int n, int* arr1, int* arr2) {
    int acc = 0;
    int prev = arr1[0];
    
    /* Loop with multiple dependency types */
    for (int i = 1; i < n; ++i) {
        /* RAW (true) dependency: arr1[i-1] -> arr1[i] */
        int temp = arr1[i-1] + i;
        
        /* WAR (anti) dependency: arr2[i] read then modified */
        int read_val = arr2[i];
        
        /* WAW (output) dependency: arr1[i] written twice */
        arr1[i] = temp + read_val;
        arr1[i] = arr1[i] * 2;  // Second write creates WAW
        
        /* Loop-carried register dependency (distance 1) */
        acc = acc + arr1[i];  // acc_i depends on acc_{i-1}
        
        /* Loop-carried memory dependency (distance 2) */
        if (i >= 2) {
            arr2[i] = arr2[i-2] + acc;  // distance 2 dependency
        } else {
            arr2[i] = read_val + 1;
        }
        
        /* Control dependency */
        if (arr1[i] > 1000) {
            prev = arr1[i];  // Control-dependent assignment
        }
    }
    
    sink = acc + prev;  /* Volatile sink */
}

/* Test 2: Nested loops for SCC formation */
static void __attribute__((noinline, noipa))
test2_nested_loops_scc(int n, int m, int* mat) {
    /* Outer loop with inner SCC */
    for (int i = 1; i < n; ++i) {
        int x = mat[i * m];
        int y = 0;
        
        /* Inner loop with loop-carried dependency forming SCC */
        for (int j = 1; j < m; ++j) {
            /* Cycle within iteration: x -> y -> x */
            y = x + mat[i * m + j - 1];  // RAW: x read
            x = y * 2 - j;               // RAW: y read, WAR: x written
            
            /* Loop-carried in inner loop (distance 1) */
            mat[i * m + j] = mat[i * m + j - 1] + x;
            
            /* Anti-dependency in inner loop */
            int temp = mat[(i-1) * m + j];  // Read
            mat[(i-1) * m + j] = temp + y;  // Write (WAR)
        }
        
        /* Cross-iteration dependency in outer loop */
        mat[i * m] = mat[(i-1) * m] + x;
    }
    
    sink = mat[n * m - 1];
}

/* Test 3: Complex recurrence chain */
static void __attribute__((noinline, noipa))
test3_recurrence_chain(int n, float* data) {
    float a = 1.0f, b = 2.0f, c = 3.0f;
    
    for (int i = 0; i < n; ++i) {
        /* Recurrence chain: a -> b -> c -> a (cycle within iteration) */
        float new_a = b * data[i];      // RAW: b
        float new_b = c + data[i];      // RAW: c
        float new_c = a - data[i];      // RAW: a
        
        /* Loop-carried with distance 3 */
        if (i >= 3) {
            data[i] = data[i-3] + new_a + new_b + new_c;
        }
        
        /* Update all three for next iteration */
        a = new_a;
        b = new_b;
        c = new_c;
        
        /* Output dependency on data */
        data[i] = data[i] * 0.5f;
        data[i] = data[i] + 1.0f;  // WAW on data[i]
    }
    
    volatile float fsink;
    fsink = a + b + c;
    sink = (int)fsink;
}

/* Test 4: Pointer arithmetic with aliasing */
static void __attribute__((noinline, noipa))
test4_pointer_aliasing(int n, int* base) {
    int* p1 = base;
    int* p2 = base + n/2;
    int* p3 = base + n/4;
    
    int sum = 0;
    for (int i = 0; i < n/2; ++i) {
        /* Potential aliasing creates complex memory deps */
        *p1 = *p2 + *p3;           // Reads through p2, p3
        
        /* Pointer arithmetic creates loop-carried dependency */
        p1++;
        p2--;
        p3 += (i % 2) ? 1 : -1;
        
        /* Anti-dependency through pointers */
        int val = *p1;             // Read
        *p1 = val + i;             // Write (WAR)
        
        /* Register dependency chain */
        sum = sum + *p1 + *p2;
        
        /* Control dependency based on pointer value */
        if (sum > 10000) {
            p3 = base;  // Changes pointer for future iterations
        }
    }
    
    sink = sum;
}

/* Test 5: If-converted style with complex conditions */
static void __attribute__((noinline, noipa))
test5_control_deps(int n, int* arr, int* brr) {
    int x = 0, y = 0, z = 0;
    
    for (int i = 1; i < n; ++i) {
        /* Multiple conditions creating control dependencies */
        int cond1 = arr[i] > arr[i-1];
        int cond2 = brr[i] < brr[i-1];
        int cond3 = (arr[i] + brr[i]) % 2;
        
        /* Data flow with control dependencies */
        int t = x;
        if (cond1) {
            t = y + arr[i];    // Control-dependent on cond1
        }
        
        if (cond2) {
            y = z * 2;         // Control-dependent on cond2
        } else {
            y = t - 1;         // Alternative control-dependent path
        }
        
        if (cond3) {
            z = arr[i] + i;    // Control-dependent on cond3
        }
        
        /* Loop-carried through control-dependent variables */
        x = y + z;
        
        /* Memory dependency with distance */
        if (i >= 4) {
            arr[i] = brr[i-4] + x;  // Distance 4 memory dependency
        }
    }
    
    sink = x + y + z;
}

/* Matrix multiplication kernel for complex DDG patterns */
static void __attribute__((noinline, noipa))
test6_matrix_multiply(int n, int* A, int* B, int* C) {
    /* Simple matrix multiply with dependencies */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int sum = 0;
            for (int k = 0; k < n; ++k) {
                /* RAW: A[i*n+k] and B[k*n+j] reads */
                /* WAW: sum written each k iteration */
                sum += A[i*n + k] * B[k*n + j];
                
                /* Anti-dependency through B */
                int temp = B[k*n + j];
                B[k*n + j] = temp + 1;  // WAR on B
            }
            /* Output dependency on C */
            C[i*n + j] = 0;
            C[i*n + j] = sum;  // WAW on C[i*n+j]
        }
        
        /* Loop-carried dependency in i-loop */
        if (i > 0) {
            A[i*n] = C[(i-1)*n] + i;  // Distance 1 memory dependency
        }
    }
    
    /* Compute checksum */
    int total = 0;
    for (int i = 0; i < n*n; ++i) {
        total += C[i];
    }
    sink = total;
}

int main(int argc, char** argv) {
    /* Use volatile to get non-constant iteration counts */
    volatile int seed = 100;
    int n = get_iteration_count(seed);
    int m = n / 2;
    
    /* Allocate arrays with volatile initialization */
    int* arr1 = (int*)malloc(n * sizeof(int));
    int* arr2 = (int*)malloc(n * sizeof(int));
    float* farr = (float*)malloc(n * sizeof(float));
    int* mat = (int*)malloc(n * m * sizeof(int));
    
    /* Initialize with non-constant values */
    for (int i = 0; i < n; ++i) {
        arr1[i] = i * 3 % 7;
        arr2[i] = i * 5 % 11;
        farr[i] = (float)(i % 13) * 0.1f;
    }
    
    for (int i = 0; i < n * m; ++i) {
        mat[i] = i % 17;
    }
    
    /* Run all test cases */
    test1_register_memory_deps(n, arr1, arr2);
    
    test2_nested_loops_scc(n, m, mat);
    
    test3_recurrence_chain(n, farr);
    
    test4_pointer_aliasing(n, arr1);
    
    test5_control_deps(n, arr1, arr2);
    
    /* Matrix test with smaller size for performance */
    int mat_size = (n > 50) ? 50 : n;
    int* A = (int*)malloc(mat_size * mat_size * sizeof(int));
    int* B = (int*)malloc(mat_size * mat_size * sizeof(int));
    int* C = (int*)malloc(mat_size * mat_size * sizeof(int));
    
    for (int i = 0; i < mat_size * mat_size; ++i) {
        A[i] = i % 19;
        B[i] = i % 23;
        C[i] = 0;
    }
    
    test6_matrix_multiply(mat_size, A, B, C);
    
    /* Aggregate results into volatile sink */
    int total = sink;
    total += arr1[n-1] + arr2[n-2] + (int)farr[n-3] + mat[n*m-1];
    
    printf("Result checksum: %d\n", total);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr);
    free(mat);
    free(A);
    free(B);
    free(C);
    
    return 0;
}
