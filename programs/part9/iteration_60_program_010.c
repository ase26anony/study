/* Test program to trigger selective scheduler debug dumps in GCC */
/* Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-sched2 -c test.c -o test.o */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization */
volatile int global_counter = 0;
volatile int global_array[256];

/* Function 1: Inner loop with conditional branch and memory write */
void test_inner_loop(int n, int *arr) {
    int i, j;
    volatile int sum = 0;
    
    /* Create data dependencies to force scheduling decisions */
    for (i = 0; i < n; i++) {
        /* Complex expression with multiple operations */
        int temp = arr[i] * 3 + 7;
        
        /* Conditional store with side effect */
        if (temp > 100) {
            arr[i] = temp / 2;
            global_array[i & 255] = temp;
        } else {
            arr[i] = temp * 2;
            global_array[(i + 1) & 255] = temp + 1;
        }
        
        /* Use inline assembly to create unschedulable dependencies */
        asm volatile ("" : : "r"(temp) : "memory");
        
        /* Nested loop with varying iteration count */
        for (j = 0; j < (i & 7); j++) {
            sum += arr[i] + j;
        }
    }
    
    /* Force use of result */
    global_counter += sum;
}

/* Function 2: Nested loops with different iteration counts */
void test_nested_loops(int rows, int cols, int **matrix) {
    int i, j, k;
    volatile int acc = 0;
    
    /* Outer loop with pipelining opportunities */
    for (i = 0; i < rows; i++) {
        /* Middle loop with data-dependent exit */
        for (j = 0; j < cols && j < i + 5; j++) {
            /* Complex addressing calculation */
            int val = matrix[i][j];
            
            /* Multiple arithmetic operations */
            val = (val * 13 + 17) % 256;
            val = val ^ (val >> 4);
            val = val * 3 - 7;
            
            /* Conditional update with side effect */
            if (val > 128) {
                matrix[i][j] = val - 128;
                acc += val;
            } else {
                matrix[i][j] = val + 64;
                acc -= val;
            }
            
            /* Small inner loop for additional scheduling complexity */
            for (k = 0; k < 3; k++) {
                asm volatile ("" : "+r"(val) : : "memory");
                val += k;
            }
        }
        
        /* Function call within loop to create control flow */
        if (i % 3 == 0) {
            /* Use volatile function-like macro */
            asm volatile ("" : : "r"(acc) : "memory");
        }
    }
    
    global_counter += acc;
}

/* Function 3: Switch statement with computed goto-like behavior */
void test_switch_complex(int mode, int iterations) {
    int i, state = 0;
    volatile int buffer[8] = {0};
    
    for (i = 0; i < iterations; i++) {
        /* Complex mode-dependent computation */
        switch (mode) {
            case 0:
                state = (state * 7 + 3) & 0xFF;
                buffer[i & 7] = state;
                break;
            case 1:
                state = (state + i) * 11;
                buffer[(i + 1) & 7] = state >> 1;
                break;
            case 2:
                state = state ^ (i * 13);
                buffer[(i + 2) & 7] = state * 2;
                break;
            case 3:
                state = (state - i) / 3;
                buffer[(i + 3) & 7] = state + 100;
                break;
            default:
                state = i * i;
                buffer[i & 7] = state % 256;
                break;
        }
        
        /* Additional conditional logic */
        if (state & 1) {
            /* Memory barrier to prevent reordering */
            asm volatile ("" : : : "memory");
            buffer[0] += 1;
        }
        
        /* Alternate path with different operations */
        if (i % 5 == 0) {
            state = ~state;
            asm volatile ("" : "+r"(state) : : "cc");
        }
    }
    
    /* Ensure results are used */
    for (i = 0; i < 8; i++) {
        global_counter += buffer[i];
    }
}

/* Function 4: Mixed control flow with pointer chasing */
void test_pointer_chasing(int *data, int size) {
    int *ptr = data;
    int *end = data + size;
    volatile int hash = 0;
    
    while (ptr < end) {
        /* Data-dependent pointer arithmetic */
        int val = *ptr;
        
        /* Multiple conditional paths */
        if (val > 0) {
            hash = (hash * 31 + val) & 0xFFFF;
            ptr += 1;
        } else if (val < 0) {
            hash = (hash * 17 - val) & 0xFFFF;
            ptr += 2;
        } else {
            hash = (hash * 13 + 1) & 0xFFFF;
            ptr += (hash & 3) + 1;
        }
        
        /* Loop with variable bound */
        int j;
        for (j = 0; j < (val & 3); j++) {
            hash ^= (hash << j);
            asm volatile ("" : "+r"(hash) : : "cc");
        }
        
        /* Prevent tail call optimization */
        if (ptr - data > 1000) {
            asm volatile ("" : : "r"(ptr), "r"(hash) : "memory");
        }
    }
    
    global_counter += hash;
}

/* Function 5: Recursive-like pattern with manual unrolling */
void test_partial_unroll(int n, int *out) {
    int i;
    volatile int a = 1, b = 1, c = 1;
    
    /* Manually unrolled loop with dependencies */
    for (i = 0; i < n; i += 4) {
        /* Block 1 */
        a = a * 3 + 7;
        out[i] = a;
        asm volatile ("" : "+r"(a) : : "cc");
        
        /* Block 2 with dependency on block 1 */
        if (i + 1 < n) {
            b = a + b * 2;
            out[i + 1] = b;
            asm volatile ("" : "+r"(b) : : "cc");
        }
        
        /* Block 3 with mixed dependencies */
        if (i + 2 < n) {
            c = (a + b) * 5 - c;
            out[i + 2] = c;
            asm volatile ("" : "+r"(c) : : "cc");
        }
        
        /* Block 4 with complex expression */
        if (i + 3 < n) {
            int d = (a * b + c) % 256;
            out[i + 3] = d;
            asm volatile ("" : "+r"(d) : : "cc");
        }
    }
    
    global_counter += a + b + c;
}

/* Main driver that ensures all code paths are compiled */
int main(int argc, char **argv) {
    /* Initialize test data */
    int array1[100];
    int *array2[10];
    int matrix_data[10][20];
    int i, j;
    
    /* Initialize arrays with non-uniform data */
    for (i = 0; i < 100; i++) {
        array1[i] = (i * 17 + 23) % 100;
    }
    
    for (i = 0; i < 10; i++) {
        array2[i] = malloc(20 * sizeof(int));
        for (j = 0; j < 20; j++) {
            array2[i][j] = (i * 31 + j * 7) % 256;
        }
    }
    
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 20; j++) {
            matrix_data[i][j] = (i * 19 + j * 11) % 128;
        }
    }
    
    /* Call all test functions with different parameters */
    test_inner_loop(100, array1);
    test_nested_loops(10, 20, array2);
    test_switch_complex(argc > 1 ? atoi(argv[1]) % 5 : 0, 50);
    
    int chase_data[200];
    for (i = 0; i < 200; i++) {
        chase_data[i] = (i * 13) % 100 - 50;
    }
    test_pointer_chasing(chase_data, 200);
    
    int out_array[50];
    test_partial_unroll(50, out_array);
    
    /* Cleanup */
    for (i = 0; i < 10; i++) {
        free(array2[i]);
    }
    
    return global_counter != 0 ? 0 : 1;
}
