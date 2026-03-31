/* Test program to trigger selective scheduler debug dumps in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile int g_volatile_array[256];

/* Function 1: Inner loop with conditional branch and memory write */
void test_inner_loop(int n, int *arr) {
    int i, j;
    volatile int local_vol = 0;
    
    for (i = 0; i < n; i++) {
        /* Create data dependency */
        int sum = i;
        
        /* Inner loop with conditional */
        for (j = 0; j < 100; j++) {
            /* Complex expression with multiple operations */
            sum += (j * 3) / (i + 1);
            
            /* Conditional store to prevent simple scheduling */
            if (sum % 2 == 0) {
                arr[j] = sum;
                /* Inline asm to create unschedulable dependency */
                asm volatile ("" : : "r"(sum) : "memory");
            } else {
                arr[j] = sum * 2;
                /* Another asm barrier */
                asm volatile ("" : : "r"(sum) : "memory");
            }
            
            /* Memory access with volatile */
            local_vol += g_volatile_array[j & 255];
        }
        
        /* Function call to create control flow complexity */
        g_volatile_counter += local_vol;
    }
}

/* Function 2: Nested loops with different iteration counts */
int test_nested_loops(int rows, int cols, int **matrix) {
    int total = 0;
    volatile int checksum = 0;
    
    /* Outer loop */
    for (int i = 0; i < rows; i++) {
        int row_sum = 0;
        
        /* Middle loop */
        for (int j = 0; j < cols; j++) {
            int cell = 0;
            
            /* Inner loop with data dependency chain */
            for (int k = 0; k < 8; k++) {
                cell += (i * j * k) + matrix[i][j];
                /* Prevent optimization with asm */
                asm volatile ("" : "+r"(cell) : : "memory");
            }
            
            /* Conditional update */
            if (cell > 1000) {
                row_sum += cell / 3;
            } else if (cell > 100) {
                row_sum += cell / 2;
            } else {
                row_sum += cell;
            }
            
            /* Access volatile global */
            checksum ^= g_volatile_array[(i + j) & 255];
        }
        
        /* Complex exit condition */
        if (row_sum % 7 == 0) {
            total += row_sum;
        } else {
            total += row_sum * 2;
        }
    }
    
    return total;
}

/* Function 3: Switch statement with computed goto-like behavior */
void test_switch_complex(int mode, int iterations, int *results) {
    volatile int state = mode;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Complex switch with multiple cases */
        switch (state % 5) {
            case 0:
                results[i] = i * 2;
                state += results[i];
                /* Asm to create specific RTL patterns */
                asm volatile ("# case 0" : : "r"(results[i]));
                break;
                
            case 1:
                results[i] = i * i;
                state ^= results[i];
                asm volatile ("# case 1" : : "r"(results[i]), "r"(state));
                break;
                
            case 2:
                results[i] = i + state;
                state *= 3;
                /* Memory barrier */
                asm volatile ("" : : : "memory");
                break;
                
            case 3:
                results[i] = state - i;
                state /= 2;
                /* Force register usage */
                register int reg_var asm ("r12") = results[i];
                asm volatile ("" : : "r"(reg_var));
                break;
                
            case 4:
                results[i] = (state << 2) | (i & 0xFF);
                state = ~state;
                /* Multiple asm statements */
                asm volatile ("# case 4 start" : : : "memory");
                asm volatile ("" : : "r"(state), "r"(results[i]));
                asm volatile ("# case 4 end" : : : "memory");
                break;
                
            default:
                results[i] = 0;
                break;
        }
        
        /* Access volatile memory */
        g_volatile_counter += g_volatile_array[i & 255];
    }
}

/* Function 4: Mixed control flow with function pointers */
typedef int (*op_func_t)(int, int);

int add_op(int a, int b) { return a + b; }
int sub_op(int a, int b) { return a - b; }
int mul_op(int a, int b) { return a * b; }
int div_op(int a, int b) { return b != 0 ? a / b : 0; }

void test_function_pointers(int n, int *output) {
    op_func_t ops[] = {add_op, sub_op, mul_op, div_op};
    volatile int selector = 0;
    
    for (int i = 0; i < n; i++) {
        /* Vary the operation based on complex condition */
        int op_idx = (i + selector) % 4;
        
        /* Create data dependency chain */
        int a = i * 3;
        int b = i + 5;
        
        /* Function call through pointer - creates call RTL */
        output[i] = ops[op_idx](a, b);
        
        /* Update volatile to prevent dead code elimination */
        selector += output[i];
        
        /* Inline asm with clobbered registers */
        asm volatile ("" : : "r"(a), "r"(b), "r"(output[i]) : "r0", "r1");
    }
}

/* Function 5: Loop with early exit and complex index calculation */
int test_early_exit(int limit, int *data) {
    int sum = 0;
    volatile int threshold = limit / 2;
    
    for (int i = 0; i < limit; i++) {
        /* Complex index calculation */
        int idx = (i * 7 + 3) % 256;
        
        /* Early exit condition */
        if (sum > threshold * 1000) {
            /* Asm barrier before break */
            asm volatile ("# early exit" : : "r"(sum) : "memory");
            break;
        }
        
        /* Load-modify-store sequence */
        int val = data[idx];
        val = (val * 3 + i) & 0xFFFF;
        data[idx] = val;
        sum += val;
        
        /* Dependency on volatile */
        if (g_volatile_counter > 100) {
            sum -= g_volatile_array[idx];
        }
    }
    
    return sum;
}

/* Main driver function */
int main(int argc, char **argv) {
    /* Initialize test data */
    int array1[100];
    int array2[256];
    int results[200];
    
    /* Initialize matrix */
    int *matrix[50];
    for (int i = 0; i < 50; i++) {
        matrix[i] = (int *)malloc(60 * sizeof(int));
        for (int j = 0; j < 60; j++) {
            matrix[i][j] = i * j;
        }
    }
    
    /* Initialize volatile array */
    srand(time(NULL));
    for (int i = 0; i < 256; i++) {
        g_volatile_array[i] = rand() % 1000;
    }
    
    /* Call test functions to ensure they're compiled */
    test_inner_loop(50, array1);
    
    int total = test_nested_loops(50, 60, matrix);
    
    test_switch_complex(2, 100, results);
    
    test_function_pointers(150, array2);
    
    int sum = test_early_exit(1000, array2);
    
    /* Use results to prevent dead code elimination */
    printf("Results: total=%d, sum=%d, counter=%d\n", 
           total, sum, g_volatile_counter);
    
    /* Cleanup */
    for (int i = 0; i < 50; i++) {
        free(matrix[i]);
    }
    
    return 0;
}
