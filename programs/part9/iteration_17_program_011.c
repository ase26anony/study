/* test_hw_doloop.c - Complex loop nesting patterns to trigger bitmap intersection logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define SIZE 1024
#define ITERATIONS 100

/* Force inlining of helper functions */
#define ALWAYS_INLINE __attribute__((always_inline)) inline

/* Global arrays to prevent optimization */
int global_array[SIZE];
int global_matrix[SIZE][SIZE];
int global_results[10] = {0};

/* Test 1: Perfectly nested loops with complex control flow */
ALWAYS_INLINE void test_perfect_nesting(int *arr, int n) {
    int sum = 0;
    
    /* Outer loop with multiple basic blocks */
    for (int i = 0; i < n; i += (i % 3) + 1) {
        if (i % 2 == 0) {
            /* First basic block path */
            for (int j = 0; j < i; j++) {
                /* Inner loop with early exit */
                if (j > n / 2) {
                    break;  /* Creates exit edge */
                }
                sum += arr[j] * (i - j);
                
                /* Conditional continue */
                if (j % 5 == 0) {
                    continue;  /* Skips to next iteration */
                }
                
                /* Another basic block in inner loop */
                sum -= arr[j] / 2;
            }
        } else {
            /* Second basic block path in outer loop */
            int k = i;
            while (k > 0) {
                /* Nested while loop */
                sum += arr[k] * k;
                k -= 2;
                
                /* Switch inside while creates multiple blocks */
                switch (k % 4) {
                    case 0:
                        sum += 1;
                        break;
                    case 1:
                        sum += 2;
                        /* Fall through */
                    case 2:
                        sum += 3;
                        break;
                    default:
                        sum += 4;
                }
            }
        }
        
        /* Common block after if-else */
        arr[i] = sum % 1000;
    }
    
    global_results[0] = sum;
}

/* Test 2: Partially overlapping loops with shared blocks */
void test_partial_overlap(int matrix[SIZE][SIZE], int rows, int cols) {
    int diag_sum = 0;
    int row_sum = 0;
    
    /* Loop A: Processes diagonal */
    for (int i = 0; i < rows && i < cols; i++) {
        /* Shared block with Loop B */
        int temp = matrix[i][i];
        
        /* Conditional creates split block */
        if (temp > 0) {
            diag_sum += temp * 2;
        } else {
            diag_sum -= temp;
        }
        
        /* Loop B: Processes row i (partially overlaps with Loop A) */
        for (int j = 0; j < cols; j++) {
            /* This block is shared when j == i */
            row_sum += matrix[i][j];
            
            /* Early exit creates another block */
            if (row_sum > 10000) {
                goto early_exit;  /* Creates non-standard control flow */
            }
        }
        
        /* Continue outer loop */
        diag_sum = diag_sum % 1000;
    }
    
early_exit:
    /* Loop C: Independent but in same function */
    int k = 0;
    do {
        matrix[k % rows][k % cols] = diag_sum + row_sum;
        k++;
        
        /* Nested infinite loop with conditional break */
        for (;;) {
            if (k > 50) break;  /* Multiple break points */
            if (k % 7 == 0) break;
            k++;
        }
    } while (k < 100);
    
    global_results[1] = diag_sum + row_sum;
}

/* Test 3: Sibling loops with common outer loop */
ALWAYS_INLINE void test_sibling_loops(int *arr, int n) {
    int even_sum = 0, odd_sum = 0;
    
    /* Outer loop containing two sibling inner loops */
    for (int block = 0; block < n; block += SIZE/4) {
        int limit = (block + SIZE/4) < n ? (block + SIZE/4) : n;
        
        /* Sibling Loop 1: Processes even indices */
        for (int i = block; i < limit; i += 2) {
            even_sum += arr[i];
            
            /* Complex condition with short-circuit */
            if (i > 0 && (arr[i-1] % 2 == 0 || arr[i] < 100)) {
                even_sum *= 2;
            }
        }
        
        /* Some computation between siblings */
        even_sum = even_sum % 777;
        
        /* Sibling Loop 2: Processes odd indices (disjoint blocks) */
        for (int i = block + 1; i < limit; i += 2) {
            odd_sum += arr[i];
            
            /* Nested switch for block complexity */
            switch (i % 3) {
                case 0: odd_sum += arr[i] * 3; break;
                case 1: 
                    odd_sum += arr[i] * 5;
                    /* Fall through */
                case 2:
                    odd_sum -= arr[i];
                    break;
            }
        }
        
        /* Conditional goto creates additional edges */
        if (even_sum + odd_sum > 1000000) {
            goto finish;
        }
    }
    
finish:
    global_results[2] = even_sum + odd_sum;
}

