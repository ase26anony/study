/* ddg_test.c - Program to exercise Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent optimization */
static int __attribute__((noinline, noipa)) get_iteration_count(int seed) {
    volatile int v = seed;
    return v + 100;  /* Non-constant iteration count */
}

/* Dummy sink to prevent dead code elimination */
static volatile int sink;

/* Test 1: Simple loop with register and memory dependencies */
static void __attribute__((noinline, noipa)) 
test1_register_memory_deps(int n, int* arr1, int* arr2) {
    int acc = 0;
    int prev = arr1[0];
    
    /* Loop with RAW, WAR, and WAW dependencies */
    for (int i = 1; i < n; ++i) {
        /* RAW: Read arr1[i] before writing arr2[i] */
        int temp = arr1[i] + acc;      /* Uses acc from previous iteration (distance 1) */
        
        /* WAR: Read arr2[i-1] then write arr2[i] */
        arr2[i] = arr2[i-1] + temp;    /* Anti-dependency on arr2[i-1] */
        
        /* WAW: Multiple writes to acc */
        acc = temp * 2;                 /* Output dependency on acc */
        
        /* Loop-carried memory dependency */
        arr1[i] = prev + i;            /* True dependency through prev */
        prev = arr1[i];                /* Distance 1 register dependency */
    }
    
    sink = acc + arr2[n-1];  /* Volatile sink */
}

/* Test 2: Nested loops forming SCCs */
static void __attribute__((noinline, noipa))
test2_nested_loops_scc(int n, int m, int* mat) {
    /* Outer loop with carried dependency */
    int outer_acc = 0;
    for (int i = 0; i < n; ++i) {
        /* Inner loop forms SCC within itself */
        int inner_acc = 0;
        int prev_val = mat[i * m];
        
        for (int j = 1; j < m; ++j) {
            /* Cycle of dependencies within one iteration */
            int idx = i * m + j;
            
            /* RAW: Read then write */
            int x = prev_val + inner_acc;
            int y = x * 2 - j;
            
            /* WAR: Read old value before write */
            int old = mat[idx];
            mat[idx] = y + old;        /* Anti-dependency on mat[idx] */
            
            /* Update for next iteration */
            prev_val = mat[idx];       /* Distance 1 memory dependency */
            inner_acc = y;             /* Distance 1 register dependency */
            
            /* Control dependency */
            if (inner_acc > 100) {     /* Condition depends on loop-variant value */
                outer_acc += 1;        /* Control-dependent update */
            }
        }
        
        /* Loop-carried in outer loop */
        outer_acc += inner_acc;
    }
    
    sink = outer_acc;
}

/* Test 3: Complex recurrence chain */
static void __attribute__((noinline, noipa))
test3_recurrence_chain(int n, float* data) {
    float a = 1.0f, b = 2.0f, c = 3.0f;
    
    /* Loop with multiple interleaved recurrences */
    for (int i = 0; i < n; ++i) {
        /* Chain of dependencies within iteration */
        float t1 = a + data[i];    /* RAW on a */
        float t2 = b * t1;         /* RAW on b, t1 */
        float t3 = c - t2;         /* RAW on c, t2 */
        
        /* Circular dependency pattern */
        a = t3 * 0.5f;             /* WAW on a, RAW on t3 */
        b = a + t1;                /* WAW on b, RAW on a, t1 */
        c = b / (t2 + 1.0f);       /* WAW on c, RAW on b, t2 */
        
        /* Memory dependency with stride */
        if (i >= 2) {
            data[i] = data[i-2] * data[i-1];  /* Distance 2 memory dependency */
        }
    }
    
    /* Use volatile to force computation */
    volatile float vsink = a + b + c;
    sink = (int)vsink;
}

/* Test 4: Pointer aliasing and indirect accesses */
static void __attribute__((noinline, noipa))
test4_pointer_aliasing(int n, int* base1, int* base2) {
    int* p1 = base1;
    int* p2 = base2;
    int* p3 = base1 + n/2;  /* Potential alias */
    
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        /* Pointer arithmetic creating complex dependencies */
        int val1 = *p1;           /* Read through p1 */
        int val2 = *p2;           /* Read through p2 */
        
        /* Potential WAR: Read *p3 before write if p3 aliases p1/p2 */
        int temp = val1 + val2 + *p3;
        
        /* Write to memory locations */
        *p1 = temp + i;           /* WAW on *p1 */
        *p2 = *p1 - val2;         /* RAW on *p1, WAR on val2 */
        
        /* Update pointers with different strides */
        p1 += 1;                  /* Linear access */
        p2 += 2;                  /* Non-unit stride */
        p3 = (i % 3 == 0) ? base1 : base2;  /* Aliasing changes */
        
        /* Loop-carried through sum */
        sum = sum + temp;         /* Distance 1 register dependency */
    }
    
    sink = sum;
}

