/* Test program to trigger selective scheduler debug dumps in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent optimization */
volatile int g_counter = 0;
volatile int g_result = 0;

/* Function 1: Inner loop with conditional branch and memory write
   This creates data dependencies that require careful scheduling */
int test_inner_loop(int *arr, int n) {
    int sum = 0;
    volatile int temp = 0;
    
    /* Loop with data dependency chain */
    for (int i = 0; i < n; i++) {
        /* Create anti-dependency with volatile */
        temp = arr[i];
        
        /* Conditional store with dependency */
        if (temp > 0) {
            sum += temp;
            /* Memory write with side effect */
            arr[i] = sum % 256;
        } else {
            sum -= (-temp);
            arr[i] = 0;
        }
        
        /* Inline assembly to create specific RTL patterns */
        asm volatile ("" : : "r"(sum), "r"(temp) : "memory");
    }
    
    /* Another dependency chain */
    for (int i = n-1; i >= 0; i--) {
        sum += arr[i];
        asm volatile ("" : : "r"(sum) : "memory");
    }
    
    return sum;
}

/* Function 2: Nested loops with different iteration counts
   Creates outer loop pipelining opportunities */
int test_nested_loops(int rows, int cols, int *matrix) {
    int total = 0;
    volatile int row_sum = 0;
    
    /* Outer loop - target for outer loop pipelining */
    for (int i = 0; i < rows; i++) {
        row_sum = 0;
        
        /* Inner loop with complex addressing */
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            int val = matrix[idx];
            
            /* Multiple condition checks */
            if (val > 100) {
                row_sum += val * 2;
            } else if (val > 50) {
                row_sum += val;
            } else {
                row_sum += val / 2;
            }
            
            /* Modify matrix with dependency */
            matrix[idx] = (row_sum + val) % 128;
            
            /* Prevent optimization */
            asm volatile ("" : : "r"(row_sum), "r"(val) : "memory");
        }
        
        total += row_sum;
        
        /* Cross-iteration dependency */
        if (i > 0) {
            total -= matrix[(i-1) * cols];
        }
    }
    
    return total;
}

/* Function 3: Switch statement with computed goto-like behavior
   Creates complex control flow for the scheduler */
int test_switch_complex(int x, int *output) {
    int result = 0;
    static volatile int state = 0;
    
    /* Multiple basic blocks with switch */
    switch (x % 5) {
        case 0:
            result = x * 2;
            /* Call to external function creates scheduling barrier */
            state++;
            asm volatile ("" : : "r"(state) : "memory");
            break;
            
        case 1:
            result = x + 100;
            for (int i = 0; i < 3; i++) {
                result += i * x;
                asm volatile ("" : : "r"(result) : "memory");
            }
            break;
            
        case 2:
            result = x - 50;
            if (result > 0) {
                *output = result;
                result *= 2;
            }
            break;
            
        case 3:
            /* Loop inside switch case */
            for (int i = 0; i < x % 10; i++) {
                result += i * i;
                asm volatile ("" : : "r"(result) : "memory");
            }
            break;
            
        case 4:
            result = 1;
            for (int i = 1; i <= (x % 8); i++) {
                result *= i;
                asm volatile ("" : : "r"(result) : "memory");
            }
            break;
    }
    
    /* Post-switch computation */
    result += state;
    *output += result;
    
    return result;
}

/* Function 4: Mixed operations with pointer aliasing
   Creates memory disambiguation challenges */
int test_mixed_ops(int *a, int *b, int *c, int n) {
    int sum1 = 0, sum2 = 0;
    volatile int acc = 0;
    
    /* Loop with potential pointer aliasing */
    for (int i = 0; i < n; i++) {
        int t1 = a[i];
        int t2 = b[i];
        
        /* Complex expression with multiple operations */
        int r1 = (t1 * t2) + (t1 >> 3) - (t2 & 0xFF);
        int r2 = (t1 + t2) * (t1 - t2);
        
        /* Conditional store with side effect */
        if (r1 > r2) {
            c[i] = r1;
            sum1 += r1;
        } else {
            c[i] = r2;
            sum2 += r2;
        }
        
        /* Accumulator with dependency */
        acc = acc + r1 - r2;
        asm volatile ("" : : "r"(acc), "r"(r1), "r"(r2) : "memory");
    }
    
    /* Final reduction */
    for (int i = 0; i < n; i += 2) {
        sum1 += c[i];
        sum2 += c[i+1];
        asm volatile ("" : : "r"(sum1), "r"(sum2) : "memory");
    }
    
    return sum1 + sum2 + acc;
}

/* Function 5: Recursive-like pattern using loop
   Creates back-edge dependencies */
int test_backedge_deps(int n, int *arr) {
    int prev = 1, curr = 1;
    volatile int mod = 10007;
    
    /* Fibonacci-like computation with memory writes */
    for (int i = 0; i < n; i++) {
        int next = (prev + curr) % mod;
        
        /* Store with dependency on previous iteration */
        arr[i] = next;
        
        /* Complex update with multiple operations */
        prev = (curr * 2) % mod;
        curr = (next + i) % mod;
        
        /* Prevent dead code elimination */
        asm volatile ("" : : "r"(prev), "r"(curr), "r"(next) : "memory");
        
        /* Additional computation every 8 iterations */
        if ((i & 7) == 0) {
            int temp = 0;
            for (int j = 0; j < 4; j++) {
                temp += arr[(i - j) & (n-1)];
                asm volatile ("" : : "r"(temp) : "memory");
            }
            arr[i] = (arr[i] + temp) % mod;
        }
    }
    
    return curr;
}

/* Main driver function */
int main(int argc, char **argv) {
    /* Initialize test data */
    int size1 = 100;
    int size2 = 10;
    int *arr1 = (int*)malloc(size1 * sizeof(int));
    int *arr2 = (int*)malloc(size2 * size2 * sizeof(int));
    int *arr3 = (int*)malloc(size1 * sizeof(int));
    int *arr4 = (int*)malloc(size1 * sizeof(int));
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < size1; i++) {
        arr1[i] = (i * 37 + 123) % 256;
        arr3[i] = (i * 51 + 79) % 256;
        arr4[i] = (i * 29 + 167) % 256;
    }
    
    for (int i = 0; i < size2 * size2; i++) {
        arr2[i] = (i * 43 + 91) % 256;
    }
    
    /* Call all test functions to ensure they're compiled */
    int output = 0;
    
    int r1 = test_inner_loop(arr1, size1);
    int r2 = test_nested_loops(size2, size2, arr2);
    int r3 = test_switch_complex(argc > 1 ? atoi(argv[1]) : 42, &output);
    int r4 = test_mixed_ops(arr1, arr3, arr4, size1);
    int r5 = test_backedge_deps(size1, arr1);
    
    /* Use results to prevent dead code elimination */
    volatile int final_result = r1 + r2 + r3 + r4 + r5 + output;
    
    /* Clean up */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    
    return final_result != 0 ? 0 : 1;
}