/* Test 4: Loops with multiple entry points via labels */
void test_multiple_entries(int *arr, int n) {
    int result = 0;
    int i = 0;
    
    /* Loop with label for goto entry */
    outer_loop:
    while (i < n) {
        /* First entry path */
        if (arr[i] == 0) {
            i++;
            continue;
        }
        
        /* Inner loop with multiple exits */
        for (int j = 0; j < 10; j++) {
            result += arr[i] * j;
            
            if (result % 7 == 0) {
                goto skip_rest;  /* Exit to middle of outer loop */
            }
            
            if (j == 5) {
                goto alternate_path;  /* Jump to different block */
            }
        }
        
        /* Normal path */
        result = result % 100;
        i += 2;
        continue;
        
    alternate_path:
        /* Alternate loop body block */
        result += arr[i] * 100;
        i += 3;
        
        /* Nested do-while */
        int k = 0;
        do {
            result -= k;
            k++;
            
            /* Conditional continue */
            if (k % 4 == 0) continue;
            
            result += k * 2;
        } while (k < 5);
        
        continue;
        
    skip_rest:
        /* Skip to next iteration */
        i++;
        if (i % 20 == 0) {
            goto outer_loop;  /* Re-enter loop */
        }
    }
    
    /* Another loop that shares some blocks via function call */
    for (int p = 0; p < n; p++) {
        if (p % 2 == 0) {
            /* Call to function that gets inlined */
            helper_loop(arr, p, &result);
        } else {
            result += p * p;
        }
    }
    
    global_results[3] = result;
}

/* Helper function for inlining */
ALWAYS_INLINE void helper_loop(int *arr, int start, int *result) {
    /* This will be inlined, creating more loop nesting */
    for (int q = start; q < start + 10; q++) {
        *result += arr[q % SIZE];
        
        /* Nested if-else chain */
        if (q % 3 == 0) {
            *result *= 2;
        } else if (q % 3 == 1) {
            *result /= 2;
        } else {
            *result += q;
        }
    }
}

/* Test 5: Recursive function creating loop-like structures */
int test_recursive_loops(int *arr, int depth, int idx) {
    if (depth <= 0 || idx >= SIZE) {
        return arr[idx % SIZE];
    }
    
    int sum = 0;
    
    /* Loop within recursive function */
    for (int i = 0; i < depth; i++) {
        sum += arr[(idx + i) % SIZE];
        
        /* Tail recursion creates loop-like flow */
        if (i % 4 == 0) {
            sum += test_recursive_loops(arr, depth - 1, idx + i);
        }
    }
    
    /* Another loop with different structure */
    int j = depth;
    while (j > 0) {
        sum -= arr[(idx + j) % SIZE];
        j -= 2;
        
        /* Switch with multiple cases */
        switch (j % 5) {
            case 0: sum += 10; break;
            case 1: sum += 20; break;
            case 2: sum += 30; break;
            case 3: sum += 40; break;
            case 4: sum += 50; break;
        }
    }
    
    return sum;
}

/* Test 6: Hardware optimization hints */
void test_hardware_hints(int *arr, int n) {
    int sum = 0;
    
    /* Loop with known bounds and restrict */
    int *restrict ptr = arr;
    
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        /* Use builtin for branch prediction */
        if (__builtin_expect(ptr[i] > 0, 1)) {
            sum += ptr[i] * 2;
        } else {
            sum -= ptr[i];
        }
        
        /* Strided access pattern */
        for (int stride = 1; stride <= 4; stride <<= 1) {
            if (i + stride < n) {
                sum += ptr[i + stride];
            }
        }
    }
    
    /* Matrix multiplication pattern */
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            int dot = 0;
            for (int k = 0; k < 32; k++) {
                /* Hardware-friendly access pattern */
                dot += global_matrix[i][k] * global_matrix[k][j];
            }
            global_matrix[i][j] = dot;
            sum += dot;
        }
    }
    
    global_results[4] = sum;
}

/* Main driver */
int main() {
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        global_array[i] = (i * 17) % 100 - 50;  /* Range -50..49 */
        for (int j = 0; j < SIZE; j++) {
            global_matrix[i][j] = (i * j) % 100;
        }
    }
    
    printf("Testing complex loop nesting patterns...\n");
    
    /* Run all tests multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        test_perfect_nesting(global_array, SIZE);
        test_partial_overlap(global_matrix, 64, 64);
        test_sibling_loops(global_array, SIZE);
        test_multiple_entries(global_array, SIZE);
        
        /* Recursive test */
        int recursive_result = test_recursive_loops(global_array, 8, iter % SIZE);
        global_results[5] = recursive_result;
        
        test_hardware_hints(global_array, SIZE);
        
        /* Modify data for next iteration */
        for (int i = 0; i < SIZE; i++) {
            global_array[i] = (global_array[i] + global_results[i % 6]) % 100;
        }
    }
    
    /* Print results to prevent optimization */
    int final_sum = 0;
    for (int i = 0; i < 6; i++) {
        final_sum += global_results[i];
        printf("Result[%d] = %d\n", i, global_results[i]);
    }
    
    printf("Final checksum: %d\n", final_sum);
    printf("Test completed.\n");
    
    return final_sum != 0 ? 0 : 1;
}
