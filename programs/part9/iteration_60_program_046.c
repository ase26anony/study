/* Test program to trigger selective scheduling debug dumps in GCC */
/* Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-sched2 -c test.c -o test.o */

#include <stdio.h>
#include <stdlib.h>

/* Function 1: Inner loop with conditional branch and memory write */
/* This creates data dependencies that prevent simple scheduling */
void test_inner_loop(int *arr, int n) {
    volatile int sum = 0;  /* volatile prevents optimization */
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Create data dependency chain */
        int temp = arr[i];
        
        /* Conditional store with dependency */
        if (temp > 0) {
            sum += temp;
            arr[i] = sum;  /* Store with anti-dependency */
        } else {
            /* Use inline asm to create unschedulable dependency */
            asm volatile ("" : : "r"(temp) : "memory");
            sum -= temp;
        }
        
        /* Additional computation to create more scheduling opportunities */
        for (j = 0; j < 3; j++) {
            sum += j * temp;
            /* Memory barrier to prevent reordering */
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Final store to ensure all computations are used */
    arr[0] = sum;
}

/* Function 2: Nested loops with different iteration counts */
/* Triggers outer loop pipelining optimizations */
int test_nested_loops(int size) {
    int i, j, k;
    volatile int result = 0;
    static int matrix[100][100];  /* Static to avoid stack overflow */
    
    /* Initialize with some values */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            matrix[i][j] = i * j;
        }
    }
    
    /* Complex nested loop with data dependencies */
    for (i = 1; i < size; i++) {
        for (j = 1; j < size; j++) {
            int acc = 0;
            for (k = 0; k < 10; k++) {
                /* Cross-iteration dependency */
                acc += matrix[i-1][j] * matrix[i][j-1] + k;
                
                /* Conditional with side effect */
                if (acc % 2 == 0) {
                    matrix[i][j] = acc;
                    /* Inline asm to prevent optimization */
                    asm volatile ("# dependency barrier" : : "r"(acc));
                }
            }
            result += acc;
        }
    }
    
    return result;
}

/* Function 3: Switch statement with computed goto for complex control flow */
void test_complex_control_flow(int mode, int *output) {
    static void *labels[] = { &&case0, &&case1, &&case2, &&case3 };
    volatile int counter = 0;
    
    if (mode < 0 || mode > 3) return;
    
    /* Computed goto creates complex control flow graph */
    goto *labels[mode];
    
case0:
    for (int i = 0; i < 100; i++) {
        output[i] = i * 2;
        counter++;
        if (counter > 50) goto case2;  /* Unusual control flow */
    }
    return;
    
case1:
    {
        int temp = 0;
        while (temp < 100) {
            output[temp] = temp * temp;
            /* Memory access with dependency */
            asm volatile ("" : "+r"(temp) : : "memory");
            temp += (counter % 3) + 1;  /* Variable increment */
        }
    }
    return;
    
case2:
    do {
        output[counter] = counter | 0xAA55;
        counter++;
        /* Loop with multiple exit conditions */
        if (counter >= 100) break;
        if (output[counter-1] < 0) goto case3;
    } while (1);
    return;
    
case3:
    for (int i = 99; i >= 0; i--) {
        output[i] = output[i] + counter;
        /* Create data dependency */
        counter = output[i] % 7;
        asm volatile ("# complex control flow" : : "r"(counter));
    }
}

/* Function 4: Mixed operations with function calls */
/* Function calls create scheduling boundaries */
static int helper1(int x) {
    return x * 3 + 1;
}

static int helper2(int x, int y) {
    volatile int r = x * y;
    asm volatile ("" : "+r"(r) : : "memory");
    return r;
}

void test_mixed_operations(int *data, int len) {
    int i;
    volatile int state = 0;
    
    for (i = 0; i < len; i++) {
        /* Mix of operations */
        int val = data[i];
        
        /* Function call creates scheduling boundary */
        val = helper1(val);
        
        /* Arithmetic with dependency */
        state = state ^ val;
        
        /* Conditional store */
        if (state > 1000) {
            data[i] = helper2(state, i);
        } else {
            data[i] = val;
            /* Memory barrier */
            asm volatile ("" : : : "memory");
        }
        
        /* Additional computation to increase basic block size */
        for (int j = 0; j < 2; j++) {
            state += (i * j) % 7;
        }
    }
}

/* Function 5: Pointer chasing with aliasing */
/* Creates memory dependencies that are hard to analyze */
void test_pointer_chasing(int **ptrs, int count) {
    volatile int sum = 0;
    int *current = ptrs[0];
    
    for (int i = 0; i < count; i++) {
        /* Pointer dereference with potential aliasing */
        int val = *current;
        
        /* Complex addressing mode */
        current = ptrs[(val + i) % count];
        
        /* Data-dependent computation */
        sum = sum + (val * i);
        
        /* Conditional with side effect */
        if (sum < 0) {
            sum = -sum;
            /* Inline asm prevents reordering */
            asm volatile ("# pointer chase" : : "r"(sum), "r"(current));
        }
    }
    
    /* Store result to prevent dead code elimination */
    *ptrs[0] = sum;
}

/* Main driver function */
int main() {
    int arr[100];
    int *ptrs[10];
    int buffer[100];
    
    /* Initialize test data */
    for (int i = 0; i < 100; i++) {
        arr[i] = i - 50;  /* Mix of positive and negative values */
    }
    
    for (int i = 0; i < 10; i++) {
        ptrs[i] = &arr[i * 10];
    }
    
    /* Call all test functions to ensure they're compiled */
    test_inner_loop(arr, 100);
    
    int result1 = test_nested_loops(50);
    
    test_complex_control_flow(2, buffer);
    
    test_mixed_operations(arr, 100);
    
    test_pointer_chasing(ptrs, 10);
    
    /* Use results to prevent dead code elimination */
    volatile int final_result = result1 + arr[0] + buffer[0];
    
    return final_result != 0 ? 0 : 1;
}