/* Test 5: Mixed data types and control flow */
static void __attribute__((noinline, noipa))
test5_mixed_control_flow(int n, short* shorts, int* ints, char* chars) {
    int int_acc = 0;
    short short_acc = 0;
    char char_acc = 0;
    
    for (int i = 1; i < n; ++i) {
        /* Different data types creating different dependency edges */
        short s_temp = shorts[i] + short_acc;
        int i_temp = ints[i-1] + int_acc;
        char c_temp = chars[i] + char_acc;
        
        /* Control-dependent updates */
        if (s_temp > 1000) {           /* Control dependency */
            ints[i] = i_temp * 2;      /* Memory write */
            int_acc = ints[i] / 3;     /* RAW on ints[i] */
        } else {
            ints[i] = i_temp / 2;
            int_acc = ints[i] + 1;
        }
        
        /* Another control flow with loop-carried dependency */
        switch (i % 4) {
            case 0:
                shorts[i] = s_temp + 1;
                short_acc = shorts[i];  /* Distance 1 */
                break;
            case 1:
                chars[i] = c_temp;
                char_acc = chars[i-1];  /* Distance 1 memory */
                break;
            default:
                /* Cross-iteration dependency chain */
                shorts[i] = shorts[i-1] + chars[i-1];  /* Distance 1 */
                break;
        }
    }
    
    sink = int_acc + short_acc + char_acc;
}

/* Matrix multiplication kernel for complex DDG patterns */
static void __attribute__((noinline, noipa))
test6_matrix_multiply(int n, int* A, int* B, int* C) {
    /* Simple matrix multiply with dependencies */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int sum = 0;
            for (int k = 0; k < n; ++k) {
                /* RAW dependencies on A and B */
                int a_val = A[i * n + k];
                int b_val = B[k * n + j];
                
                /* Multiply and accumulate with loop-carried dependency */
                sum = sum + a_val * b_val;  /* Distance 1 in k-loop */
                
                /* Anti-dependency: modify B element for next i iteration */
                if (k == n-1 && i > 0) {
                    B[k * n + j] += 1;      /* WAR on B */
                }
            }
            
            /* Output dependency on C */
            C[i * n + j] = sum;             /* WAW on C */
            
            /* Cross-iteration dependency in j-loop */
            if (j > 0) {
                C[i * n + j] += C[i * n + j-1] / 2;  /* Distance 1 in j-loop */
            }
        }
        
        /* Outer loop carried dependency */
        if (i > 0) {
            A[i * n] += C[(i-1) * n + n-1];  /* Distance 1 in i-loop */
        }
    }
    
    /* Compute checksum */
    int total = 0;
    for (int i = 0; i < n * n; ++i) {
        total += C[i];
    }
    sink = total;
}

int main(int argc, char** argv) {
    /* Use volatile to prevent constant propagation */
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 42;
    int n = get_iteration_count(seed);
    
    /* Allocate arrays with volatile elements to prevent optimization */
    int size = (n > 1000) ? 1000 : n + 10;
    int* arr1 = (int*)malloc(size * sizeof(int));
    int* arr2 = (int*)malloc(size * sizeof(int));
    float* farr = (float*)malloc(size * sizeof(float));
    short* sarr = (short*)malloc(size * sizeof(short));
    char* carr = (char*)malloc(size * sizeof(char));
    
    /* Initialize with non-constant values */
    for (int i = 0; i < size; ++i) {
        arr1[i] = (i * seed) % 100;
        arr2[i] = (i + seed) % 100;
        farr[i] = (float)(i % 50) * 0.5f;
        sarr[i] = (short)((i * 3) % 1000);
        carr[i] = (char)((i + 5) % 128);
    }
    
    /* Execute all test cases to trigger various DDG edge creations */
    test1_register_memory_deps(n, arr1, arr2);
    
    int matrix_size = (n > 30) ? 30 : n;
    int* mat = (int*)malloc(matrix_size * matrix_size * sizeof(int));
    for (int i = 0; i < matrix_size * matrix_size; ++i) {
        mat[i] = (i * 7) % 100;
    }
    test2_nested_loops_scc(matrix_size, matrix_size, mat);
    
    test3_recurrence_chain(n, farr);
    test4_pointer_aliasing(n, arr1, arr2);
    test5_mixed_control_flow(n, sarr, arr1, carr);
    
    /* Matrix multiplication test */
    int small_n = (n > 20) ? 20 : (n > 5 ? n : 5);
    int* A = (int*)malloc(small_n * small_n * sizeof(int));
    int* B = (int*)malloc(small_n * small_n * sizeof(int));
    int* C = (int*)malloc(small_n * small_n * sizeof(int));
    
    for (int i = 0; i < small_n * small_n; ++i) {
        A[i] = (i * 11) % 50;
        B[i] = (i * 13) % 50;
        C[i] = 0;
    }
    test6_matrix_multiply(small_n, A, B, C);
    
    /* Aggregate results into volatile sink */
    volatile int final_sink = sink;
    
    /* Print something to prevent complete optimization */
    printf("DDG test completed. Checksum: %d\n", final_sink);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr);
    free(sarr);
    free(carr);
    free(mat);
    free(A);
    free(B);
    free(C);
    
    return 0;
}
