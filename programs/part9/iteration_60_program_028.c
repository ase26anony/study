/* test_sel_sched_dump.c
 * 
 * This program is designed to trigger GCC's selective scheduler debug dumps
 * specifically to cover lines 159-163 in sel-sched-dump.cc.
 * 
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-sched2 -c test_sel_sched_dump.c
 * Or: gcc -O3 -fsel-sched-pipelining-outer-loops -dS -fdump-rtl-all -c test_sel_sched_dump.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimizations from removing our test code */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Function 1: Inner loop with conditional branch and memory write
 * Creates data dependencies that force the scheduler to work hard */
NOINLINE int test_inner_loop(int *arr, int n) {
    int sum = 0;
    VOLATILE_VAR int temp;
    
    /* Complex loop with data dependencies */
    for (int i = 0; i < n; i++) {
        /* Create dependency chain */
        temp = arr[i];
        
        /* Conditional store with side effect */
        if (temp > 100) {
            arr[i] = temp / 2;
            sum += arr[i] * 3;
        } else if (temp < 0) {
            arr[i] = temp * 2;
            sum += arr[i] - 5;
        } else {
            arr[i] = temp + 1;
            sum += arr[i];
        }
        
        /* Inline assembly to create unschedulable dependencies */
        asm volatile ("" : : "r"(temp), "r"(arr[i]) : "memory");
    }
    
    /* Another dependency */
    asm volatile ("" : : "r"(sum) : "memory");
    return sum;
}

/* Function 2: Nested loops with different iteration counts
 * Triggers outer loop pipelining optimizations */
NOINLINE int test_nested_loops(int *matrix, int rows, int cols) {
    int total = 0;
    VOLATILE_VAR int row_sum;
    
    /* Outer loop - may trigger outer loop pipelining */
    for (int i = 0; i < rows; i++) {
        row_sum = 0;
        
        /* Inner loop with complex addressing */
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            int val = matrix[idx];
            
            /* Multiple conditional paths */
            switch (val % 4) {
                case 0:
                    matrix[idx] = val + i;
                    row_sum += val * 2;
                    break;
                case 1:
                    matrix[idx] = val - j;
                    row_sum += val / 2;
                    break;
                case 2:
                    matrix[idx] = val * 3;
                    row_sum += val + 7;
                    break;
                default:
                    matrix[idx] = val;
                    row_sum += val - 3;
                    break;
            }
            
            /* Memory barrier to prevent reordering */
            asm volatile ("" : : "r"(val), "r"(matrix[idx]) : "memory");
        }
        
        total += row_sum;
        
        /* Prevent loop invariant code motion */
        asm volatile ("" : : "r"(row_sum) : "memory");
    }
    
    return total;
}

/* Function 3: Complex control flow with switch and computed goto
 * Creates challenging scheduling problems */
NOINLINE int test_complex_control_flow(int *data, int size) {
    static void *labels[] = { &&case0, &&case1, &&case2, &&case3, &&default_case };
    int result = 0;
    
    for (int i = 0; i < size; i++) {
        int val = data[i];
        int op = val % 5;
        
        /* Computed goto - creates complex control flow */
        goto *labels[op];
        
    case0:
        data[i] = val + result;
        result = data[i] * 2;
        asm volatile ("" : : "r"(val) : "memory");
        continue;
        
    case1:
        data[i] = val - i;
        result += data[i] / 3;
        asm volatile ("" : : "r"(val) : "memory");
        continue;
        
    case2:
        data[i] = val * val;
        result = result - data[i];
        asm volatile ("" : : "r"(val) : "memory");
        continue;
        
    case3:
        data[i] = val >> 2;
        result = result ^ data[i];
        asm volatile ("" : : "r"(val) : "memory");
        continue;
        
    default_case:
        data[i] = val;
        result = result | val;
        asm volatile ("" : : "r"(val) : "memory");
        continue;
    }
    
    return result;
}

/* Function 4: Mixed operations with function calls
 * Creates scheduling barriers */
NOINLINE int helper1(int x) {
    return x * 3 + 7;
}

NOINLINE int helper2(int x, int y) {
    return (x + y) * 2;
}

NOINLINE int test_mixed_operations(int *arr, int n) {
    int acc = 0;
    
    for (int i = 0; i < n; i++) {
        /* Function calls create scheduling barriers */
        int t1 = helper1(arr[i]);
        int t2 = helper2(arr[i], i);
        
        /* Complex expression with multiple dependencies */
        arr[i] = (t1 > t2) ? t1 - t2 : t2 - t1;
        
        /* Volatile access prevents dead code elimination */
        VOLATILE_VAR int dummy = arr[i];
        acc += dummy * (i + 1);
        
        /* Memory clobber prevents reordering */
        asm volatile ("" : : "r"(t1), "r"(t2), "r"(dummy) : "memory");
    }
    
    return acc;
}

/* Function 5: Loop with early exit and multiple exits
 * Creates control flow challenges */
NOINLINE int test_early_exit(int *arr, int n, int threshold) {
    int sum = 0;
    int count = 0;
    
    for (int i = 0; i < n; i++) {
        if (arr[i] > threshold) {
            /* Early exit path */
            sum += arr[i] * 2;
            count++;
            
            if (count > 5) {
                /* Another exit point */
                asm volatile ("" : : "r"(sum) : "memory");
                return sum;
            }
        } else if (arr[i] < -threshold) {
            /* Alternative path */
            sum -= arr[i];
            arr[i] = 0;
        } else {
            /* Default path */
            sum += arr[i];
            arr[i] = sum;
        }
        
        /* Dependency on previous iteration */
        asm volatile ("" : : "r"(sum), "r"(arr[i]) : "memory");
        
        /* Occasionally break */
        if (i % 7 == 0 && sum > 1000) {
            break;
        }
    }
    
    return sum;
}

/* Main driver function */
int main(void) {
    /* Initialize test data */
    int data1[100];
    int data2[10][10];
    int data3[50];
    int data4[80];
    int data5[60];
    
    /* Fill with pseudo-random but deterministic values */
    for (int i = 0; i < 100; i++) {
        data1[i] = (i * 37 + 123) % 200 - 100;
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            data2[i][j] = (i * 13 + j * 29 + 47) % 150 - 75;
        }
    }
    
    for (int i = 0; i < 50; i++) {
        data3[i] = (i * 19 + 83) % 100 - 50;
    }
    
    for (int i = 0; i < 80; i++) {
        data4[i] = (i * 23 + 61) % 120 - 60;
    }
    
    for (int i = 0; i < 60; i++) {
        data5[i] = (i * 31 + 97) % 180 - 90;
    }
    
    /* Call all test functions to ensure they're compiled */
    int result = 0;
    
    result += test_inner_loop(data1, 100);
    result += test_nested_loops((int *)data2, 10, 10);
    result += test_complex_control_flow(data3, 50);
    result += test_mixed_operations(data4, 80);
    result += test_early_exit(data5, 60, 50);
    
    /* Use result to prevent dead code elimination */
    VOLATILE_VAR int final_result = result;
    
    printf("Result: %d\n", final_result);
    
    return 0;
}
