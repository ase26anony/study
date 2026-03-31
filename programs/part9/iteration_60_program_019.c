/* test_sel_sched_dump.c
 * Program to trigger selective scheduler debug dumps in GCC
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-sched2 -c test_sel_sched_dump.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile int g_volatile_array[100];

/* Function 1: Inner loop with conditional branch and memory write
 * This creates a scheduling region with data dependencies */
int func_inner_loop(int *arr, int n) {
    int sum = 0;
    volatile int local_vol = 0;
    
    /* Loop with data dependency chain */
    for (int i = 0; i < n; i++) {
        /* Create anti-dependency with volatile */
        local_vol = arr[i];
        
        /* Conditional store with side effect */
        if (i % 3 == 0) {
            arr[i] = local_vol * 2 + g_volatile_counter;
            sum += arr[i];
        } else if (i % 3 == 1) {
            arr[i] = local_vol / 2 - g_volatile_counter;
            sum -= arr[i];
        } else {
            arr[i] = local_vol + i;
            sum ^= arr[i];
        }
        
        /* Inline asm to create unschedulable dependency */
        asm volatile ("" : : "r"(local_vol), "r"(arr[i]) : "memory");
    }
    
    /* Another loop with different pattern */
    for (int j = n - 1; j >= 0; j -= 2) {
        arr[j] += sum;
        /* Memory barrier-like asm */
        asm volatile ("" : : "m"(arr[j]));
    }
    
    return sum;
}

/* Function 2: Nested loops with outer loop pipelining opportunities */
double func_nested_loops(double *matrix, int rows, int cols) {
    double total = 0.0;
    volatile double acc = 0.0;
    
    /* Outer loop - target for outer loop pipelining */
    for (int i = 0; i < rows; i++) {
        double row_sum = 0.0;
        
        /* Inner loop with complex addressing */
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            
            /* Conditional with floating point ops */
            if (j % 4 == 0) {
                matrix[idx] = matrix[idx] * 1.5 + acc;
            } else if (j % 4 == 1) {
                matrix[idx] = matrix[idx] / 1.3 - acc;
            } else if (j % 4 == 2) {
                matrix[idx] = matrix[idx] + (double)j;
            } else {
                matrix[idx] = matrix[idx] - (double)i;
            }
            
            row_sum += matrix[idx];
            
            /* Dependency on volatile */
            acc = row_sum * 0.01;
            
            /* Asm to prevent reordering */
            asm volatile ("" : : "r"(row_sum), "r"(acc) : "memory");
        }
        
        total += row_sum;
        
        /* Store to volatile array */
        g_volatile_array[i % 100] = (int)row_sum;
    }
    
    return total;
}

/* Function 3: Switch statement with computed goto-like pattern */
int func_switch_complex(int mode, int iterations) {
    static void* jump_table[] = {
        &&case_0, &&case_1, &&case_2, &&case_3, &&case_default
    };
    
    int result = 0;
    volatile int state = mode;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex switch with fallthrough */
        switch (state % 5) {
            case 0:
                result += i * 2;
                /* Fall through */
            case 1:
                result ^= (i << 3);
                state = result % 7;
                break;
            case 2:
                result -= i / 2;
                if (result < 0) result = -result;
                state = (state * 3) % 5;
                break;
            case 3:
                result *= (i % 10) + 1;
                /* Memory operation */
                g_volatile_counter = result;
                state = (state + g_volatile_counter) % 5;
                break;
            default:
                result = (result >> 1) | 0x8000;
                state = mode;
                break;
        }
        
        /* Inline asm with multiple constraints */
        asm volatile (
            "/* Dependency chain */"
            : "=r"(result), "=r"(state)
            : "0"(result), "1"(state), "m"(g_volatile_counter)
            : "cc"
        );
    }
    
    return result;
}

/* Function 4: Mixed operations with pointer aliasing */
void func_mixed_ops(int *a, int *b, int *c, int n) {
    volatile int temp;
    
    for (int i = 0; i < n; i++) {
        /* Pointer arithmetic with potential aliasing */
        int *ptr1 = a + i;
        int *ptr2 = b + (n - i - 1);
        int *ptr3 = c + (i % 10);
        
        /* Complex sequence of operations */
        temp = *ptr1;
        *ptr1 = *ptr2 + temp;
        
        temp = *ptr2;
        *ptr2 = *ptr3 - temp;
        
        temp = *ptr3;
        *ptr3 = *ptr1 ^ temp;
        
        /* Conditional with side effect */
        if ((i & 0xF) == 0) {
            g_volatile_array[i % 100] = *ptr1 + *ptr2 + *ptr3;
        }
        
        /* Memory barrier */
        asm volatile ("" : : "m"(*ptr1), "m"(*ptr2), "m"(*ptr3) : "memory");
    }
}

/* Function 5: Recursive-like pattern with tail operations */
int func_tail_operations(int seed, int count) {
    int x = seed;
    volatile int y = 0;
    
    while (count-- > 0) {
        /* Data-dependent chain */
        int t1 = x * 1103515245 + 12345;
        int t2 = (t1 >> 16) & 0x7FFF;
        int t3 = t2 ^ x;
        
        /* Conditional store to volatile */
        if (t3 % 7 == 0) {
            y = t3;
            g_volatile_counter = y;
        }
        
        /* Complex expression */
        x = (t1 + t2 * t3) % 1000;
        
        /* Asm with clobber */
        asm volatile (
            "addl %1, %0\n\t"
            "rorl $3, %0"
            : "+r"(x)
            : "r"(t3)
            : "cc"
        );
        
        /* Function call to external function (prevents inlining) */
        x = abs(x);
    }
    
    return x ^ y;
}

/* Main driver function */
int main(int argc, char **argv) {
    /* Initialize test data */
    int array1[200];
    double matrix[20][30];
    int array2[100];
    int array3[100];
    
    /* Initialize arrays */
    for (int i = 0; i < 200; i++) {
        array1[i] = i * 3 + 7;
    }
    
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 30; j++) {
            matrix[i][j] = (double)(i * j) / 10.0;
        }
    }
    
    for (int i = 0; i < 100; i++) {
        array2[i] = i * 2;
        array3[i] = i * 5;
    }
    
    /* Call all test functions to ensure they're compiled */
    int result1 = func_inner_loop(array1, 150);
    double result2 = func_nested_loops(&matrix[0][0], 20, 30);
    int result3 = func_switch_complex(argc > 1 ? atoi(argv[1]) : 2, 50);
    func_mixed_ops(array2, array3, array1, 80);
    int result5 = func_tail_operations(42, 30);
    
    /* Use results to prevent dead code elimination */
    volatile int final_result = result1 + (int)result2 + result3 + result5;
    
    printf("Results: %d, %.2f, %d, %d\n", 
           result1, result2, result3, result5);
    
    return final_result != 0 ? 0 : 1;
}
